#pragma once


typedef struct recv_client {
    int fd;
    struct sockaddr_un addr;
}recv_client;

int create_connection(struct sockaddr_un server_addr);
int client_accept(int server_fd,struct sockaddr_un server_addr,recv_client client,ssize_t bytes_read,char buffer[]);