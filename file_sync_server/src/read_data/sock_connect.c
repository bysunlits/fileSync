#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <logger.h>
#include "sock_connect.h"
#define PORT 2715
#define SOCKET_PATH "/tmp/service.sock"
#define BUFFER_SIZE 4096


int create_connection(struct sockaddr_un server_addr){

	int server_fd;

	server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_fd < 0) {
		LOG_ERROR("Socket creation failed:%s",strerror(errno));
		return -1;
	}

	unlink(SOCKET_PATH);
	memset(&server_addr, 0,
	       sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	strncpy(server_addr.sun_path, SOCKET_PATH,
	sizeof(server_addr.sun_path) - 1);
	if(bind(server_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
		LOG_ERROR("Bind failed:%s",strerror(errno));
		close(server_fd);
		return -1;
	}

	const int backlog = 1024;
	listen(server_fd,backlog);
	LOG_INFO("C TCP Server is listening on :%s:%.*s\n",
	       (server_addr.sun_family == AF_UNIX) ? "AF_UNIX" : "UNKNOWN",
	       (int)sizeof(server_addr.sun_path), server_addr.sun_path);
	return 0;
}

int client_accept(int server_fd,struct sockaddr_un server_addr,recv_client client,ssize_t bytes_read,char buffer[]){
	while (1) {
		socklen_t client_addr_len=sizeof(client.addr);
		client.fd = accept(server_fd, (struct sockaddr *)&client.addr,&client_addr_len);
		if (client.fd < 0) {
			LOG_DEBUG("Accept failed:%s",strerror(errno));
			continue;
		}
		LOG_INFO("New connection established,from Nginx:%s:%.*s\n",
		       (client.addr.sun_family == AF_UNIX) ? "AF_UNIX"
							   : "UNKNOWN",
		       (int)sizeof(client.addr.sun_path), client.addr.sun_path);

		while ((bytes_read = recv(client.fd, buffer, sizeof(buffer) - 1,
					  0)) > 0) {
			buffer[bytes_read] = '\0';
			LOG_DEBUG("Receive %zd bytes:\n%s\n", bytes_read, buffer);
		}
		if (bytes_read == 0) {
			LOG_DEBUG(("Client/Nginx closed connection\n"));
		} else if (bytes_read < 0) {
			LOG_ERROR("recv error:%s",strerror(errno));
			return -1;
		}
	}
}
