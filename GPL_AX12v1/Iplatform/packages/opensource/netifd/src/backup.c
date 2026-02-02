/*! Copyright(c) 2008-2014 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     detect_online.c
 *\brief	Only apply to ipv4 temporarily.
 *\details
 *
 *\author   Feng JiaFa <fengjiafa@tp-link.net>
 *\version  1.0.0
 *\date     19Nov14
 *
 *\warning
 *
 *\history \arg 19Nov14, Teng Fei, create the file.
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>		/* fcntl() */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <uci.h>

#include "netifd.h"
#include "interface.h"
#include "interface-ip.h"
#include "proto.h"
#include "connect.h"
#include "config.h"
#include "system.h"
#include "backup_config.h"
#include "usbmodem_log.h"

#include <arpa/inet.h>

/*
**	Detection states.
**	Do NOT modify values of following vars unless you understand it!
##	=======================================================
##	Actions [State1 -> State2]	Contitions...
##	---------------------------------------------------------------------
1.	a [* -> 6]	Non automatic connection OR non connected OR an error happened.
2.	b [0 -> 4]	Get info of connected status,  initialize some resources, try to begin 
				to prepare for the online detection.
3.	c [4 -> 1]	If the current attempt isn't the last attempt, will begin to start online
				detection.
4.	d [4 -> 5]	If you have already completed the last attempt, enter Offine state to
				terminate the "bad" connection.
5.	e [1 -> 2]	It has sent a request package of some protocol, waiting for response
				from the peer to detect the link state.
6.	f [1 -> 4]	The current attempt way cannot start, enter the next attempt.
7.	g [2 -> 2]	Timeout to receive a response from the peer, continue to do it again!
8.	h [2 -> 3]	Has received a GOOD response from the peer, enter Online state, 
				implies current connection is well.
9.	i [2 -> 4] 	Received a bad response from the peer, or received none after all 
				attempts, try to begin to prepare for the next time online detection.
10.	j [3 -> 0]	After sleeping, enter the next round of online detection.
11.	k [5 -> 0]	After sleeping, enter the next round of online detection.
12.	l [6 -> 0] 	Current connection state has changed to connected from disconnected
				begin to enter a new round of online detection.
##	=======================================================
*/
#define	D_INITIAL		0	/* initial state */
#define	D_STARTED		1	/* begin to do online-detect */
#define	D_DETECTING		2	/* waiting for result */
#define	D_ONLINE		3	/* connected */
#define	D_RETRY			4	/* retry to detect */
#define	D_OFFLINE		5	/* disconnected from ISP */
#define	D_OVER			6	/* idle, nothing to do */
#define	D_MAX			7	/* number of states */

/* ways for online-detect */
#define	DETECT_WAY_DNS	1
#define	DETECT_WAY_GW	2
#define	DETECT_WAY_CNT	4

#define	DETECT_DNS_MAX	2
#define	DETECT_DNS_PORT	0x35
#define	DETECT_ICMP_ECHO	0x8  /* Echo Request */

/* invalid socket fd implies to close current socket. */
#define	DETECT_SOCK_INVALID	-1

#define NETWORK_STABLE_LIMIT				15
#define NETWORK_STABLE_TIME_INTERVAL		0x18
#define NETWORK_NOT_STABLE_TIME_INTERVAL	0x3


/*  Online Detect Finite State Machine. */
struct detect_fsm
{
	unsigned	state;
	unsigned	timeunit;
	unsigned	times;
	unsigned	times_lmt;

	void (*cb)(struct detect_fsm*, unsigned*);
};

/* args */
struct detect_data
{
	int	af;
	union if_addr addr;
	struct interface *iface;
};

struct detect_ways
{
	unsigned	type;
	struct detect_data args;
	bool	done;
	
	int (*cb)(struct detect_data*, bool);
};

static void detect_initial(struct detect_fsm*, unsigned*);
static void detect_started(struct detect_fsm*, unsigned*);
static void detect_detecting(struct detect_fsm*, unsigned*);
static void detect_online(struct detect_fsm*, unsigned*);
static void detect_retry(struct detect_fsm*, unsigned*);
static void detect_offline(struct detect_fsm*, unsigned*);
static void detect_over(struct detect_fsm*, unsigned*);

/*
**	Detection handlers.
**	Do NOT modify values of "state" member in "struct detect_fsm" unless you understand it!
*/
static struct detect_fsm detect_hdlrs[D_MAX] =
{
	{
		.state = D_INITIAL,
		.timeunit = 0x1,
		.times = 0x0,
		.times_lmt = 0x1,
		.cb = detect_initial,
	},
	{
		.state = D_STARTED,
		.timeunit = 0x1,
		.times = 0x1,
		.times_lmt = 0x1,
		.cb = detect_started,
	},
	{
		.state = D_DETECTING,
		.timeunit = 0x1,
		.times = 0x1,
		.times_lmt = 0x3,
		.cb = detect_detecting,
	},
	{
		.state = D_ONLINE,
		.timeunit = 0x0,
		.times = 0x1,
		.times_lmt = 0x8,
		.cb = detect_online,
	},
	{
		.state = D_RETRY,
		.timeunit = 0x0,
		.times = 0x0,
		.times_lmt = 0x2,
		.cb = detect_retry,
	},
	{
		.state = D_OFFLINE,
		.timeunit = 0x0,
		.times = 0x1,
		.times_lmt = 0x8,
		.cb = detect_offline,
	},
	{
		.state = D_OVER,
		.timeunit = NETWORK_NOT_STABLE_TIME_INTERVAL,
		.times = 0x0,
		.times_lmt = 0x1,
		.cb = detect_over,
	},
};

static struct detect_ways detect_way[DETECT_WAY_CNT];

/* Timer to do online-detect. */
static struct uloop_timeout detect_timer;

static struct interface *detect_if = NULL;

static struct nw_backup_cfg curr_backup_cfg;

static bool set_ifname(struct interface *iface, char* buf, int buf_size)
{
	bool ret = false;
	if (buf == NULL)
	{
		goto exit;
	}

	memset(buf, 0, buf_size);
	if (iface->l3_dev.dev && buf_size > strlen(iface->l3_dev.dev->ifname))
	{
		strcpy(buf, iface->l3_dev.dev->ifname);
		ret = true;
	}
	else if (iface->ifname && buf_size > strlen(iface->ifname))
	{
		strcpy(buf, iface->ifname);
		ret = true;
	}

exit:
	return ret;
}

static void set_next_detect_if ()
{
	enum network_type target = NETWORK_TYPE_NONE;

	if (curr_backup_cfg.detecting_nw == NETWORK_TYPE_NONE)
	{
		target = curr_backup_cfg.primary_nw;
	}
	else if (curr_backup_cfg.detecting_nw == curr_backup_cfg.primary_nw)
	{
		target = curr_backup_cfg.secondary_nw;
	}

	curr_backup_cfg.detecting_nw = target;

	return ;
}

static void switch_network_type(enum network_type type, struct interface *iface)
{
	struct interface_ip_settings *ip = NULL;
	struct device_route *route = NULL;
	char wan_ifname[30];
	char buf[0x80];
	int j = 0;
	int found_dft_route = 0;

	if (iface == NULL)
	{
		return;
	}

	if (curr_backup_cfg.current_nw != type)
	{
		curr_backup_cfg.stable = 0;
		curr_backup_cfg.current_nw = type;
		memset(buf, 0, sizeof(buf));

		for (ip = &iface->proto_ip, j = 0; j < 0x2; ip = &iface->config_ip, ++j)
		{
			if(found_dft_route)
				break;

			vlist_for_each_element(&ip->route, route, node) 
			{
				if (!route->enabled)	
					continue;
				
				if ((route->flags & DEVADDR_FAMILY) != DEVADDR_INET4)
					continue;

				if (route->nexthop.in.s_addr == 0)
					continue;

				if (route->addr.in.s_addr != 0)
					continue;

				inet_ntop(AF_INET, &route->nexthop, buf, sizeof(buf));
				found_dft_route = 1;
				break;
			}
		}

		//wan_ifname = config_option_str(iface->name, "ifname");
		
		if (strlen(buf) > 0 && set_ifname(iface, wan_ifname, sizeof(wan_ifname)) &&
			set_ubus_curr_nw(type) == 0)
		{
			system_exec_fmt("ip route replace default via %s dev %s", buf, wan_ifname);
#ifdef SUPPORT_VPN_CLIENT_IN_3G4G_MODULES
			set_vpn_client_parent(iface->name);
#endif
		}
		else
		{
			/* fail to set default route */
		}
	}
	else
	{
		curr_backup_cfg.stable = (curr_backup_cfg.stable > 2 * NETWORK_STABLE_LIMIT) ?
								curr_backup_cfg.stable : curr_backup_cfg.stable + 1;
	}

	return;
}

static struct interface * target_network_up(enum network_type type)
{
	struct interface *iface = NULL;
	vlist_for_each_element(&interfaces, iface, node)
	{
		if (get_network_type(iface->name) != type)
		{
			continue;
		}

		if (iface->state == IFS_UP)
		{
			return iface;
		}
	}
	return NULL;
}

static void backup_prefer_only()
{
	enum network_type type = curr_backup_cfg.primary_nw;
	struct interface *iface = NULL;
	int count = 0;

again:
	if ((iface = target_network_up(type)) != NULL)
	{
		switch_network_type(type, iface);
	}
	else if (count < 1)	/* primary_nw is not up */
	{
		count++;
		type = curr_backup_cfg.secondary_nw;
		iface = NULL;
		goto again;
	}
	else	/* all network are not up, just set default nw for display */
	{
		set_ubus_curr_nw(curr_backup_cfg.primary_nw);
	}

	return;
}

static int backup_cfg_init()
{
	int ret = 0;
	ret = uci_bk_cfg_get(&curr_backup_cfg);
	if (ret != 0)
	{
		return ret;
	}

	switch (curr_backup_cfg.mode)
	{
	case CONN_MODE_MOBILE_ONLY:
	case CONN_MODE_MOBILE_PREFERRED:
		curr_backup_cfg.primary_nw = NETWORK_TYPE_MOBILE;
		curr_backup_cfg.secondary_nw = NETWORK_TYPE_WIRED;
		break;
	case CONN_MODE_CABLE_ONLY:
	case CONN_MODE_CABLE_PREFERRED:
	default:
		curr_backup_cfg.primary_nw = NETWORK_TYPE_WIRED;
		curr_backup_cfg.secondary_nw = NETWORK_TYPE_MOBILE;
		break;
	}
	curr_backup_cfg.detecting_nw = NETWORK_TYPE_NONE;
	curr_backup_cfg.detect_ret = D_OFFLINE;
	curr_backup_cfg.current_nw = NETWORK_TYPE_NONE;

	curr_backup_cfg.stable = 0;
	return ret;
}

/*
**	Get && Set index of Arrary detect_way.
*/
static int detect_gset_way_idx(const int newidx, bool set)
{
	static int detect_way_idx = 0;

	/* Set */
	if (set)
	{
		detect_way_idx = newidx;
	}

	return detect_way_idx;
}


/*
**	1. Get current socket fd if "set" false.
**	2. Record new socket fd and close current socket if newsock equals DETECT_SOCK_INVALID.
*/
static int detect_gset_sock(const int newsock, bool set)
{
	/* Socket fd for online-detect */
	static int detect_sockfd = DETECT_SOCK_INVALID;

	/* Just get current socket fd  OR nothing to do */
	if (!set || detect_sockfd == newsock)
	{
		return detect_sockfd;
	}

	/* Record new socket fd */
	if (detect_sockfd >= 0)
	{
		close(detect_sockfd);
	}

	detect_sockfd = newsock;
	return detect_sockfd;
}


static int 
detect_be_over(void)
{
	struct interface *iface = NULL;
	int ret = 0x2;

	iface = target_network_up(curr_backup_cfg.detecting_nw);
	if (iface)
	{
		detect_if = iface;
		return 0;
	}

	return ret;
}

static char * strncpy_IFNAMSIZ(char *dst, const char *src)
{
	return strncpy(dst, src, 16);
}
static int setsockopt_bindtodevice(int fd, const char *iface)
{
	struct ifreq ifr;
	strncpy_IFNAMSIZ(ifr.ifr_name, iface);
	return setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr));
}

static int 
detect_dns_query(struct detect_data* data, bool send)
{
	int sockfd = -1;
	struct sockaddr_in cli_addr;
	struct sockaddr_in serv_addr;

	int nbytes       = 0;
	char buf[128]    = {0};
	char rbuf[256]   = {0};
	char *ptr        = NULL;
	char *ptmp       = NULL;
	short one = 0, n = 0;

	if (!send)
	{
		goto recv_step;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = data->af;
	serv_addr.sin_addr = data->addr.in;
	serv_addr.sin_port = htons(DETECT_DNS_PORT); /* 53 */
	if ((sockfd = socket(data->af, SOCK_DGRAM, 0)) < 0)
	{
		return -1;
	}

	if (set_ifname(data->iface, rbuf, sizeof(rbuf)) == false)
	{
		close(sockfd);
		return -1;
	}
	if (setsockopt_bindtodevice(sockfd, rbuf) < 0)
	{
		close(sockfd);
		return -1;
	}

	memset(&cli_addr, 0, sizeof(cli_addr));
	cli_addr.sin_family = data->af;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port = htons(0);
	if (bind(sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) <0)
	{
		close(sockfd);
		return -2;
	}	

#if 0
	char xbuf[32],ybuf[256];
	memset(xbuf,0,256);
	inet_ntop(data->af, &data->addr, xbuf, 0x80);
	sprintf(ybuf,"echo [dns qry]%s >/dev/console",xbuf);
	system(ybuf);
#endif	

	ptr = buf;
	*((u_short *)ptr) = htons(1234);	/* id */
	ptr += 2;
	*((u_short *)ptr) = htons(0x0100);	/* flag */
	ptr += 2;
	*((u_short *)ptr) = htons(1);		/* number of questions */
	ptr += 2;
	*((u_short *)ptr) = htons(0);		/* number of answers RR */
	ptr += 2;
	*((u_short *)ptr) = htons(0);		/* number of Authority RR */
	ptr += 2;
	*((u_short *)ptr) = htons(0);		/* number of Additional RR */
	ptr += 2;

	// memcpy(ptr,"\001a\014root-servers\003net\000", 20);
	memcpy(ptr,"\007tp-link\003com\000", 13);
	ptr += 13;
	one = htons(1);
	memcpy(ptr, &one, 2);		/* query type = A */
	ptr += 2;
	memcpy(ptr, &one, 2);		/* query class = 1 (IP addr) */
	ptr += 2;

	nbytes = 29;
	if (sendto(sockfd, buf, nbytes, 0, (struct sockaddr *)&serv_addr, 
		sizeof(struct sockaddr_in)) != nbytes)
	{
		/* perror("sendto"); */
		close(sockfd);
		return -3;
	}

	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
	{
		/* perror("fcntl"); */
		close(sockfd);
		return -4;
	}

	detect_gset_sock(sockfd, true);
	return 0;

recv_step:

	sockfd = detect_gset_sock(sockfd, false);

	if (sockfd < 0)
	{
		return -3;
	}

	n = recvfrom(sockfd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)0, (socklen_t *)0);
	if (n == -1 && (errno == EAGAIN /*|| *perrno == EAGAIN*/))
	{
		return -1;
	}
	if (n >= 0)
	{
		/* good answer */
		/* If DNS response is longer than 256, the rbuf will be full and n will be 256, AVOID rbuf[256] */
		if (n >= 256)
		{
			rbuf[255] = 0;
		}
		else
		{
			rbuf[n] = 0;
		}

		ptmp = &rbuf[6];
		if (htons(*(short *)ptmp) >= 1) /* good we got an answer */
		{
			return 0;
		}
	}

	return -2;
}


#if	0

static int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return ans;
}

static int 
detect_gw_query(struct detect_data* data, bool send)
{
	int sockfd = -1;
	struct sockaddr_in pingaddr;
	ssize_t ret = 0;
	char sndPacket[192];
	struct icmp *sndPkt = NULL;

	struct icmp *rcvPkt = NULL;
	char rbuf[192];
	short n = 0;
	static short detect_icmp_id = 0x0;

	if (!send)
	{
		goto recv_step;
	}

	sockfd = socket(AF_INET, SOCK_RAW, 0x1);	/* 0x1 means ICMP */
	if (sockfd < 0)
	{
		return -1;
	}

	if (set_ifname(data->iface, rbuf, sizeof(rbuf)) == false)
	{
		close(sockfd);
		return -1;
	}
	if (setsockopt_bindtodevice(sockfd, rbuf) < 0)
	{
		close(sockfd);
		return -1;
	}

	memset(&pingaddr, 0x0, sizeof(struct sockaddr_in));
	pingaddr.sin_family = AF_INET;
	pingaddr.sin_addr = data->addr.in;

	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
    {
        close(sockfd);
        return -3;
    }
	
	detect_icmp_id &= 0x0fff;
	detect_icmp_id++;
	
	sndPkt = (struct icmp *)sndPacket;
	memset(sndPkt, 0x0, sizeof(sndPacket));
	sndPkt->icmp_type = DETECT_ICMP_ECHO;
	sndPkt->icmp_id = detect_icmp_id;
	sndPkt->icmp_cksum = in_cksum((unsigned short *)sndPkt, sizeof(sndPacket));
	n = sendto(sockfd, sndPkt, sizeof(sndPacket)/*0x40*//*DEFDATALEN + ICMP_MINLEN*/, 0x0, 
		(struct sockaddr *)&pingaddr, sizeof(struct sockaddr_in));
	if (n < 0)
	{
		close(sockfd);
		return -2;
	}

	//char buf[0x80];
	//inet_ntop(AF_INET, &data->addr, buf, 0x80);
	////tp_prtf("gw: %s sockfd%d", buf, sockfd);
	detect_gset_sock(sockfd, true);
	return 0;

recv_step:

	sockfd = detect_gset_sock(sockfd, false);
	if (sockfd < 0)
	{
		return -3;
	}

	memset(rbuf, 0, sizeof(rbuf));
	n = recvfrom(sockfd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)0, (socklen_t *)0);
	if (n == -1 && (errno == EAGAIN /*|| *perrno == EAGAIN*/))
	{
		return -1;
	}

	if (n >= 76)
	{
		struct iphdr *iphdr = (struct iphdr *)rbuf;			/* ip + icmp */
		rcvPkt = (struct icmp *)(rbuf + (iphdr->ihl << 2));	/* skip ip hdr */			
		if (rcvPkt->icmp_type == ICMP_ECHOREPLY && rcvPkt->icmp_id == detect_icmp_id)
		{
			ret = 0;
		}
		else
		{
			ret = -4;
		}

		while ((n = recvfrom(sockfd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)0, (socklen_t *)0)) >= 0)
		{
			/* drop the redundancy data... */
		}
		return ret;
	}

	return -2;
}
#endif

static void __set_curr_nw_status(enum network_type nt, bool on)
{
	int st  = 0;
	int bit = 0;
	int st_old = get_curr_nw_status();

	/*
	 * curr_nw_status = AB(2)
	 * A represent the network status of secondary_nw
	 * B represent the network status of primary_nw
	 * 0 represent OFFLINE, 1 represent ONLINE
	 */
	if (nt == curr_backup_cfg.primary_nw)
	{
		bit = 0;
	}
	else if (nt == curr_backup_cfg.secondary_nw)
	{
		bit = 1;
	}
	else
	{
		return;
	}

	if (on)	/* 1:online 0:offline  */
	{
		st = st_old | (1 << bit);
	}
	else
	{
		st = st_old & (~(1 << bit));
	}

	if ( (st & (1 << bit)) != (st_old  & (1 << bit)) )
	{
		set_curr_nw_status(st);
	}

	return;
}

/*
##	=======================================================
##	Actions [State1 -> State2]	Contitions...
##	---------------------------------------------------------------------
1.	a [0 -> 6]	Non automatic connection OR non connected OR an error happened.
2.	b [0 -> 4]	Get info of connected status,  initialize some resources, try to begin 
				to prepare for the online detection.
10.	j [3 -> 0]	After sleeping, enter the next round of online detection.
11.	k [5 -> 0]	After sleeping, enter the next round of online detection.
12.	l [6 -> 0] 	Current connection state has changed to connected from disconnected
				begin to enter a new round of online detection.
##	=======================================================
*/
static void
detect_initial(struct detect_fsm* fsm, unsigned* phase)
{
	struct interface *iface = detect_if;
	struct interface_ip_settings *ip = NULL;
	struct dns_server *dns = NULL;
	struct detect_ways *way = NULL;
	int i = 0;
	// char cmd[0x100];
	char buf[0x80];

	if (!iface || iface->state != IFS_UP)
	{
		*phase = D_OVER;
		return;
	}

	i = 0;

	if (iface->proto_handler && strcmp(iface->proto_handler->name, "static") == 0)
	{
		ip = &iface->config_ip;
	}
	else
	{
		ip = &iface->proto_ip;
		if (ip->no_dns)
		{
			ip = &iface->config_ip;
			if (ip->no_dns)
			{
				*phase = D_OVER;
				goto add_gw_detect;
			}
		}
	}

	vlist_simple_for_each_element(&ip->dns_servers, dns, node) 
	{
		if (dns->af != AF_INET || dns->addr.in.s_addr == 0)
		{
			continue;
		}
		way = detect_way + i;
		way->type = DETECT_WAY_DNS;
		way->args.af = dns->af;
		way->args.addr = dns->addr;
		way->args.iface = iface;
		way->done = false;
		way->cb = detect_dns_query;
		
		inet_ntop(dns->af, &dns->addr, buf, 0x80);
		i++;
		//tp_prtf("dns server%d: %s", i, buf);
#if 0		
		char xbuf[256];
		memset(xbuf,0,256);
		sprintf(xbuf,"echo dns server%d: %s >/dev/console", i, buf);
		system(xbuf);
#endif		
		if (i == DETECT_DNS_MAX)
		{
			break;
		}
	}

add_gw_detect:
#if	0	
	for (ip = &iface->proto_ip, j = 0; j < 0x2; ip = &iface->config_ip, ++j)
	{
		vlist_for_each_element(&ip->route, route, node) 
		{	
			if (!route->enabled)	
				continue;
			
			if ((route->flags & DEVADDR_FAMILY) != DEVADDR_INET4)
				continue;

			if (route->nexthop.in.s_addr == 0)
				continue;

			if (route->addr.in.s_addr != 0)
				continue;

			if (i >= DETECT_WAY_CNT)
			{
				break;
			}

			way = detect_way + i;
			way->type = DETECT_WAY_GW;
			way->args.af = AF_INET;
			way->args.addr = route->nexthop;
			way->args.iface = iface;
			way->done = false;
			way->cb = detect_gw_query;
			inet_ntop(AF_INET, &route->nexthop, buf, 0x80);
			//tp_prtf("gw%d: %s", i, buf);
			i++;
			break;
		}

		if (i >= DETECT_WAY_CNT)
		{
			break;
		}
	}
#endif	

	detect_hdlrs[D_RETRY].times = 0;
	detect_hdlrs[D_RETRY].times_lmt = i;

	fsm->times++;
	if (fsm->times > fsm->times_lmt)
	{
		fsm->times = fsm->times_lmt;
	}

	*phase = D_RETRY;
}


/*
##	=======================================================
##	Actions [State1 -> State2]	Contitions...
##	---------------------------------------------------------------------
1.	a [1 -> 6]	Non automatic connection OR non connected OR an error happened.
3.	c [4 -> 1]	If the current attempt isn't the last attempt, will begin to start online
				detection.
5.	e [1 -> 2]	It has sent a request package of some protocol, waiting for response
				from the peer to detect the link state.
6.	f [1 -> 4]	The current attempt way cannot start, enter the next attempt.
##	=======================================================
*/
static void
detect_started(struct detect_fsm* fsm, unsigned* phase)
{
	int i = detect_gset_way_idx(0, false);
	struct detect_ways *way = NULL;

	way = detect_way + i;
	if (!way || way->done || i >= DETECT_WAY_CNT)
	{
		*phase = D_RETRY;
		return;
	}

	way->done = true;

	if (!way->cb || way->cb(&way->args, true))
	{
		*phase = D_OVER;
		return;
	}

	detect_hdlrs[D_DETECTING].times = 0x1;
	*phase = D_DETECTING;
}


/*
##	=======================================================
##	Actions [State1 -> State2]	Contitions...
##	---------------------------------------------------------------------
1.	a [2 -> 6]	Non automatic connection OR non connected OR an error happened.
5.	e [1 -> 2]	It has sent a request package of some protocol, waiting for response
				from the peer to detect the link state.
7.	g [2 -> 2]	Timeout to receive a response from the peer, continue to do it again!
8.	h [2 -> 3]	Has received a GOOD response from the peer, enter Online state, 
				implies current connection is well.
9.	i [2 -> 4] 	Received a bad response from the peer, or received none after all 
				attempts, try to begin to prepare for the next time online detection.
##	=======================================================
*/
static void
detect_detecting(struct detect_fsm* fsm, unsigned* phase)
{
	int ret = 0;
	int i = detect_gset_way_idx(0, false);
	struct detect_ways *way = NULL;
	//char cmd[0x100];

	way = detect_way + i;
	if (!way || i >= DETECT_WAY_CNT)
	{
		*phase = D_RETRY;
		goto end;
	}

	if (!way->cb)
	{
		*phase = D_OVER;
		goto end;
	}

	ret = way->cb(&way->args, false);
	//tp_prtf("detecting: ret = %d", ret);
	
	fsm->times++;
	switch (ret)
	{
	case 0:
		/* msg received */
		fsm->times = 0x1;
		*phase = D_ONLINE;
		break;
	case -1:
	case -2:
	case -4:
		if (fsm->times >= fsm->times_lmt || ret != -1)
		{
			/* timeout */
			fsm->times = 0x1;
			*phase = D_RETRY;
			break;
		}
		*phase = D_DETECTING;
		return;
	case -3:
		*phase = D_OVER;
		break;
	default:
		*phase = D_OVER;
	}

end:
	detect_gset_sock(DETECT_SOCK_INVALID, true);
}


/*
##	=======================================================
##	Actions [State1 -> State2]	Contitions...
##	---------------------------------------------------------------------
8.	h [2 -> 3]	Has received a GOOD response from the peer, enter Online state, 
				implies current connection is well.
10.	j [3 -> 0]	After sleeping, enter the next round of online detection.
##	=======================================================
*/
static void
detect_online(struct detect_fsm* fsm, unsigned* phase)
{
	fsm->times++;
	if (fsm->times > fsm->times_lmt)
	{
		fsm->times = fsm->times_lmt;
	}

	detect_hdlrs[D_OFFLINE].times = 0x1;
	*phase = D_OVER;
}


/*
##	=======================================================
##	Actions [State1 -> State2]	Contitions...
##	---------------------------------------------------------------------
2.	b [0 -> 4]	Get info of connected status,  initialize some resources, try to begin 
				to prepare for the online detection.
3.	c [4 -> 1]	If the current attempt isn't the last attempt, will begin to start online
				detection.
4.	d [4 -> 5]	If you have already completed the last attempt, enter Offine state to
				terminate the "bad" connection.
6.	f [1 -> 4]	The current attempt way cannot start, enter the next attempt.
9.	i [2 -> 4] 	Received a bad response from the peer, or received none after all 
				attempts, try to begin to prepare for the next time online detection.
##	=======================================================
*/
static void
detect_retry(struct detect_fsm* fsm, unsigned* phase)
{
	detect_gset_sock(DETECT_SOCK_INVALID, true);
	
	detect_gset_way_idx(fsm->times, true);
	
	fsm->times++;
	if (fsm->times > fsm->times_lmt)
	{
		fsm->times = 0;
		*phase = D_OFFLINE;
		return;
	}

	*phase = D_STARTED;
}


/*
##	=======================================================
##	Actions [State1 -> State2]	Contitions...
##	---------------------------------------------------------------------
4.	d [4 -> 5]	If you have already completed the last attempt, enter Offine state to
				terminate the "bad" connection.
11.	k [5 -> 0]	After sleeping, enter the next round of online detection.
##	=======================================================
*/
static void
detect_offline(struct detect_fsm* fsm, unsigned* phase)
{
	fsm->times++;
	if (fsm->times > fsm->times_lmt)
	{
		fsm->times = fsm->times_lmt;
	}

	detect_hdlrs[D_ONLINE].times = 0x1;
	*phase = D_OVER;
}


/*
##	=======================================================
##	Actions [State1 -> State2]	Contitions...
##	---------------------------------------------------------------------
1.	a [* -> 6]	Non automatic connection OR non connected OR an error happened.
12.	l [6 -> 0] 	Current connection state has changed to connected from disconnected
				begin to enter a new round of online detection.
##	=======================================================
*/
static void
detect_over(struct detect_fsm* fsm, unsigned* phase)
{
	detect_gset_sock(DETECT_SOCK_INVALID, true);

	fsm->times++;
	if (fsm->times > fsm->times_lmt)
	{
		fsm->times = fsm->times_lmt;
	}

	*phase = D_INITIAL;

#if 0 /* for test!!!! */
	if (curr_backup_cfg.detecting_nw == NETWORK_TYPE_MOBILE)
	{
		if (0 == access("/tmp/34G_ON", F_OK))
		{
			curr_backup_cfg.detect_ret = D_ONLINE;
		}
		else if (0 == access("/tmp/34G_OFF", F_OK))
		{
			curr_backup_cfg.detect_ret = D_OFFLINE;
		}
	}
#endif /* end of for test!!!! */

	if (curr_backup_cfg.detect_ret == D_ONLINE)
	{
		/* network type (curr_backup_cfg.detecting_nw) is online, just switch to it. */
		/* USBMODEM_DBG(LOG_BK_NETWORK_ON, LP_UINT32, curr_backup_cfg.detecting_nw); */
		__set_curr_nw_status(curr_backup_cfg.detecting_nw, 1);
		switch_network_type(curr_backup_cfg.detecting_nw, detect_if);
		/* one round online-test end, go next round */
		curr_backup_cfg.detecting_nw = NETWORK_TYPE_NONE;
	}
	else
	{
		/* USBMODEM_DBG(LOG_BK_NETWORK_OFF, LP_INT32, curr_backup_cfg.detecting_nw); */
		__set_curr_nw_status(curr_backup_cfg.detecting_nw, 0);
		set_next_detect_if();
		if (curr_backup_cfg.detecting_nw == NETWORK_TYPE_NONE)
		{
			/* all network offline, set network type 0 and just do nothing. */
			curr_backup_cfg.current_nw = NETWORK_TYPE_NONE;
			//switch_network_type(curr_backup_cfg.primary_nw);
		}
	}
	curr_backup_cfg.detect_ret = D_OFFLINE;

	if (curr_backup_cfg.detecting_nw == NETWORK_TYPE_NONE && curr_backup_cfg.stable > NETWORK_STABLE_LIMIT)
	{
		fsm->timeunit = NETWORK_STABLE_TIME_INTERVAL;
	}
	else
	{
		fsm->timeunit = NETWORK_NOT_STABLE_TIME_INTERVAL;
	}
}

static int ifs_processing(void)
{
	enum network_type nt    = NETWORK_TYPE_NONE;
	struct interface *iface = NULL;
	vlist_for_each_element(&interfaces, iface, node)
	{
		nt = get_network_type(iface->name);
		if (nt != NETWORK_TYPE_MAX &&
			(iface->state == IFS_SETUP || iface->state == IFS_TEARDOWN))
		{
			__set_curr_nw_status(nt, 0); /* refix 438860 in SPF, set processing ifs'online flag to 0 */
			return nt;
		}
	}
	return 0;
}

static void 
detect_timeout(struct uloop_timeout *timeout)
{
	static unsigned detect_phase = D_INITIAL;
	struct detect_fsm* hdlr = NULL;
	int period = 0x1, res = 0;

	//system("echo [backup]detect--- >/dev/console");

#if 0
	char buf[256];
	memset(buf,0,256);
	sprintf(buf,"echo detecting_nw:%d current_nw:%d detect_ret:%d >/dev/console",curr_backup_cfg.detecting_nw,curr_backup_cfg.current_nw,curr_backup_cfg.detect_ret);
	system(buf);
#endif

	if ((res = ifs_processing()))	/* some iface is connecting or disconnecting */
	{
		curr_backup_cfg.current_nw = NETWORK_TYPE_NONE;
		goto go_next;
	}

	if (curr_backup_cfg.mode == CONN_MODE_CABLE_ONLY || curr_backup_cfg.mode == CONN_MODE_MOBILE_ONLY)
	{
		backup_prefer_only();
		goto go_next;
	}

	res = detect_be_over();
	if (res)
	{
		detect_phase = D_OVER;
		if (!access("/tmp/backup_exit", F_OK))
		{
			return;
		}
	}

	hdlr = &detect_hdlrs[detect_phase];

	if (hdlr->cb)
	{
		/* "detect_phase" will be moved to another value in most cases. */
		(void)hdlr->cb(hdlr, &detect_phase);
	}

	if (hdlr->timeunit == 0)
	{
		period = 0;
	}
	else if (hdlr->times)
	{
		period = hdlr->timeunit * (1 << (hdlr->times - 1));
	}

	if (detect_phase == D_ONLINE)
	{
		curr_backup_cfg.detect_ret = D_ONLINE;
	}
	else if (detect_phase == D_OFFLINE)
	{
		curr_backup_cfg.detect_ret = D_OFFLINE;
	}

go_next:
	(void)uloop_timeout_set(timeout, period * 1000);
}

void unset_nw(enum network_type type)
{
	enum network_type target = NETWORK_TYPE_NONE;
	struct interface *iface = NULL;

	if (type == NETWORK_TYPE_WIRED )
	{
		target = NETWORK_TYPE_MOBILE;
	}
	else if (type == NETWORK_TYPE_MOBILE)
	{
		target = NETWORK_TYPE_WIRED;
	}
	else
	{
		return;
	}

	/* USBMODEM_DBG(LOG_BK_UNSET_NW, LP_INT32, type); */

	iface = target_network_up(target);
	if (iface)
	{
		switch_network_type(target, iface);
	}
	else if(target == NETWORK_TYPE_WIRED) /*fix bug156102,from wangjiang_w8226*/
	{
		set_ubus_curr_nw(target);
	}

	return;
}

int 
detect_init_exit(bool init)
{
	static bool detect_inited = false;
	int ret = 0;

	/* exit case */
	if (!init)
	{
		if (detect_inited)
		{
			uloop_timeout_cancel(&detect_timer);
			detect_inited = false;
		}
		return ret;
	}

	/* init case */
	if (!detect_inited)
	{
		ret = backup_cfg_init();
		if (ret != 0 || !curr_backup_cfg.use_backup)
		{
			return ret;
		}
		
		detect_timer.cb = detect_timeout;
		ret = uloop_timeout_set(&detect_timer, 6500);

		detect_inited = true;
	}
	return ret;
}

