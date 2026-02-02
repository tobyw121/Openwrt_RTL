/*
 * For dslite 
 * Copyright(c) 2019 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * Author  : Wu Zexuan <wuzexuan@tp-link.com.cn>
 * Detail  : When the domain of the dslite has more than one ipv6 address,
 *           check the online-test result to confirm the address is appropriate.
 * Version : 1.0
 * Date    : 11 June, 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define DNSREPLYFILE  "/tmp/dslt-dnsreply"
#define DNSREPLYUSED  "/tmp/dslt-dnsreplyUsed"

#define CMD_LEN      512
#define DOMAIN_LEN   30 
#define BUFLEN_512   512
#define IFNAME_LEN   20
#define IP6_ADDR_LEN 40
#define SLEEP_TIME   30

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

char dslite_ifname[IFNAME_LEN] = {0};
char local_address[IP6_ADDR_LEN] = {0};
char wanv6_ifname[IFNAME_LEN] = {0};
char cfg[IFNAME_LEN] = {0};
char server[DOMAIN_LEN] = {0};
int mtu;

int get_online_test();
void execSystem(const char *fmt, ...);
void change_dslite_tunnel(char *remote_address);
void get_dns6_server(char *dns1, char *dns2);
void dnsreplyfile_update();
static void _usage(void);

static void _usage(void)
{
    puts("Usage: dslite -i DSLITE_IFNAME -m MTU -l LOCAL_ADDRESS -6 WANV6_IFNAME -s AFTR_NAME WAN_IFNAME");
}


/* 
 * fn			void util_execSystem(const char *call, const char *fmt, ...) 
 * brief		format cmd string and exec cmd
 *
 * param[in]	call  - the name of function using this call
 * param[in]	fmt - cmd string(variable length parameter)
 *
 * return		N/A.	
 */
void execSystem(const char *fmt, ...) 
{
	int size = 0;
	char cmd[CMD_LEN] = {0};
	va_list args;

	va_start(args, fmt);
	size = vsnprintf(cmd, CMD_LEN - 1, fmt, args);
	va_end(args);

	if (size > 0)
	{
		if (-1 == system(cmd))
		{
			perror("util_execSystem call error:");
		}
	}
	return;
}


/* 
 * fn			int get_online_test()
 * brief		read the online-test result
 *
 * return		1--online; 0--offline	
 */
int get_online_test()
{
	pid_t status = system("online-test");
	if (0 == WEXITSTATUS(status))
	{
		return 1;
	}
	else if (1 == WEXITSTATUS(status))
	{
		return 0;
	}
	else
	{
		status = system("online-test");
		if (0 == WEXITSTATUS(status))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}	
}

/* 
 * fn			change_dslite_tunnel(char *remote_address)
 * brief		change dslite tunnel parameters
 * param[in]	remote_address  - the remote address of the tunnel
 *
 * return		N/A	
 */
void change_dslite_tunnel(char *remote_address)
{
	execSystem("/lib/netifd/dslite-up.sh %s %d %s %s %s %s", 
			   dslite_ifname, mtu, local_address, remote_address, wanv6_ifname, cfg);
}

void get_dns6_server(char *dns1, char *dns2)
{
	int j;
	int num = 1;
	char *str = NULL;
	char *token = NULL;
	char *saveptr1 = NULL;
	char *dns = NULL;
	FILE *fp = NULL;
	char line[100] = {0};
	
	fp = fopen("/tmp/resolv.conf", "r");
	if (fp != NULL)
	{
		while (fgets(line, sizeof(line), fp))
		{
			if (strchr(line, ':'))
			{
				for (j = 1, str = line; ; j++, str = NULL)
				{
					token = strtok_r(str, " ", &saveptr1);
						
					if (token == NULL)
						break;
					if (j == 2)
						dns = token;
				}
				if (dns[strlen(dns) - 1] == '\n')
				{
					dns[strlen(dns) - 1] = '\0';
				}
				if (num == 1)
				{
					strncpy(dns1, dns, IP6_ADDR_LEN - 1);
					dns1[IP6_ADDR_LEN - 1] = '\0';
				}
				if (num == 2)
				{
					strncpy(dns2, dns, IP6_ADDR_LEN - 1);
					dns2[IP6_ADDR_LEN - 1] = '\0';
				}
				num++;
			}
		}
		fclose(fp);
	}
}

void dnsreplyfile_update()
{
	char dns1[IP6_ADDR_LEN] = {0};
	char dns2[IP6_ADDR_LEN] = {0};

	get_dns6_server(dns1, dns2);
	if (strlen(dns1) || strlen(dns2))
	{
		execSystem("dnslookup -6 %s %s,%s > %s", server, dns1, dns2, DNSREPLYFILE);
	}
}

int main(int argc, char *argv[])
{
	const char *optstring = "i:m:l:6:s:h";
    int opt;
	int offline_times = 0;
	FILE *fp = NULL;
	FILE *fpUsed = NULL;
	char confStr[BUFLEN_512] = {0};
	int nBytes = 0;
	char addr_str[IP6_ADDR_LEN] = {0};
	char flag = FALSE;

	while ((opt = getopt(argc, argv, optstring)) != -1)
	{
		switch (opt)
		{
			case 'i':
			    strncpy(dslite_ifname, optarg, IFNAME_LEN - 1);
				dslite_ifname[IFNAME_LEN - 1] = '\0';
				break;

			case 'm':
				mtu = strtoul(optarg, NULL, 10);
				break;

			case 'l':
			    strncpy(local_address, optarg, IP6_ADDR_LEN - 1);
				local_address[IP6_ADDR_LEN - 1] = '\0';
				break;

			case '6':
			    strncpy(wanv6_ifname, optarg, IFNAME_LEN - 1);
				wanv6_ifname[IFNAME_LEN - 1] = '\0';
				break;

			case 's':
			    strncpy(server, optarg, DOMAIN_LEN - 1);
				server[DOMAIN_LEN - 1] = '\0';
				break;

			case 'h':
				_usage();
				exit(0);

			default:
				_usage();
				exit(1);
		}
	}
	
	if (optind >= argc)
    {
        _usage();
        exit(1);
    }
    strncpy(cfg, argv[optind++], IFNAME_LEN - 1);
	cfg[IFNAME_LEN - 1] = '\0';
	
	while (1)
	{
		if (get_online_test() == 0)
			offline_times++;
		else
			offline_times = 0;
		
		if (offline_times == 3)
		{
			offline_times = 0;
			
			fpUsed = fopen(DNSREPLYUSED, "r");
	
			if (fpUsed != NULL)
			{
				/* read the current config content */
				if ((nBytes = fread(confStr, sizeof(char) * BUFLEN_512, 1, fpUsed)) < 0)
				{
					printf("read file: %s error: %s\n", DNSREPLYUSED, strerror(errno));
				}
				fclose(fpUsed);
			}

			dnsreplyfile_update();
			fp = fopen(DNSREPLYFILE, "r");
			if (fp == NULL)
			{
				printf("open file: %s error\n", DNSREPLYFILE);
				return 0;
			}
			
			flag = FALSE;
			while (fgets(addr_str, IP6_ADDR_LEN, fp))
			{
				if (addr_str[strlen(addr_str)-1] == '\n')
				{
					addr_str[strlen(addr_str)-1] = '\0';
				}
				//fread /tmp/dslt-dnsreply compare with /tmp/dslt-dnsreplyUsed 
				if (strstr(confStr, addr_str) == NULL)
				{
					flag = TRUE;
					change_dslite_tunnel(addr_str);
					fpUsed = fopen(DNSREPLYUSED, "w");
					
					if (fpUsed != NULL)
					{
						fprintf(fpUsed, "%s%s\n", confStr, addr_str);
						fclose(fpUsed);
					}
					break;
				}
			}
			fclose(fp);
			
			if(FALSE == flag)
			{
				execSystem("rm -r %s", DNSREPLYUSED);
				memset(confStr, 0, sizeof(confStr));
			}
		}
		sleep(SLEEP_TIME);
	}
	return 0;
}

