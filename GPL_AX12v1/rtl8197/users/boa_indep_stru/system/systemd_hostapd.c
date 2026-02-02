#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "systemd_common.h"

/*
	In this file, the following functions
	will obtain hostapd/wpa_supplicant events from systemd_action,
	and send buffer to systemd.
*/

int init_hostapd_action_socket(int *fd)
{
	struct sockaddr_un s_addr;
	int reuse_addr = 1, ret = 0;
	int backlog = 2;
	int sockfd = socket( AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0 );

	if(sockfd < 0)
	{
		printf("socket create error!\n");
		return -1;
	}
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,&reuse_addr, sizeof(reuse_addr));

	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sun_family = AF_UNIX;
	strcpy(s_addr.sun_path, HOSTAPD_ACTION_PATH);

	ret = bind(sockfd, (struct sockaddr *)&s_addr, sizeof(struct sockaddr_un));
	if(ret < 0)
	{
		printf("### bind error!(%s) ###\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	ret = listen( sockfd, backlog );
	if(ret == -1)
	{
		printf("### listen error!(%s) ###\n", strerror(errno));
		return -1;
	}

	*fd = sockfd;
	return 0;
}

int recv_hostapd_action(int sockfd, char *buf, int buf_len)
{
	struct sockaddr_un clientaddr;
	int clientlen = sizeof(clientaddr);
	char recv_buf[HOSTAPD_RECV_MAX_DATA_SIZE];
	int recvbyte = 0;
	int client_fd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientlen);
	//setsockopt(client_fd,SOL_SOCKET,SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));
	recvbyte = recv(client_fd, recv_buf, HOSTAPD_RECV_MAX_DATA_SIZE-1, 0);
	if(recvbyte>0)
	{
		//to something
		//fprintf(stderr, "get something!\n");
		//fprintf(stderr, "EVENT: %s\n", recv_buf);
		snprintf(buf, buf_len, "%s", recv_buf);
	}
	close(client_fd);
}

int exit_hostapd_action_socket(int fd)
{
	close(fd);
	return 0;
}

int hostapd_action_client_conn(char *sun_path)
{
	int sock_client_fd;

	int len, rval;

	struct sockaddr_un un;
	sock_client_fd = socket( AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0 ) ;
	if(sock_client_fd < 0)
	{
		fprintf(stderr,"smart client socket create error fail, pid %d !\n", getpid());
		perror ("socket");
		return -1;
	}
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;

	strcpy(un.sun_path, sun_path);

	if (connect(sock_client_fd, (struct sockaddr *)&un, sizeof(struct sockaddr_un)) < 0)
	{
		fprintf(stderr,"connect is failure \n\n");
		close(sock_client_fd);
		return -1;
	}
	else
	{
		return sock_client_fd;
	}
	return 0;
}

int hostapd_action_send_client_msg(int sockfd, char *buf, int size)
{
	//char tmp_send[HOSTAPD_RECV_MAX_DATA_SIZE] = {0};
	int sendbytes = 0;
	int send_count = 0;
	int totalsend = 0;

	//fprintf(stderr,"%s:%d: send info:size [%d]\n",__FUNCTION__, __LINE__,size);
	//while(totalsend < size)
	{
		if((sendbytes = send(sockfd, buf, size, 0)) < 0)
		{
			fprintf(stderr,"smart client::Failed to send client,error:%s, pid %d \n", strerror(errno), getpid());
			close(sockfd);
			return -1;
		}
		//totalsend += sendbytes;
		//fprintf(stderr,"%d bytes send OK!\n", totalsend);

	}
	//fprintf(stderr,"%s:%d:  send success,sendbyte:%d\n",__FUNCTION__, __LINE__,sendbytes);
	return 0;

}
