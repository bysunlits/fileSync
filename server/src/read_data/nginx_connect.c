//
// Created by sunlit on 2026/9/15.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include  <sys/socket.h>
#include  <arpa/inet.h>

#define PORT 2715
// #define SOCKET_PATH "/tmp/service.sock"
#define SOCKET_PATH "127.0.0.1"
#define BUFFER_SIZE 4096
int main()
{
	int server_fd,client_fd;
	struct sockaddr_in server_addr,client_addr;
	socklen_t client_len=sizeof(client_addr);
	char buffer[BUFFER_SIZE];

	server_fd=socket(AF_INET,SOCK_STREAM,0);
	if (server_fd<0)
	{
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	int opt=1;
	setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	//SOL_SOCKET indicates that there are general options in the socket layer.
	//SO_REUSEADDR:The name of specific option that to be set.SO_REUSEADDR indicates that address reuse is allowed.
	//&opt:A pointer to the option value,usually be set as a integer.1 indicates on,0 off.

	memset(&server_addr,0,sizeof(server_addr));//Clear the space of server_addr to 0;
	server_addr.sin_family=AF_INET;//sin_family stands for the address family,tells the kernel which protocol want to use.AF_INET is IPv4.
	server_addr.sin_addr.s_addr=inet_addr(SOCKET_PATH);
	//s_addr is the only variable in the sin_addr substructure that actually stores data.It is a 32-bit integer(uint_t) representing an IPv4 address that in binary format.

	server_addr.sin_port=htons(PORT);//hton is host to network
	if (bind(server_fd,(struct sockaddr*)&server_addr,client_len))//bind a socket file descriptor to a local net address.
	{
		perror("Bind failed");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	const int backlog=1024;
	listen(server_fd,backlog);//Convert this active socket to passive socket so that it can accept requests from client.The backlog is the maximum number of the connection in the queue before being processed by accept();

	printf("C TCP Server is listening on %s:%d...\n",SOCKET_PATH,PORT);

	while (1)
	{
		client_fd=accept(server_fd,(struct sockaddr *)&client_addr,&client_len);
		if (client_fd<0)
		{
			perror("Accept failed");
			continue;
		}

		printf(("New connection established,from Nginx:%s:%d\n"),inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

		ssize_t bytes_read;
		while ((bytes_read=recv(client_fd,buffer,sizeof(buffer)-1,0))>0)
		{
			buffer[bytes_read]='\0';
			printf("Receive %zd bytes:\n%s\n",bytes_read,buffer);
		}
		if (bytes_read==0)
		{
			printf(("Client/Nginx closed connection\n"));
		}else if (bytes_read<0)
		{
			perror("recv error.");
		}
		close((client_fd));
	}
	close(server_fd);
	return 0;

}