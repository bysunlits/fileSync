//
// Created by wx on 2026/10/16.
//
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
#include <sys/un.h>
#include <logger.h>

#define MAX_EVENTS 64
#define THREAD_NUM 4
#define QUEUE_SIZE 1024
#define UNIX_SOCKET "/tmp/upload.sock"


typedef struct {
    int fds[QUEUE_SIZE];
    int head, tail,
            count;
    pthread_mutex_t lock;
    pthread_cond_t
            not_empty;
} TaskQueue;

TaskQueue queue;
int g_epoll_fd;

void pop_queue(TaskQueue *q) {
    pthread_mutex_lock(&q->lock);
    while (q->count == 0) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }
    q->head = (q->head + 1);
    q->count--;
    pthread_mutex_unlock(&q->lock);
}

void queue_push(TaskQueue *q) {
    pthread_mutex_lock(&q->lock);
    while (q->count <= 0) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }
    q->tail = q->tail + 1 % QUEUE_SIZE;
    q->count++;
    pthread_mutex_unlock(&q->lock);
}

void queue_init(TaskQueue *q) {
    pthread_mutex_lock(&q->lock);
    q->tail = q->head = q->count = 0;
    pthread_cond_init(&q->not_empty, NULL);
    pthread_mutex_init(&q->lock, NULL);
}

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0); // 取出原来的所有配置
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK); //  叠加上“非阻塞”标签后写回
}

void reset_oneshot(int epoll_fd, int fd) {
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET |
                EPOLLONESHOT; // 重新监听可读，仍用边缘触发 + 一次性
    ev.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev); // MOD：修改已有注册项
}

void *worker(void *arg) {
    ;;
}

void thread_init() {
    queue_init(&queue);

    pthread_t tid[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_create(&tid[i], NULL, worker, NULL);
    }

    struct sockaddr_un listen_addr;
    socklen_t listen_len = sizeof(listen_addr);
    int listen_fd;
    memset(&listen_fd, '\0', sizeof(listen_fd));
    if (sizeof(UNIX_SOCKET) < sizeof(listen_addr.sun_path)) {
        strncpy(listen_addr.sun_path, UNIX_SOCKET, sizeof(listen_addr.sun_path) - 1);
    }
    listen_addr.sun_family = AF_UNIX;

//    int opt = 1;
//    setsockopt(listen_fd, SOL_SOCKET,SO_REUSEADDR,&opt,sizeof (opt));//Unix socket needn't prevent the TCP/IP socket reuse;
    unlink(UNIX_SOCKET);

    if (bind(listen_fd, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) < 0) {
        LOG_DEBUG("Bind Failed.");
    }

    int baklog = 1024;
    listen(listen_fd, baklog);

    g_epoll_fd = epoll_create1(
            EPOLL_CLOEXEC);//EPOLL_CLOEXEC 是epoll_create1() 的 flags 参数，当前 Linux 只支持这一个合法取值（可传 0）。它的作用等价于在返回的 epoll fd 上设置了 FD_CLOEXEC 标志，即：当进程调用 execve() 家族函数成功替换掉当前进程映像时，这个 fd 会被内核自动关闭，不会泄漏给新程序。注意它只在 exec 成功时生效；fork() 之后子进程里 fd 仍然是打开的（子进程可自己 close）
    struct epoll_event ev;
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN;
    epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev);

    while (1) {
        int nready = epoll_wait(g_epoll_fd, &ev, MAX_EVENTS, -1);//timeout==-1 表示无限期阻塞直到有线程就绪

        if (nready > 0) {
            for (int i = 0; i < nready; i++) {
                int cur_fd = ev.data.fd;

                if (cur_fd == listen_fd) {
                    int client_fd = accept(listen_fd, (struct sockaddr *) &listen_addr,&listen_len);
                    

                } else {

                }
            }
        } else if (nready == 0) {
            continue;
        } else {
            if (errno == EINTR) {
                continue;
            }
            int saved = errno;
            LOG_ERROR("Unknown error,errno is %d", saved);
        }
    }

}
