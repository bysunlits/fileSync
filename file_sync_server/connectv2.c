#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8888 // 服务器监听端口
#define MAX_EVENTS 64 // epoll_wait 一次最多返回的就绪事件数
#define THREAD_NUM 4 // 工作线程数量
#define QUEUE_SIZE 1024 // 任务队列容量（最多缓存多少个待处理的 fd）

// --- 1. 任务队列（主线程向里扔 fd，工作线程从里取 fd 去 recv） ---
// 这是一个环形队列（ring buffer）：用 head/tail 两个下标在固定数组上循环使用，
// 避免频繁的插入/删除操作。它是主线程和多个工作线程之间的“共享缓冲区”。
typedef struct {
	int fds[QUEUE_SIZE]; // 存放待处理的 client fd
	int head, tail,
		count; // head: 出队位置；tail: 入队位置；count: 当前元素个数
	pthread_mutex_t lock; // 互斥锁：保护 head/tail/count
			      // 这些共享变量，防止多线程同时改
	pthread_cond_t
		not_empty; // 条件变量：队列“非空”时通知等待的工作线程来取任务
} TaskQueue;

TaskQueue queue; // 全局任务队列（主线程入队，工作线程出队）
int g_epoll_fd; // 全局 epoll 实例 fd，工作线程处理完数据后要用它来重置 ONESHOT

// 初始化队列：下标清零、创建锁和条件变量
void queue_init(TaskQueue *q)
{
	q->head = q->tail = q->count = 0;
	pthread_mutex_init(&q->lock, NULL); // 初始化互斥锁（默认属性）
	pthread_cond_init(&q->not_empty, NULL); // 初始化条件变量（默认属性）
}

// 入队：把就绪的 client fd 放进队列（由主线程调用）
void queue_push(TaskQueue *q, int fd)
{
	pthread_mutex_lock(&q->lock); // 加锁，保证对队列的修改是原子的
	if (q->count <
	    QUEUE_SIZE) { // 队列没满才放入（满了就丢弃，这里没做阻塞处理）
		q->fds[q->tail] = fd;
		q->tail = (q->tail + 1) % QUEUE_SIZE; // tail 环形前进
		q->count++;
		pthread_cond_signal(
			&q->not_empty); // 唤醒一个正在等任务的工作线程
	}
	pthread_mutex_unlock(&q->lock); // 解锁
}

// 出队：从队列取出一个 client fd（由工作线程调用），队列为空时会阻塞等待
int queue_pop(TaskQueue *q)
{
	pthread_mutex_lock(&q->lock);
	// 用 while 而不是 if：被唤醒后必须重新判断条件。
	// 原因：① POSIX 允许“虚假唤醒”（没信号也返回）；②
	// 可能被唤醒时任务已被别的线程抢走。
	while (q->count == 0) {
		// 队列空 → 睡在条件变量上等通知。
		// pthread_cond_wait 会原子地“释放锁 +
		// 挂起”，被唤醒后再重新加锁才返回。
		// 注意：条件变量本身不存数据、也不知道
		// q->count，真正的“条件”就是这个 while 判断。
		pthread_cond_wait(&q->not_empty, &q->lock);
	}
	int fd =
		q->fds[q->head]; // 取出队头元素（此时已重新持有锁，读取是安全的）
	q->head = (q->head + 1) % QUEUE_SIZE; // head 环形前进
	q->count--;
	pthread_mutex_unlock(&q->lock);
	return fd;
}

// 把 fd 设置为非阻塞模式（边缘触发 EPOLLET 必须搭配非阻塞 IO）
int set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0); // 取出原来的所有配置
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK); //  叠加上“非阻塞”标签后写回
}

// 重新激活该 fd 的 epoll 监听（解冻）
// EPOLLONESHOT 让一个 fd 的事件只触发一次；处理完后必须用 EPOLL_CTL_MOD
// 重新“武装”， 否则这个连接再也不会被 epoll 通知了。
void reset_oneshot(int epoll_fd, int fd)
{
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET |
		    EPOLLONESHOT; // 重新监听可读，仍用边缘触发 + 一次性
	ev.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev); // MOD：修改已有注册项
}

// --- 2. 工作线程函数：真正干 recv 活的地方 ---
void *worker_thread(void *arg)
{
	char buf[512]; // 每个线程自己的读缓冲区（放在栈上，天然线程私有，无需加锁）

	while (1) {
		// 从队列抢一个就绪的 client_fd（队列空则在此阻塞休眠）
		int client_fd = queue_pop(&queue);

		int is_closed = 0; // 标记本次处理后连接是否已关闭
		printf("[线程 %lx] 正在处理 fd: %d 的 recv\n", pthread_self(),
		       client_fd);

		// 因为使用了边缘触发(EPOLLET)，必须循环 recv 直到读完(返回
		// EAGAIN)
		// 边缘触发只在“状态变化”时通知一次，所以一次通知后要把数据全部读干净，
		// 否则剩下的数据不会再有新事件提醒你。
		while (1) {
			ssize_t n = recv(client_fd, buf, sizeof(buf) - 1,
					 0); // 留 1 字节给 '\0'

			if (n > 0) {
				// 读到数据：n 是本次读到的字节数
				buf[n] = '\0'; // 手动补上字符串结束符，方便
					       // printf
				printf("[线程 %lx] 从 fd %d 收到: %s",
				       pthread_self(), client_fd, buf);
				send(client_fd, buf, n,
				     0); // 回显：把收到的原样发回去
			} else if (n == 0) {
				// 返回 0 表示对端正常关闭连接（收到 FIN）
				printf("客户端 fd %d 断开连接\n", client_fd);
				close(client_fd); // 关闭 fd，释放资源
				is_closed = 1;
				break;
			} else {
				// n < 0，说明出错，需要看 errno
				// 区分“暂时没数据”还是“真错误”
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					// 非阻塞 fd
					// 没有更多数据可读了（这才是边缘触发下“读完”的正常退出）
					break;
				}
				// 其他 errno 才是真正的网络错误
				perror("recv 出错");
				close(client_fd);
				is_closed = 1;
				break;
			}
		}

		// 核心关键：数据处理完后，如果连接还没断，立刻重置 EPOLLONESHOT
		// 否则这个 fd 因为 ONESHOT
		// 已经被“冻结”，后续数据不会再有事件通知。
		if (!is_closed) {
			reset_oneshot(g_epoll_fd, client_fd);
		}
	}
	return NULL;
}
// --- 3. 主线程：只负责 epoll_wait 监听事件 ---
// 架构：主线程（Reactor）只做“监听 + 分发”，真正的 IO 读写交给工作线程池，
// 这样单个线程不会被某个慢连接卡住，充分利用多核。
int main()
{
	queue_init(&queue); // 初始化任务队列

	// 启动 4 个工作线程
	pthread_t tids[THREAD_NUM];
	for (int i = 0; i < THREAD_NUM; i++) {
		pthread_create(&tids[i], NULL, worker_thread, NULL);
	}

	// 创建监听 socket
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	// SO_REUSEADDR：允许服务器重启时立刻复用刚释放的端口，避免“Address
	// already in use”
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// 填写监听地址：INADDR_ANY 表示监听本机所有网卡，端口转成网络字节序
	struct sockaddr_in addr = {.sin_family = AF_INET,
				   .sin_addr.s_addr = INADDR_ANY,
				   .sin_port = htons(PORT)};
	bind(listen_fd, (struct sockaddr *)&addr,
	     sizeof(addr)); // 绑定地址和端口
	listen(listen_fd, 128); // 开始监听，128 是等待连接队列长度

	// 创建 epoll 实例
	g_epoll_fd = epoll_create1(0);

	// 将 listen_fd 注册到 epoll（listen_fd 不需要 ONESHOT）
	// listen_fd 只关心“有新连接可 accept”，它会被反复触发，所以不能用
	// ONESHOT。
	struct epoll_event ev;
	ev.events = EPOLLIN; // 只监听可读事件（新连接到来）
	ev.data.fd = listen_fd;
	epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev); // ADD：新增注册项

	printf("服务器启动，监听端口: %d\n", PORT);

	struct epoll_event
		events[MAX_EVENTS]; // 存放 epoll_wait 返回的就绪事件数组
	while (1) {
		// 阻塞等待事件；返回就绪的事件个数 nready（-1
		// 表示无限等待）正常情况下不会返回-1，返回-1说明被打断了
		int nready = epoll_wait(g_epoll_fd, events, MAX_EVENTS, -1);

		for (int i = 0; i < nready; i++) {
			int cur_fd = events[i].data.fd; // 当前就绪的 fd

			if (cur_fd == listen_fd) {
				// 有新连接建立
				struct sockaddr_in client_addr;
				socklen_t len = sizeof(client_addr);
				int client_fd = accept(
					listen_fd,
					(struct sockaddr *)&client_addr, &len);

				set_nonblocking(client_fd); // 客户端 fd
							    // 也必须设为非阻塞

				// 核心：注册新客户端时，必须带上 EPOLLET 和
				// EPOLLONESHOT
				// EPOLLET：边缘触发，减少事件通知次数，但要求一次把数据读干净；
				// EPOLLONESHOT：保证同一时刻只有一个线程在处理这个
				// fd，避免多线程同时 recv 同一连接。
				ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
				ev.data.fd = client_fd;
				epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, client_fd,
					  &ev);
			} else {
				// 客户端发来了数据，主线程绝对不
				// recv，直接丢给工作线程队列
				// 主线程只负责“通知”，真正耗时的读写由工作线程完成，保持事件循环高效。
				queue_push(&queue, cur_fd);
			}
		}
	}

	close(listen_fd); // 理论上不会执行到这里（死循环）
	return 0;
}