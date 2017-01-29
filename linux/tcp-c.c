#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVPORT 1707 /*server port number */
#define BACKLOG 10 /*max link*/
#define MAX_MESG_LEN 1024  /*max message len*/

int main(int argc, char *argv[])
{
	int sockfd;
	int recvbytes;
	char hostname[1000];
	char buf[MAX_MESG_LEN];
	struct hostent *host;
	struct sockaddr_in serv_addr;
	//printf("Please enter the server's hostname!\n");
	//scanf("%s",hostname);
	if((host=gethostbyname("192.168.152.6"))==NULL) 
	{
		perror("gethostbyname出错!");
		exit(1);
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket创建出错!");
		exit(1);
	}
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERVPORT);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero( &(serv_addr.sin_zero),8);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("connect出错!");
		exit(1);
	}
	if (send(sockfd, "Hello, you are connected!", MAX_MESG_LEN, 0) == -1)
	{
		perror( "send出错!");
	}
	if ((recvbytes=recv(sockfd, buf, MAX_MESG_LEN, 0)) ==-1) 
	{
		perror("recv出错!");
		exit(1);
	}
	buf[recvbytes] = '\0';
	printf( "Received: %s",buf);//*/
	close(sockfd);
	return 0;
}
