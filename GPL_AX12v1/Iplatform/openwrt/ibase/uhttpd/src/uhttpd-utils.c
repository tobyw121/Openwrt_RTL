/*
 * uhttpd - Tiny single-threaded httpd - Utility functions
 *
 *   Copyright (C) 2010-2012 Jo-Philipp Wich <xm@subsignal.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "uhttpd.h"
#include "uhttpd-utils.h"

#ifdef HAVE_TLS
#include "uhttpd-tls.h"
#include <uci.h>
#endif

#include <inttypes.h>

#ifdef HAVE_TLS
void uh_get_operation_mode(struct admin_config *conf)
{	
    char* config = UCI_MODE_CFG;
    struct uci_context *uciCtx  = NULL;
    struct uci_package *pkg     = NULL;
    struct uci_section *section = NULL; 
    char *value = NULL; 
    
    conf->mode = MODE_ROUTER;
    
    /* load network config */
    uciCtx = uci_alloc_context();   
    uci_set_confdir(uciCtx, "/etc/config");
    uci_set_savedir(uciCtx, "/var/state");
    if (UCI_OK != uci_load(uciCtx, config, &pkg))
    {
        uci_perror(uciCtx, config); 
        HTTPS_D("uci: load %s config failed.\n", config);		
        goto err;
    }

    /* get lan */
    section = uci_lookup_section(uciCtx, pkg, "sysmode");
    if(NULL == section)
    {
        uci_perror(uciCtx, "sysmode"); 
        HTTPS_D("uci: get %s section failed.\n", "sysmode");		
        goto ret;
    }

    /* get ip */
    value = uci_lookup_option_string(uciCtx, section, "mode");
    if (NULL == value)
    {
        HTTPS_D("uci: get %s option failed.\n", "mode");
        goto ret;
    }
    else
    {
        HTTPS_D("%s of %s in %s is %s\n", "sysmode", "mode", config, value); 
        if(0 == strncasecmp(value, "ap", 2))
        {
        	conf->mode = MODE_AP;
        }
#ifdef SUPPORT_MESH_WEB_DEBUG
        else if (0 == strncasecmp(value, "repeater", 8))
		{
			conf->mode = MODE_RE;
		}
#endif
    }   
    HTTPS_D("operation mode = %d\n", conf->mode);

ret:
    uci_unload(uciCtx, pkg);        
err:
    uci_free_context(uciCtx);
    uciCtx = NULL;	
}

int uh_get_local_addr(struct admin_config *conf)
{
	int res = 0;
    char* config = UCI_NET_CFG;
    struct uci_context *uciCtx  = NULL;
    struct uci_package *pkg     = NULL;
    struct uci_section *section = NULL; 
    char *value = NULL;  
    struct in_addr addr;
    struct in6_addr addr6;

    /* treated as lan access on default */
    conf->lan_ip = 0;
    conf->lan_mask = 0;
    
    /* load network config */
    uciCtx = uci_alloc_context();   
    uci_set_confdir(uciCtx, "/etc/config");
    uci_set_savedir(uciCtx, "/var/state");
    if (UCI_OK != uci_load(uciCtx, config, &pkg))
    {
        uci_perror(uciCtx, config); 
        HTTPS_D("uci: load %s config failed.\n", config);
		res = -1;
        goto err;
    }

    /* get lan */
    section = uci_lookup_section(uciCtx, pkg, "lan");
    if(NULL == section)
    {
        uci_perror(uciCtx, "lan"); 
        HTTPS_D("uci: get lan section failed.\n");
		res = -1;
        goto ret;
    }

    /* get ip */
    value = uci_lookup_option_string(uciCtx, section, "ipaddr");
    if (NULL == value)
    {
        HTTPS_D("uci: get %s option failed.\n", "ipaddr");
		res = -1;
        goto ret;
    }
    else
    {
        HTTPS_D("%s of %s in %s is %s\n", "ipaddr", "lan", config, value); 
        if(inet_pton(AF_INET, value, (void *)&(addr)) <= 0)
        {
            D("SRV: uh_get_local_addr() failed.\n");
			res = -1;
            goto ret;
        }
        conf->lan_ip = ntohl(addr.s_addr);
    }

    /* get mask */    
    value = uci_lookup_option_string(uciCtx, section, "netmask");
    if (NULL == value)
    {
        HTTPS_D("uci: get %s option failed.\n", "netmask");
		res = -1;
        goto ret;
    }
    else
    {
        HTTPS_D("%s of %s in %s is %s\n", "netmask", "lan", config, value); 
        if(inet_pton(AF_INET, value, (void *)&(addr)) <= 0)
        {
            D("SRV: uh_get_local_addr() failed.\n");
			res = -1;
            goto ret;
        }
        conf->lan_mask = ntohl(addr.s_addr);
    }
    HTTPS_D("lan_ip = %X, lan_mask = %X\n", conf->lan_ip, conf->lan_mask);

    /* get lan */
    section = uci_lookup_section(uciCtx, pkg, "lanv6");

    if(section)
    {
        /* get ipv6 prefix */
        value = uci_lookup_option_string(uciCtx, section, "prefix");
        if (value)
        {
            HTTPS_D("%s of %s in %s is %s\n", "prefix", "lanv6", config, value);
            if(inet_pton(AF_INET6, value, (void *)&(addr6)) <= 0)
            {
                D("SRV: uh_get_local_addr() failed.\n");
                res = -1;
                goto ret;
            }
            memcpy((unsigned char *)&conf->prefix,  (unsigned char *)&addr6, sizeof(addr6));
        }

        /* get ipv6 prefixlen */
        value = uci_lookup_option_string(uciCtx, section, "prefixlen");
        if (value)
        {
            HTTPS_D("%s of %s in %s is %s\n", "prefixlen", "lanv6", config, value);
            conf->prefixlen = atoi(value);
        }
        else
        {
            conf->prefixlen = 64;
        }
    }

ret:
    uci_unload(uciCtx, pkg);        
err:
    uci_free_context(uciCtx);
    uciCtx = NULL;
	return res;
}

void uh_get_admin_config(struct admin_config *conf)
{
    char* config = UCI_ADMIN_CFG;
    struct uci_context *uciCtx  = NULL;
    struct uci_package *pkg     = NULL;
    struct uci_section *section = NULL; 
    char *value = NULL;
    int port = 0;    
    
    uciCtx = uci_alloc_context();   
    uci_set_confdir(uciCtx, "/etc/config");
    uci_set_savedir(uciCtx, "/var/state");
    if (UCI_OK != uci_load(uciCtx, config, &pkg))
    {
        uci_perror(uciCtx, config); 
        HTTPS_D("uci: load %s config failed.\n", config); 
        goto err;
    }

    /* get local */
    section = uci_lookup_section(uciCtx, pkg, "local");
    if(NULL == section)
    {
        uci_perror(uciCtx, "local"); 
        HTTPS_D("uci: get local section failed.\n");        
        goto ret;
    }
    
    value = uci_lookup_option_string(uciCtx, section, "https_enable");
    if (NULL == value)
    {
        HTTPS_D("uci: get https_enable option failed.\n");        
        conf->local_https_on = false;
    }
    else
    {
        HTTPS_D("%s of %s in %s is %s\n", "https_enable", "local", config, value);        
        if(!strncasecmp(value, "on", 2))
        {
            conf->local_https_on = true;
        }
        else
        {
            conf->local_https_on = false;
        }
    }

    /* get remote */
    section = uci_lookup_section(uciCtx, pkg, "remote");
    if(NULL == section)
    {
        uci_perror(uciCtx, "remote"); 
        HTTPS_D("uci: get %s section failed.\n", "remote");        
        goto ret;
    }
    
    value = uci_lookup_option_string(uciCtx, section, "enable");
    if (NULL == value)
    {
        HTTPS_D("uci: get %s option failed.\n", "enable");        
        conf->remote_on = false;
    }
    else
    {
        HTTPS_D("%s of %s in %s is %s\n", "enable", "remote", config, value); 
        /* enable: all/partial/off */
        if(!strncasecmp(value, "off", 3))
        {
            conf->remote_on = false;
        }
        else
        {
            conf->remote_on = true;
        }
    }

    value = uci_lookup_option_string(uciCtx, section, "port");
    if (NULL == value)
    {
        HTTPS_D("uci: get %s option failed.\n", "port");        
        conf->remote_http_port = DEFAULT_HTTP_PORT;
    }
    else
    {
        HTTPS_D("%s of %s in %s is %s\n", "port", "remote", config, value); 
        port = atoi(value);
        if((port < UH_MIN_PORT && port != DEFAULT_HTTP_PORT) || port > UH_MAX_PORT)
        {
            conf->remote_http_port = DEFAULT_HTTP_PORT;
        }
        else
        {
            conf->remote_http_port = port;
        }
    }

    value = uci_lookup_option_string(uciCtx, section, "https_port");
    if (NULL == value)
    {
        HTTPS_D("uci: get %s option failed.\n", "https_port");        
        conf->remote_https_port = DEFAULT_HTTPS_PORT;
    }
    else
    {
        HTTPS_D("%s of %s in %s is %s\n", "https_port", "remote", config, value); 
        port = atoi(value);
        if((port < UH_MIN_PORT && port != DEFAULT_HTTPS_PORT) || port > UH_MAX_PORT)
        {
            conf->remote_https_port = DEFAULT_HTTPS_PORT;
        }
        else
        {
            conf->remote_https_port = port;
        }
    }
    HTTPS_D("local_https_on = %s, remote_on = %s, remote_http_port = %u, remote_https_port = %u\n",
        conf->local_https_on ? "on":"off", conf->remote_on ? "on":"off",
        conf->remote_http_port, conf->remote_https_port);
    
ret:
    uci_unload(uciCtx, pkg);        
err:
    uci_free_context(uciCtx);
    uciCtx = NULL;
}

bool uh_check_local_access(struct admin_config *conf, struct in_addr *addr)
{
    unsigned int ipaddr = ntohl(addr->s_addr);
    HTTPS_D("ipaddr = %X, lan_ip = %X\n", ipaddr, conf->lan_ip);
    
    return ((ipaddr & conf->lan_mask) == (conf->lan_ip & conf->lan_mask));
}

bool uh_check_local_access_v6(struct admin_config *conf, struct in6_addr *addr6)
{
	int quotient;
	int remainder;
	int flag = 0;
	struct in6_addr local_addr;

	memset(&local_addr, 0, sizeof(local_addr));
	inet_pton(AF_INET6, "fe80::", (void *)&(local_addr));

	//char ip6addr[40] = {0};
	//inet_ntop(AF_INET6, addr6, ip6addr, 40);
	//HTTPS_D("ipv6 addr is : %s\n", ip6addr);
	
	if (!memcmp(addr6, &local_addr, 8))
	{	
		flag = 1;
	}
	else if (conf->prefixlen > 0 && !memcmp(addr6, &conf->prefix, (conf->prefixlen)/8))
	{
		if ((conf->prefixlen)%8 == 0)
		{
			flag = 2;
		}
		else
		{
			quotient = (conf->prefixlen)/8;
			remainder = 8 - (conf->prefixlen)%8;
			HTTPS_D("Irregular prefix:quotient=%d,remainder=%d\n", quotient, remainder);
			if(((char)(addr6->s6_addr[quotient]) >> remainder) == ((char)(conf->prefix.s6_addr[quotient]) >> remainder))
			{
				flag = 2;
			}
		}
	}

	if (flag == 1)
	{
		HTTPS_D("local ipv6 address access\n");
		return true;
	}
	else if(flag == 2)
	{
		HTTPS_D("the accessing ipv6 address's prefix is the same with lanv6 prefix\n");
		return true;
	}
	else
	{
		HTTPS_D("remote access\n");
		return false;
	}
}

bool uh_check_local_https_enable(struct admin_config *conf)
{
    return conf->local_https_on;
}

#endif

const char * sa_straddr(void *sa)
{
	static char str[INET6_ADDRSTRLEN];
	struct sockaddr_in *v4 = (struct sockaddr_in *)sa;
	struct sockaddr_in6 *v6 = (struct sockaddr_in6 *)sa;

	if (v4->sin_family == AF_INET)
		return inet_ntop(AF_INET, &(v4->sin_addr), str, sizeof(str));
	else
		return inet_ntop(AF_INET6, &(v6->sin6_addr), str, sizeof(str));
}

const char * sa_strport(void *sa)
{
	static char str[6];
	snprintf(str, sizeof(str), "%i", sa_port(sa));
	return str;
}

int sa_port(void *sa)
{
	return ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
}

int sa_rfc1918(void *sa)
{
	struct sockaddr_in *v4 = (struct sockaddr_in *)sa;
	unsigned long a = htonl(v4->sin_addr.s_addr);

	if (v4->sin_family == AF_INET)
	{
		return ((a >= 0x0A000000) && (a <= 0x0AFFFFFF)) ||
		       ((a >= 0xAC100000) && (a <= 0xAC1FFFFF)) ||
		       ((a >= 0xC0A80000) && (a <= 0xC0A8FFFF));
	}

	return 0;
}

/* Simple strstr() like function that takes len arguments for both haystack and needle. */
char *strfind(char *haystack, int hslen, const char *needle, int ndlen)
{
	int match = 0;
	int i, j;

	for (i = 0; i < hslen; i++)
	{
		if (haystack[i] == needle[0])
		{
			match = ((ndlen == 1) || ((i + ndlen) <= hslen));

			for (j = 1; (j < ndlen) && ((i + j) < hslen); j++)
			{
				if (haystack[i+j] != needle[j])
				{
					match = 0;
					break;
				}
			}

			if (match)
				return &haystack[i];
		}
	}

	return NULL;
}

bool uh_socket_wait(int fd, int sec, bool write)
{
	int rv;
	struct timeval timeout;

	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	timeout.tv_sec = sec;
	timeout.tv_usec = 0;

	while (((rv = select(fd+1, write ? NULL : &fds, write ? &fds : NULL,
						 NULL, &timeout)) < 0) && (errno == EINTR))
	{
		D("IO: FD(%d) select interrupted: %s\n",
				fd, strerror(errno));

		continue;
	}

	if (rv <= 0)
	{
		D("IO: FD(%d) appears dead (rv=%d)\n", fd, rv);
		return false;
	}

	return true;
}

static int __uh_raw_send(struct client *cl, const char *buf, int len, int sec,
						 int (*wfn) (struct client *, const char *, int))
{
	ssize_t rv;
	int fd = cl->fd.fd;

	while (true)
	{
		if ((rv = wfn(cl, buf, len)) < 0)
		{
			if (errno == EINTR)
			{
				D("IO: FD(%d) interrupted\n", cl->fd.fd);
				continue;
			}
			else if ((sec > 0) && (errno == EAGAIN || errno == EWOULDBLOCK))
			{
				if (!uh_socket_wait(fd, sec, true))
					return -1;
			}
			else
			{
				D("IO: FD(%d) write error: %s\n", fd, strerror(errno));
				return -1;
			}
		}
		/*
		 * It is not entirely clear whether rv = 0 on nonblocking sockets
		 * is an error. In real world fuzzing tests, not handling it as close
		 * led to tight infinite loops in this send procedure, so treat it as
		 * closed and break out.
		 */
		else if (rv == 0)
		{
			D("IO: FD(%d) appears closed\n", fd);
			return 0;
		}
		else if (rv < len)
		{
			D("IO: FD(%d) short write %d/%d bytes\n", fd, rv, len);
			len -= rv;
			buf += rv;
			continue;
		}
		else
		{
			D("IO: FD(%d) sent %d/%d bytes\n", fd, rv, len);
			return rv;
		}
	}
}

int uh_tcp_send_lowlevel(struct client *cl, const char *buf, int len)
{
	return write(cl->fd.fd, buf, len);
}

int uh_raw_send(int fd, const char *buf, int len, int sec)
{
	struct client_light cl = { .fd = { .fd = fd } };
	return __uh_raw_send((struct client *)&cl, buf, len, sec,
						 uh_tcp_send_lowlevel);
}

int uh_tcp_send(struct client *cl, const char *buf, int len)
{
	int seconds = cl->server->conf->network_timeout;
#ifdef HAVE_TLS
	if (cl->tls)
		return __uh_raw_send(cl, buf, len, seconds,
							 cl->server->conf->tls_send);
#endif
	return __uh_raw_send(cl, buf, len, seconds, uh_tcp_send_lowlevel);
}

static int __uh_raw_recv(struct client *cl, char *buf, int len, int sec,
						 int (*rfn) (struct client *, char *, int))
{
	ssize_t rv;
	int fd = cl->fd.fd;

	while (true)
	{
		if ((rv = rfn(cl, buf, len)) < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if ((sec > 0) && (errno == EAGAIN || errno == EWOULDBLOCK))
			{
				if (!uh_socket_wait(fd, sec, false))
					return -1;
			}
			else
			{
				D("IO: FD(%d) read error: %s\n", fd, strerror(errno));
				return -1;
			}
		}
		else if (rv == 0)
		{
			D("IO: FD(%d) appears closed\n", fd);
			return 0;
		}
		else
		{
			D("IO: FD(%d) read %d bytes\n", fd, rv);
			return rv;
		}
	}
}

int uh_tcp_recv_lowlevel(struct client *cl, char *buf, int len)
{
	return read(cl->fd.fd, buf, len);
}

int uh_raw_recv(int fd, char *buf, int len, int sec)
{
	struct client_light cl = { .fd = { .fd = fd } };
	return __uh_raw_recv((struct client *)&cl, buf, len, sec,
						 uh_tcp_recv_lowlevel);
}

int uh_tcp_recv(struct client *cl, char *buf, int len)
{
	int seconds = cl->server->conf->network_timeout;
#ifdef HAVE_TLS
	if (cl->tls)
		return __uh_raw_recv(cl, buf, len, seconds,
							 cl->server->conf->tls_recv);
#endif
	return __uh_raw_recv(cl, buf, len, seconds, uh_tcp_recv_lowlevel);
}


int uh_http_sendhf(struct client *cl, int code, const char *summary,
				   const char *fmt, ...)
{
	va_list ap;

	char buffer[UH_LIMIT_MSGHEAD];
	int len;

	len = snprintf(buffer, sizeof(buffer),
		"HTTP/1.1 %03i %s\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n"
		"Transfer-Encoding: chunked\r\n\r\n",
			code, summary
	);

	ensure_ret(uh_tcp_send(cl, buffer, len));

	va_start(ap, fmt);
	len = vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	ensure_ret(uh_http_sendc(cl, buffer, len));
	ensure_ret(uh_http_sendc(cl, NULL, 0));

	return 0;
}

#ifdef HAVE_TLS
int uh_http_sendht(struct client *cl, int code, const char *summary,
				  const char *file_path)
{
	int fd = -1;
	struct stat fd_stat;
	char buffer[UH_LIMIT_MSGHEAD] = {0};
	int len = 0;
	const char https_notice[] = { 
		"Local Management via HTTPS is disabled. Please access the device via HTTP." };
		
	if( NULL == file_path)
	{
		HTTPS_D("file path is empty!");
		uh_http_sendhf(cl, 403, "Forbidden", https_notice);
		return 0;
	}

	if((stat(file_path, &fd_stat) != 0) || (!(fd_stat.st_mode & S_IFREG)))
	{
		/* remove print here, because st_size type is off_t, %d may not show correct result */
		/* if you want to look up it, please confirm how off_t is defined, and use %lu, %llu or others format */

		//HTTPS_D("Get file %s size error! fd_stat.st_size = %d\n", file_path, fd_stat.st_size);
		uh_http_sendhf(cl, 403, "Forbidden", https_notice);
		return 0;
	}
	//HTTPS_D("Get file %s size: fd_stat.st_size = %d\n", file_path, fd_stat.st_size);

	if((fd = open(file_path, O_RDONLY)) <= 0)
	{
		HTTPS_D("Open file error!");
		uh_http_sendhf(cl, 403, "Forbidden", https_notice);
		return 0;
	}

	/* st_size is off_t defined, for compatibility in bcm/qca and 32/64bits */
	/* here we cast st_size to unsigned long long, then use PRIu64 to print */
	len = snprintf(buffer, sizeof(buffer),
	   	"HTTP/1.1 %03i %s\r\n"
	   	"Connection: close\r\n"
	   	"Content-type: text/html\r\n"
	   	"Content-length: %" PRIu64 "\r\n"
	   	"\r\n",
		        code, summary, (unsigned long long)fd_stat.st_size
	);	

	ensure_out(uh_tcp_send(cl, buffer, len));
	
	HTTPS_D("To read and send file fd = %d\n", fd);
	
   	memset(buffer, 0, sizeof(buffer));	
	while ((len = read(fd, buffer, sizeof(buffer))) > 0)
	{
		HTTPS_D("read len = %d\n", len);
		ensure_out(uh_tcp_send(cl, buffer, len));
	}

	/* send trailer in chunked mode */
	ensure_out(uh_tcp_send(cl, "", 0));

out:
	if(fd > 0)
	{
		close(fd);
	}
	
   	return 0;
}
#endif

int uh_http_sendc(struct client *cl, const char *data, int len)
{
	char chunk[8];
	int clen;

	if (len == -1)
		len = strlen(data);

	if (len > 0)
	{
		clen = snprintf(chunk, sizeof(chunk), "%X\r\n", len);
		ensure_ret(uh_tcp_send(cl, chunk, clen));
		ensure_ret(uh_tcp_send(cl, data, len));
		ensure_ret(uh_tcp_send(cl, "\r\n", 2));
	}
	else
	{
		ensure_ret(uh_tcp_send(cl, "0\r\n\r\n", 5));
	}

	return 0;
}

int uh_http_sendf(struct client *cl, struct http_request *req,
				  const char *fmt, ...)
{
	va_list ap;
	char buffer[UH_LIMIT_MSGHEAD];
	int len;

	va_start(ap, fmt);
	len = vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

/*	if ((req != NULL) && (req->version > UH_HTTP_VER_1_0))
		ensure_ret(uh_http_sendc(cl, buffer, len));
	else if (len > 0)*/
		ensure_ret(uh_tcp_send(cl, buffer, len));

	return 0;
}

int uh_http_send(struct client *cl, struct http_request *req,
				 const char *buf, int len)
{
	if (len < 0)
		len = strlen(buf);
/*
	if ((req != NULL) && (req->version > UH_HTTP_VER_1_0))
		ensure_ret(uh_http_sendc(cl, buf, len));
	else if (len > 0) */
		ensure_ret(uh_tcp_send(cl, buf, len));

	return 0;
}


/* blen is the size of buf; slen is the length of src.  The input-string need
** not be, and the output string will not be, null-terminated.  Returns the
** length of the decoded string, -1 on buffer overflow, -2 on malformed string. */
int uh_urldecode(char *buf, int blen, const char *src, int slen)
{
	int i;
	int len = 0;

#define hex(x) \
	(((x) <= '9') ? ((x) - '0') : \
		(((x) <= 'F') ? ((x) - 'A' + 10) : \
			((x) - 'a' + 10)))

	for (i = 0; (i < slen) && (len < blen); i++)
	{
		if (src[i] == '%')
		{
			if (((i+2) < slen) && isxdigit(src[i+1]) && isxdigit(src[i+2]))
			{
				buf[len++] = (char)(16 * hex(src[i+1]) + hex(src[i+2]));
				i += 2;
			}
			else
			{
				/* Encoding error: it's hard to think of a
				** scenario in which returning an incorrect
				** 'decoding' of the malformed string is
				** preferable to signaling an error condition. */
				#if 0 /* WORSE_IS_BETTER */
				    buf[len++] = '%';
				#else
				    return -2;
				#endif
			}
		}
		else
		{
			buf[len++] = src[i];
		}
	}

	return (i == slen) ? len : -1;
}

/* blen is the size of buf; slen is the length of src.  The input-string need
** not be, and the output string will not be, null-terminated.  Returns the
** length of the encoded string, or -1 on error (buffer overflow) */
int uh_urlencode(char *buf, int blen, const char *src, int slen)
{
	int i;
	int len = 0;
	const char hex[] = "0123456789abcdef";

	for (i = 0; (i < slen) && (len < blen); i++)
	{
		if( isalnum(src[i]) || (src[i] == '-') || (src[i] == '_') ||
		    (src[i] == '.') || (src[i] == '~') )
		{
			buf[len++] = src[i];
		}
		else if ((len+3) <= blen)
		{
			buf[len++] = '%';
			buf[len++] = hex[(src[i] >> 4) & 15];
			buf[len++] = hex[ src[i]       & 15];
		}
		else
		{
			len = -1;
			break;
		}
	}

	return (i == slen) ? len : -1;
}

int uh_b64decode(char *buf, int blen, const unsigned char *src, int slen)
{
	int i = 0;
	int len = 0;

	unsigned int cin  = 0;
	unsigned int cout = 0;


	for (i = 0; (i <= slen) && (src[i] != 0); i++)
	{
		cin = src[i];

		if ((cin >= '0') && (cin <= '9'))
			cin = cin - '0' + 52;
		else if ((cin >= 'A') && (cin <= 'Z'))
			cin = cin - 'A';
		else if ((cin >= 'a') && (cin <= 'z'))
			cin = cin - 'a' + 26;
		else if (cin == '+')
			cin = 62;
		else if (cin == '/')
			cin = 63;
		else if (cin == '=')
			cin = 0;
		else
			continue;

		cout = (cout << 6) | cin;

		if ((i % 4) == 3)
		{
			if ((len + 3) < blen)
			{
				buf[len++] = (char)(cout >> 16);
				buf[len++] = (char)(cout >> 8);
				buf[len++] = (char)(cout);
			}
			else
			{
				break;
			}
		}
	}

	buf[len++] = 0;
	return len;
}

static char * canonpath(const char *path, char *path_resolved)
{
	char path_copy[PATH_MAX];
	char *path_cpy = path_copy;
	char *path_res = path_resolved;

	/* relative -> absolute */
	if (*path != '/')
	{
		getcwd(path_copy, PATH_MAX);
		strncat(path_copy, "/", PATH_MAX - strlen(path_copy));
		strncat(path_copy, path, PATH_MAX - strlen(path_copy));
	}
	else
	{
		strncpy(path_copy, path, PATH_MAX);
	}

	/* normalize */
	while ((*path_cpy != '\0') && (path_cpy < (path_copy + PATH_MAX - 2)))
	{
		if (*path_cpy == '/')
		{
			/* skip repeating / */
			if (path_cpy[1] == '/')
			{
				path_cpy++;
				continue;
			}
			/* skip \*/
			else if (path_cpy[1] == '\\')
			{
				path_cpy++;
				continue;
			}
			/* /./ or /../ */
			else if (path_cpy[1] == '.')
			{
				/* skip /./ */
				if ((path_cpy[2] == '/') || (path_cpy[2] == '\0'))
				{
					path_cpy += 2;
					continue;
				}

				/* collapse /x/../ */
				else if ((path_cpy[2] == '.') &&
						 ((path_cpy[3] == '/') || (path_cpy[3] == '\0')))
				{
					while ((path_res > path_resolved) && (*--path_res != '/'))
						;

					path_cpy += 3;
					continue;
				}
			}
		}

		*path_res++ = *path_cpy++;
	}

	/* remove trailing slash if not root / */
	if ((path_res > (path_resolved+1)) && (path_res[-1] == '/'))
		path_res--;
	else if (path_res == path_resolved)
		*path_res++ = '/';

	*path_res = '\0';

	/* test access */
	if (!access(path_resolved, R_OK))
		return path_resolved;

	return NULL;
}


struct index_file *uh_index_files = NULL;

struct index_file * uh_index_add(const char *filename)
{
	struct index_file *new = NULL;

	if ((filename != NULL) && (new = malloc(sizeof(*new))) != NULL)
	{
		new->name = filename;
		new->next = uh_index_files;

		uh_index_files = new;
	}

	return new;
}


/* Returns NULL on error.
** NB: improperly encoded URL should give client 400 [Bad Syntax]; returning
** NULL here causes 404 [Not Found], but that's not too unreasonable. */
struct path_info * uh_path_lookup(struct client *cl, const char *url)
{
	static char path_phys[PATH_MAX];
	static char path_info[PATH_MAX];
	static struct path_info p;

	char buffer[UH_LIMIT_MSGHEAD];
	char *docroot = cl->server->conf->docroot;
	char *pathptr = NULL;

	int slash = 0;
	int no_sym = cl->server->conf->no_symlinks;
	int i = 0;
	struct stat s;
	struct index_file *idx;

	/* back out early if url is undefined */
	if (url == NULL)
		return NULL;

	memset(path_phys, 0, sizeof(path_phys));
	memset(path_info, 0, sizeof(path_info));
	memset(buffer, 0, sizeof(buffer));
	memset(&p, 0, sizeof(p));

	/* copy docroot */
	memcpy(buffer, docroot,
		   min(strlen(docroot), sizeof(buffer) - 1));

	/* separate query string from url */
	if ((pathptr = strchr(url, '?')) != NULL)
	{
		p.query = pathptr[1] ? pathptr + 1 : NULL;

		/* urldecode component w/o query */
		if (pathptr > url)
		{
			if (uh_urldecode(&buffer[strlen(docroot)],
							 sizeof(buffer) - strlen(docroot) - 1,
							 url, pathptr - url ) < 0)
			{
				return NULL; /* bad URL */
			}
		}
	}

	/* no query string, decode all of url */
	else
	{
		if (uh_urldecode(&buffer[strlen(docroot)],
						 sizeof(buffer) - strlen(docroot) - 1,
						 url, strlen(url) ) < 0)
		{
			return NULL; /* bad URL */
		}
	}

	/* create canon path */
	for (i = strlen(buffer), slash = (buffer[max(0, i-1)] == '/'); i >= 0; i--)
	{
		if ((buffer[i] == 0) || (buffer[i] == '/'))
		{
			memset(path_info, 0, sizeof(path_info));
			memcpy(path_info, buffer, min(i + 1, sizeof(path_info) - 1));

			if (no_sym ? realpath(path_info, path_phys)
			           : canonpath(path_info, path_phys))
			{
				memset(path_info, 0, sizeof(path_info));
				memcpy(path_info, &buffer[i],
					   min(strlen(buffer) - i, sizeof(path_info) - 1));

				break;
			}
		}
	}

	/* check whether found path is within docroot */
	if (strncmp(path_phys, docroot, strlen(docroot)) ||
		((path_phys[strlen(docroot)] != 0) &&
		 (path_phys[strlen(docroot)] != '/')))
	{
		return NULL;
	}

	/* test current path */
	if (!stat(path_phys, &p.stat))
	{
		/* is a regular file */
		if (p.stat.st_mode & S_IFREG)
		{
			p.root = docroot;
			p.phys = path_phys;
			p.name = &path_phys[strlen(docroot)];
			p.info = path_info[0] ? path_info : NULL;
		}

		/* is a directory */
		else if ((p.stat.st_mode & S_IFDIR) && !strlen(path_info))
		{
			/* ensure trailing slash */
			if (path_phys[strlen(path_phys)-1] != '/')
				path_phys[strlen(path_phys)] = '/';

			/* try to locate index file */
			memset(buffer, 0, sizeof(buffer));
			memcpy(buffer, path_phys, sizeof(buffer));
			pathptr = &buffer[strlen(buffer)];

			/* if requested url resolves to a directory and a trailing slash
			   is missing in the request url, redirect the client to the same
			   url with trailing slash appended */
			if (!slash)
			{
				uh_http_sendf(cl, NULL,
					"HTTP/1.1 302 Found\r\n"
					"Location: %s%s%s\r\n"
					"Connection: close\r\n\r\n",
						&path_phys[strlen(docroot)],
						p.query ? "?" : "",
						p.query ? p.query : ""
				);

				p.redirected = 1;
			}
			else
			{
				for (idx = uh_index_files; idx; idx = idx->next)
				{
					strncat(buffer, idx->name, sizeof(buffer));

					if (!stat(buffer, &s) && (s.st_mode & S_IFREG))
					{
						memcpy(path_phys, buffer, sizeof(path_phys));
						memcpy(&p.stat, &s, sizeof(p.stat));
						break;
					}

					*pathptr = 0;
				}
			}

			p.root = docroot;
			p.phys = path_phys;
			p.name = &path_phys[strlen(docroot)];
		}
	}

	return p.phys ? &p : NULL;
}


static struct auth_realm *uh_realms = NULL;

struct auth_realm * uh_auth_add(char *path, char *user, char *pass)
{
	struct auth_realm *new = NULL;
	struct passwd *pwd;

#ifdef HAVE_SHADOW
	struct spwd *spwd;
#endif

	if((new = (struct auth_realm *)malloc(sizeof(struct auth_realm))) != NULL)
	{
		memset(new, 0, sizeof(struct auth_realm));

		memcpy(new->path, path,
			   min(strlen(path), sizeof(new->path) - 1));

		memcpy(new->user, user,
			   min(strlen(user), sizeof(new->user) - 1));

		/* given password refers to a passwd entry */
		if ((strlen(pass) > 3) && !strncmp(pass, "$p$", 3))
		{
#ifdef HAVE_SHADOW
			/* try to resolve shadow entry */
			if (((spwd = getspnam(&pass[3])) != NULL) && spwd->sp_pwdp)
			{
				memcpy(new->pass, spwd->sp_pwdp,
					   min(strlen(spwd->sp_pwdp), sizeof(new->pass) - 1));
			}

			else
#endif

			/* try to resolve passwd entry */
			if (((pwd = getpwnam(&pass[3])) != NULL) && pwd->pw_passwd &&
				(pwd->pw_passwd[0] != '!') && (pwd->pw_passwd[0] != 0))
			{
				memcpy(new->pass, pwd->pw_passwd,
					   min(strlen(pwd->pw_passwd), sizeof(new->pass) - 1));
			}
		}

		/* ordinary pwd */
		else
		{
			memcpy(new->pass, pass,
				min(strlen(pass), sizeof(new->pass) - 1));
		}

		if (new->pass[0])
		{
			new->next = uh_realms;
			uh_realms = new;

			return new;
		}

		free(new);
	}

	return NULL;
}

int uh_auth_check(struct client *cl, struct http_request *req,
				  struct path_info *pi)
{
	int i, plen, rlen, protected;
	char buffer[UH_LIMIT_MSGHEAD];
	char *user = NULL;
	char *pass = NULL;

	struct auth_realm *realm = NULL;

	plen = strlen(pi->name);
	protected = 0;

	/* check whether at least one realm covers the requested url */
	for (realm = uh_realms; realm; realm = realm->next)
	{
		rlen = strlen(realm->path);

		if ((plen >= rlen) && !strncasecmp(pi->name, realm->path, rlen))
		{
			req->realm = realm;
			protected = 1;
			break;
		}
	}

	/* requested resource is covered by a realm */
	if (protected)
	{
		/* try to get client auth info */
		foreach_header(i, req->headers)
		{
			if (!strcasecmp(req->headers[i], "Authorization") &&
				(strlen(req->headers[i+1]) > 6) &&
				!strncasecmp(req->headers[i+1], "Basic ", 6))
			{
				memset(buffer, 0, sizeof(buffer));
				uh_b64decode(buffer, sizeof(buffer) - 1,
					(unsigned char *) &req->headers[i+1][6],
					strlen(req->headers[i+1]) - 6);

				if ((pass = strchr(buffer, ':')) != NULL)
				{
					user = buffer;
					*pass++ = 0;
				}

				break;
			}
		}

		/* have client auth */
		if (user && pass)
		{
			/* find matching realm */
			for (realm = uh_realms; realm; realm = realm->next)
			{
				rlen = strlen(realm->path);

				if ((plen >= rlen) &&
					!strncasecmp(pi->name, realm->path, rlen) &&
					!strcmp(user, realm->user))
				{
					req->realm = realm;
					break;
				}
			}

			/* found a realm matching the username */
			if (realm)
			{
				/* check user pass */
				if (!strcmp(pass, realm->pass) ||
				    !strcmp(crypt(pass, realm->pass), realm->pass))
					return 1;
			}
		}

		/* 401 */
		uh_http_sendf(cl, NULL,
		              "%s 401 Authorization Required\r\n"
		              "WWW-Authenticate: Basic realm=\"%s\"\r\n"
		              "Content-Type: text/plain\r\n"
		              "Content-Length: 23\r\n\r\n"
		              "Authorization Required\n",
		              http_versions[req->version],
		              cl->server->conf->realm);

		return 0;
	}

	return 1;
}


static struct listener *uh_listeners = NULL;
static struct client *uh_clients = NULL;

struct listener * uh_listener_add(int sock, struct config *conf)
{
	struct listener *new = NULL;
	socklen_t sl;

	if ((new = (struct listener *)malloc(sizeof(struct listener))) != NULL)
	{
		memset(new, 0, sizeof(struct listener));

		new->fd.fd = sock;
		new->conf  = conf;


		/* get local endpoint addr */
		sl = sizeof(struct sockaddr_in6);
		memset(&(new->addr), 0, sl);
		getsockname(sock, (struct sockaddr *) &(new->addr), &sl);

		new->next = uh_listeners;
		uh_listeners = new;

		return new;
	}

	return NULL;
}

struct listener * uh_listener_lookup(int sock)
{
	struct listener *cur = NULL;

	for (cur = uh_listeners; cur; cur = cur->next)
		if (cur->fd.fd == sock)
			return cur;

	return NULL;
}


struct client * uh_client_add(int sock, struct listener *serv,
                              struct sockaddr_in6 *peer)
{
	struct client *new = NULL;
	socklen_t sl;

	if ((new = (struct client *)malloc(sizeof(struct client))) != NULL)
	{
		memset(new, 0, sizeof(struct client));
		memcpy(&new->peeraddr, peer, sizeof(new->peeraddr));

		new->fd.fd  = sock;
		new->server = serv;

		new->rpipe.fd = -1;
		new->wpipe.fd = -1;

		/* get local endpoint addr */
		sl = sizeof(struct sockaddr_in6);
		getsockname(sock, (struct sockaddr *) &(new->servaddr), &sl);

#ifdef HAVE_TLS        
		new->tls = NULL;
		new->local_access = false;
		new->local_https_on = false;        
#endif

		new->next = uh_clients;
		uh_clients = new;

		serv->n_clients++;

		D("IO: Client(%d) allocated\n", new->fd.fd);
	}

	return new;
}

struct client * uh_client_lookup(int sock)
{
	struct client *cur = NULL;

	for (cur = uh_clients; cur; cur = cur->next)
		if (cur->fd.fd == sock)
			return cur;

	return NULL;
}

void uh_client_shutdown(struct client *cl)
{
#ifdef HAVE_TLS
	/* free client tls context */
	if (cl->server && cl->server->conf->tls)
		cl->server->conf->tls_close(cl);
#endif

	/* remove from global client list */
	uh_client_remove(cl);
}

void uh_client_remove(struct client *cl)
{
	struct client *cur = NULL;
	struct client *prv = NULL;

	for (cur = uh_clients; cur; prv = cur, cur = cur->next)
	{
		if (cur == cl)
		{
			if (prv)
				prv->next = cur->next;
			else
				uh_clients = cur->next;

			if (cur->timeout.pending)
				uloop_timeout_cancel(&cur->timeout);

			if (cur->proc.pid)
				uloop_process_delete(&cur->proc);

			D("IO: Client(%d) freeing\n", cur->fd.fd);

			uh_ufd_remove(&cur->rpipe);
			uh_ufd_remove(&cur->wpipe);
			uh_ufd_remove(&cur->fd);

			cur->server->n_clients--;

			free(cur);
			break;
		}
	}
}


void uh_ufd_add(struct uloop_fd *u, uloop_fd_handler h, unsigned int ev)
{
	if (h != NULL)
	{
		u->cb = h;
		uloop_fd_add(u, ev);
		D("IO: FD(%d) added to uloop\n", u->fd);
	}
}

void uh_ufd_remove(struct uloop_fd *u)
{
	if (u->cb != NULL)
	{
		uloop_fd_delete(u);
		D("IO: FD(%d) removed from uloop\n", u->fd);
		u->cb = NULL;
	}

	if (u->fd > -1)
	{
		close(u->fd);
		D("IO: FD(%d) closed\n", u->fd);
		u->fd = -1;
	}
}


#ifdef HAVE_CGI
static struct interpreter *uh_interpreters = NULL;

struct interpreter * uh_interpreter_add(const char *extn, const char *path)
{
	struct interpreter *new = NULL;

	if ((new = (struct interpreter *)malloc(sizeof(struct interpreter))) != NULL)
	{
		memset(new, 0, sizeof(struct interpreter));

		memcpy(new->extn, extn, min(strlen(extn), sizeof(new->extn)-1));
		memcpy(new->path, path, min(strlen(path), sizeof(new->path)-1));

		new->next = uh_interpreters;
		uh_interpreters = new;

		return new;
	}

	return NULL;
}

struct interpreter * uh_interpreter_lookup(const char *path)
{
	struct interpreter *cur = NULL;
	const char *e;

	for (cur = uh_interpreters; cur; cur = cur->next)
	{
		e = &path[max(strlen(path) - strlen(cur->extn), 0)];

		if (!strcmp(e, cur->extn))
			return cur;
	}

	return NULL;
}
#endif
