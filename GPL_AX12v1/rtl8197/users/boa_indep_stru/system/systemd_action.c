#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>

#include "systemd_common.h"

#define READ_BUF_SIZE	50

static pid_t _rtk_find_pid_by_name( char* pidName)
{
	DIR *dir;
	struct dirent *next;
	pid_t pid;

	if ( strcmp(pidName, "init")==0)
		return 1;

	dir = opendir("/proc");
	if (!dir) {
		printf("Cannot open /proc");
		return 0;
	}

	while ((next = readdir(dir)) != NULL) {
		FILE *status;
		char filename[READ_BUF_SIZE];
		char buffer[READ_BUF_SIZE];
		char name[READ_BUF_SIZE];

		/* Must skip ".." since that is outside /proc */
		if (strcmp(next->d_name, "..") == 0)
			continue;

		/* If it isn't a number, we don't want it */
		if (!isdigit(*next->d_name))
			continue;

		sprintf(filename, "/proc/%s/status", next->d_name);
		if (! (status = fopen(filename, "r")) ) {
			continue;
		}
		if (fgets(buffer, READ_BUF_SIZE-1, status) == NULL) {
			fclose(status);
			continue;
		}
		fclose(status);

		/* Buffer should contain a string like "Name:   binary_name" */
		sscanf(buffer, "%*s %s", name);
		if (strcmp(name, pidName) == 0) {
		//	pidList=xrealloc( pidList, sizeof(pid_t) * (i+2));
			pid=(pid_t)strtol(next->d_name, NULL, 0);
			closedir(dir);
			return pid;
		}
	}
	closedir(dir);
	return 0;
}

/*
	In this file, the following functions
	will recv event from hostapd_cli or wpa_cli
*/

int main (int argc, char *argv[])
{
	char buf[HOSTAPD_RECV_MAX_DATA_SIZE]={0};
	int sockfd = 0;
	int i=0;
#if defined(CONFIG_APP_MULTI_AP)
	int map_sockfd = 0;
#endif

	if (!argv[1]){
		return 0;
	}

	for(i=1; i<argc; i++){
		if(strstr(argv[i], "global"))
			continue;
		strncat(buf, argv[i], (HOSTAPD_RECV_MAX_DATA_SIZE-1) - strlen(buf));
		if(i!=argc)
			strncat(buf, " ", (HOSTAPD_RECV_MAX_DATA_SIZE-1) - strlen(buf));
	}

	sockfd		= hostapd_action_client_conn(HOSTAPD_ACTION_PATH);
#if defined(CONFIG_APP_MULTI_AP)
	if((_rtk_find_pid_by_name("map_controller") != 0) || (_rtk_find_pid_by_name("map_agent") != 0))
		map_sockfd	= hostapd_action_client_conn(HOSTAPD_MAP_ACTION_PATH);
	else
	{
		map_sockfd = -1;
	}
#endif

	if(sockfd < 0) {
		perror("systemd socket error\n");
	}
	else{
		/* send event to systemd */
		hostapd_action_send_client_msg(sockfd, buf, sizeof(buf));
		close(sockfd);
	}
#if defined(CONFIG_APP_MULTI_AP)
	if(map_sockfd < 0) {
		//perror("map socket");
	}
	else{
		hostapd_action_send_client_msg(map_sockfd, buf, sizeof(buf));
		close(map_sockfd);
	}
#endif

	return 0;
}

