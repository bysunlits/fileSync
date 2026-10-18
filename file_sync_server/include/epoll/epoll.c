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

#define PORT 8888
#define MAX_EVENTS 64
#define THREAD_NUM 4
#define QUEUE_SIZE 1024


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

void pop_queue(TaskQueue *q)
{
	pthread_mutex_lock(&queue.lock);

}

int set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0); // 取出原来的所有配置
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK); //  叠加上“非阻塞”标签后写回
}

void reset_oneshot(int epoll_fd, int fd)
{
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET |
		    EPOLLONESHOT; // 重新监听可读，仍用边缘触发 + 一次性
	ev.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev); // MOD：修改已有注册项
}

void thread_init()
{

}
