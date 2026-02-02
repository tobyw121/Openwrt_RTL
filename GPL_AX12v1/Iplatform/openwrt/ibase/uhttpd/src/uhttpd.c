/*
 * uhttpd - Tiny single-threaded httpd - Main component
 *
 *   Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
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
#include "uhttpd-file.h"
#include <netinet/in.h>

#include <time.h>
//#include "uci.h"

#ifdef HAVE_CGI
#include "uhttpd-cgi.h"
#endif

#ifdef HAVE_LUA
#include "uhttpd-lua.h"
#endif

#ifdef HAVE_TLS
#include "uhttpd-tls.h"
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <libubox/ustream.h>
#endif

/*************************************************************************/
/*                             defines                                   */
/*************************************************************************/
#ifdef HAVE_TLS
static int uh_cfg_lan_update(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method, struct blob_attr *msg);

static int uh_cfg_local_update(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method, struct blob_attr *msg);

static int uh_cfg_remote_update(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method, struct blob_attr *msg);
#endif

#define CONFIG_FILE		"/www/webpages/app.manifest"

/*************************************************************************/
/*                             variables                                 */
/*************************************************************************/

time_t m_modified_time;

const char * http_methods[] = { "GET", "POST", "HEAD", "PUT", };
const char * http_versions[] = { "HTTP/0.9", "HTTP/1.0", "HTTP/1.1", };

static int run = 1;
static char webpage_time[64] = {0};

#ifdef SUPPORT_MESH_WEB_DEBUG
static int re_access = 0;
#endif

#ifdef HAVE_TLS

static struct admin_config g_admin_cfg;

/* 
 * add for https redirect/block, wl, 2017-09-28 
 * webpages/init.html should be locally accessed in factory mode, when https is disabled
 * "/cgi-bin/luci/;stok=/login"?
 */
static char* new_session_urls[] = {
	"/webpages/login.html",	
	"/webpages/index.%shtml",		/* access when reflesh */
	"/webmobile/login.html",
	"/webmobile/index.html",
	NULL
};

static char* no_cache_files[] = {
	"/index.html",
	"/webpages/login.html",	
	"/webpages/index.%shtml",		/* access when reflesh */
	NULL
};

const char https_forbidden_html[] = {
"<!DOCTYPE html><html><head>"
"<title>Error</title>"
"<style>body {width: 35em;margin: 0 auto;font-family: Tahoma, Verdana, Arial, sans-serif;}</style>"
"</head><body>"
"<h1>An error occurred.</h1>"
"<p>Sorry, the page you are looking for is currently unavailable.<br/>"
"Please try again later.</p>"
"<p>If you are the system administrator of this resource then you should check"
"the <a href=\"http://nginx.org/r/error_log\">error log</a> for details.</p>"
"<p><em>Faithfully yours, nginx.</em></p>"
"</body></html>"
};

/* add ubus server for config change, wl, 2017-09-21 */
static struct ubus_context *uh_cfg_ctx = NULL;
static struct blob_buf uh_cfg_buf;

enum {	
    UH_CFG_LAN_ADDR,
	UH_CFG_LAN_MASK,
	UH_CFG_LAN_ADDR_V6,
	UH_CFG_LAN_PREFIX_V6,
	UH_CFG_LAN_PREFIXLEN_V6,
	UH_CFG_LAN_MAX
};

enum {	
    UH_CFG_LOCAL_ENABLE,	
	UH_CFG_LOCAL_MAX
};

enum {
    UH_CFG_REMOTE_ENABLE,
    UH_CFG_REMOTE_PORT,
    UH_CFG_REMOTE_SPORT,
	UH_CFG_REMOTE_MAX
};

static const struct blobmsg_policy cfg_lan_policy[] = {    
    [UH_CFG_LAN_ADDR] = { .name = "ip", .type = BLOBMSG_TYPE_STRING },
    [UH_CFG_LAN_MASK] = { .name = "mask", .type = BLOBMSG_TYPE_STRING },
    [UH_CFG_LAN_ADDR_V6] = { .name = "ipv6", .type = BLOBMSG_TYPE_STRING },
    [UH_CFG_LAN_PREFIX_V6] = { .name = "prefix", .type = BLOBMSG_TYPE_STRING },
    [UH_CFG_LAN_PREFIXLEN_V6] = { .name = "prefixlen", .type = BLOBMSG_TYPE_STRING }
};

static const struct blobmsg_policy cfg_local_policy[] = {    
    [UH_CFG_LOCAL_ENABLE] = { .name = "https_enable", .type = BLOBMSG_TYPE_STRING }
};

static const struct blobmsg_policy cfg_remote_policy[] = {
    [UH_CFG_REMOTE_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_STRING },
    [UH_CFG_REMOTE_PORT] = { .name = "http_port", .type = BLOBMSG_TYPE_INT32 },
    [UH_CFG_REMOTE_SPORT] = { .name = "https_port", .type = BLOBMSG_TYPE_INT32 }
};

static const struct ubus_method uh_cfg_methods[] = {
    UBUS_METHOD("lan_update", uh_cfg_lan_update, cfg_lan_policy),
    UBUS_METHOD("local_update", uh_cfg_local_update, cfg_local_policy),
    UBUS_METHOD("remote_update", uh_cfg_remote_update, cfg_remote_policy),
};

static struct ubus_object_type uh_cfg_object_type =
    UBUS_OBJECT_TYPE("uhttpd", uh_cfg_methods);

static struct ubus_object uh_cfg_obj = {
    .name = "uhttpd",
    .type = &uh_cfg_object_type,
    .methods = uh_cfg_methods,
    .n_methods = ARRAY_SIZE(uh_cfg_methods),
};
/* add ended */
#endif

/*************************************************************************/
/*                             local function                            */
/*************************************************************************/

#ifdef HAVE_TLS
static int uh_cfg_lan_update(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    int re = 0;
	bool updated = false;
    int parse_status = 0;
    /*
    char *lan_ipv6 = NULL;
    struct in6_addr ip6addr;
    */
    char *lan_ip = NULL;
    char *lan_mask = NULL;
    struct in_addr addr;
    struct blob_attr *tb[UH_CFG_LAN_MAX] = {NULL};    
    
    /* 1. get msg */
	do
	{
	    parse_status = blobmsg_parse(cfg_lan_policy, UH_CFG_LAN_MAX, tb, blob_data(msg), blob_len(msg));
	    if (parse_status < 0)
	    {
	        HTTPS_D("Parse blog msg failed.\n");	        
	        break;
	    }
	    
	    if (!tb[UH_CFG_LAN_ADDR] || !tb[UH_CFG_LAN_MASK] || !tb[UH_CFG_LAN_ADDR_V6] || !tb[UH_CFG_LAN_PREFIX_V6] || !tb[UH_CFG_LAN_PREFIXLEN_V6])
	    {
	        HTTPS_D("access_policy error.\n");	        
	        break;
	    }

        /*lan_ipv6 = blobmsg_get_string(tb[UH_CFG_LAN_ADDR_V6]);
        if (lan_ipv6)
        {
            HTTPS_D("IPv6 address is: %s\n", lan_ipv6);
            if(inet_pton(AF_INET6, lan_ipv6, &ip6addr) > 0)
            {
                memcpy((unsigned char *)&g_admin_cfg.lan_ipv6,  (unsigned char *)&ip6addr, sizeof(ip6addr));
            }
        }
        else
        {
            HTTPS_D("br-lan has no ipv6 local address\n");
        }
        */

	    lan_ip = blobmsg_get_string(tb[UH_CFG_LAN_ADDR]);    
	    lan_mask = blobmsg_get_string(tb[UH_CFG_LAN_MASK]);
	    if (!lan_ip || !lan_mask)
	    {
	        HTTPS_D("Invalid params.\n");	        
	        break;
	    }
	    
	    HTTPS_D("lan_ip:%s, lan_mask:%s\n", lan_ip, lan_mask); 

	    /* 2. update lan info */
	    if(inet_pton(AF_INET, lan_ip, (void *)&(addr)) <= 0)
	    {	        
	        break;
	    }
	    g_admin_cfg.lan_ip = ntohl(addr.s_addr);
	    if(inet_pton(AF_INET, lan_mask, (void *)&(addr)) <= 0)
	    {	        
	        break;
	    }
	    g_admin_cfg.lan_mask = ntohl(addr.s_addr);
		updated = true;
	}while(0);

	if(!updated)
	{
		re = uh_get_local_addr(&g_admin_cfg);
	}
	
    /* 3. return result */
	memset(&uh_cfg_buf, 0, sizeof(uh_cfg_buf));
	blob_buf_init(&uh_cfg_buf, 0);
	blobmsg_add_u32(&uh_cfg_buf, "re", re);	
	ubus_send_reply(ctx, req, uh_cfg_buf.head);        

	HTTPS_D("uh_cfg_lan_update() done.\n");

	return 0;
}

static int uh_cfg_local_update(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    int re = 0;
    int parse_status = 0;
    char* https_enable = NULL;
    struct blob_attr *tb[UH_CFG_LOCAL_MAX] = {NULL};    
    
    /* 1. get msg */
    parse_status = blobmsg_parse(cfg_local_policy, UH_CFG_LOCAL_MAX, tb, blob_data(msg), blob_len(msg));
    if (parse_status < 0)
    {
        HTTPS_D("Parse blog msg failed.\n");
        re = -1;
        goto ret;
    }
    
    if (!tb[UH_CFG_LOCAL_ENABLE])
    {
        HTTPS_D("access_policy error.\n");
        re = -1;
        goto ret;
    }
    
    https_enable = blobmsg_get_string(tb[UH_CFG_LOCAL_ENABLE]);
    if (!https_enable)
    {
        HTTPS_D("Invalid params.\n");
        re = -1;
        goto ret;
    }    
    HTTPS_D("https_enable:%s\n", https_enable); 

    /* 2. update local info */
    if(!strncasecmp(https_enable, "on", 2))
    {
        g_admin_cfg.local_https_on = true;
    }
    else
    {
        g_admin_cfg.local_https_on = false;
    }

ret:
    /* 3. return result */
    memset(&uh_cfg_buf, 0, sizeof(uh_cfg_buf));
    blob_buf_init(&uh_cfg_buf, 0);
    blobmsg_add_u32(&uh_cfg_buf, "re", re); 
    ubus_send_reply(ctx, req, uh_cfg_buf.head);        

    HTTPS_D("uh_cfg_local_update() done.\n");

    return 0;
}

static int uh_cfg_remote_update(struct ubus_context *ctx, struct ubus_object *obj,
		    struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    int re = 0;
    int parse_status = 0;
    char* enable = NULL;
    unsigned int http_port = 0;
    unsigned int https_port = 0;
    struct blob_attr *tb[UH_CFG_REMOTE_MAX] = {NULL};    
    
    /* 1. get msg */
    parse_status = blobmsg_parse(cfg_remote_policy, UH_CFG_REMOTE_MAX, tb, blob_data(msg), blob_len(msg));
    if (parse_status < 0)
    {
        HTTPS_D("Parse blog msg failed.\n");
        re = -1;
        goto ret;
    }

    /* 2. check enable */
    if (!tb[UH_CFG_REMOTE_ENABLE])
    {
        D("access_policy error.\n");
        re = -1;
        goto ret;
    }
    
    enable = blobmsg_get_string(tb[UH_CFG_REMOTE_ENABLE]);    
    if (!enable)
    {
        HTTPS_D("Invalid params.\n");
        re = -1;
        goto ret;
    }    
    HTTPS_D("remote enable:%s\n", enable);
    
    if(!strncasecmp(enable, "off", 3))
    {
        g_admin_cfg.remote_on= false;
        goto ret;
    }  
    
    /* 3. get ports if enabled */
    if (!tb[UH_CFG_REMOTE_PORT] || !tb[UH_CFG_REMOTE_SPORT])
    {
        HTTPS_D("access_policy error.\n");
        re = -1;
        goto ret;
    }
    http_port = blobmsg_get_u32(tb[UH_CFG_REMOTE_PORT]);
    https_port = blobmsg_get_u32(tb[UH_CFG_REMOTE_SPORT]);
    g_admin_cfg.remote_on = true;
    g_admin_cfg.remote_http_port = http_port;
    g_admin_cfg.remote_https_port = https_port;
    HTTPS_D("enable:%s, http_port:%u, https_port:%u\n", enable, http_port, https_port); 

ret:    
    /* 4. return result */
    memset(&uh_cfg_buf, 0, sizeof(uh_cfg_buf));
    blob_buf_init(&uh_cfg_buf, 0);
    blobmsg_add_u32(&uh_cfg_buf, "re", re); 
    ubus_send_reply(ctx, req, uh_cfg_buf.head);        

    HTTPS_D("uh_cfg_remote_update() done.\n");

    return 0;
}

static int matchMobile (char *userAgent)
{
	char pattern[] = "iPod;|iPod touch;|iPhone;|Mobile Safari|IEMobile|Windows Phone|Fennec|Opera Mini|Opera Mobi";
    char *subpar   = NULL;
    char delims[]  = "|";

    subpar = strtok(pattern, delims);
    while ( NULL != subpar )
    {
    	if ( strstr(userAgent, subpar) )
    	{
    		return 1;
    	}
    	subpar = strtok( NULL, delims);
    }
    return 0;
}
#endif

///////////////////////////////////

static void uh_sigterm(int sig)
{
	run = 0;
}

static void uh_config_parse(struct config *conf)
{
	FILE *c;
	char line[512];
	char *col1 = NULL;
	char *col2 = NULL;
	char *eol  = NULL;

	const char *path = conf->file ? conf->file : "/etc/httpd.conf";


	if ((c = fopen(path, "r")) != NULL)
	{
		memset(line, 0, sizeof(line));

		while (fgets(line, sizeof(line) - 1, c))
		{
			if ((line[0] == '/') && (strchr(line, ':') != NULL))
			{
				if (!(col1 = strchr(line, ':')) || (*col1++ = 0) ||
				    !(col2 = strchr(col1, ':')) || (*col2++ = 0) ||
					!(eol = strchr(col2, '\n')) || (*eol++  = 0))
				{
					continue;
				}

				if (!uh_auth_add(line, col1, col2))
				{
					fprintf(stderr,
							"Notice: No password set for user %s, ignoring "
							"authentication on %s\n", col1, line
					);
				}
			}
			else if (!strncmp(line, "I:", 2))
			{
				if (!(col1 = strchr(line, ':')) || (*col1++ = 0) ||
				    !(eol = strchr(col1, '\n')) || (*eol++  = 0))
				{
				   	continue;
				}

				if (!uh_index_add(strdup(col1)))
				{
					fprintf(stderr,
					        "Unable to add index filename %s: "
					        "Out of memory\n", col1
					);
				}
			}
			else if (!strncmp(line, "E404:", 5))
			{
				if (!(col1 = strchr(line, ':')) || (*col1++ = 0) ||
				    !(eol = strchr(col1, '\n')) || (*eol++  = 0))
				{
					continue;
				}

				conf->error_handler = strdup(col1);
			}
#ifdef HAVE_CGI
			else if ((line[0] == '*') && (strchr(line, ':') != NULL))
			{
				if (!(col1 = strchr(line, '*')) || (*col1++ = 0) ||
				    !(col2 = strchr(col1, ':')) || (*col2++ = 0) ||
				    !(eol = strchr(col2, '\n')) || (*eol++  = 0))
				{
					continue;
				}

				if (!uh_interpreter_add(col1, col2))
				{
					fprintf(stderr,
							"Unable to add interpreter %s for extension %s: "
							"Out of memory\n", col2, col1
					);
				}
			}
#endif
		}

		fclose(c);
	}
}

static void uh_listener_cb(struct uloop_fd *u, unsigned int events);

static int uh_socket_bind(const char *host, const char *port,
                          struct addrinfo *hints, int do_tls,
                          struct config *conf)
{
	int sock = -1;
	int yes = 1;
	int status;
	int bound = 0;

#ifdef linux
	int tcp_ka_idl, tcp_ka_int, tcp_ka_cnt;
#endif

	struct listener *l = NULL;
	struct addrinfo *addrs = NULL, *p = NULL;

	if ((status = getaddrinfo(host, port, hints, &addrs)) != 0)
	{
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(status));
	}

	/* try to bind a new socket to each found address */
	for (p = addrs; p; p = p->ai_next)
	{
		/* get the socket */
		if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("socket()");
			goto error;
		}

		/* "address already in use" */
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
		{
			perror("setsockopt()");
			goto error;
		}

		/* TCP keep-alive */
		if (conf->tcp_keepalive > 0)
		{
#ifdef linux
			tcp_ka_idl = 1;
			tcp_ka_cnt = 3;
			tcp_ka_int = conf->tcp_keepalive;
#endif

			if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes))
#ifdef linux
			    || setsockopt(sock, SOL_TCP, TCP_KEEPIDLE,  &tcp_ka_idl, sizeof(tcp_ka_idl))
			    || setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, &tcp_ka_int, sizeof(tcp_ka_int))
			    || setsockopt(sock, SOL_TCP, TCP_KEEPCNT,   &tcp_ka_cnt, sizeof(tcp_ka_cnt))
#endif
				)
			{
			    fprintf(stderr, "Notice: Unable to enable TCP keep-alive: %s\n",
			    	strerror(errno));
			}
		}

		/* required to get parallel v4 + v6 working */
		if (p->ai_family == AF_INET6)
		{
			if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) == -1)
			{
				perror("setsockopt()");
				goto error;
			}
		}

		/* bind */
		if (bind(sock, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("bind()");
			goto error;
		}

		/* listen */
		if (listen(sock, UH_LIMIT_CLIENTS) == -1)
		{
			perror("listen()");
			goto error;
		}

		/* add listener to global list */
		if (!(l = uh_listener_add(sock, conf)))
		{
			fprintf(stderr, "uh_listener_add(): Failed to allocate memory\n");
			goto error;
		}

#ifdef HAVE_TLS
		/* init TLS */
		l->tls = do_tls ? conf->tls : NULL;
#endif

		/* add socket to uloop */
		fd_cloexec(sock);
		uh_ufd_add(&l->fd, uh_listener_cb, ULOOP_READ|ULOOP_ERROR_CB);

		bound++;
		continue;

		error:
		if (sock > 0)
			close(sock);
	}

	freeaddrinfo(addrs);

	return bound;
}

static struct http_request * uh_http_header_parse(struct client *cl,
												  char *buffer, int buflen)
{
	char *method  = buffer;
	char *path    = NULL;
	char *version = NULL;

	char *headers = NULL;
	char *hdrname = NULL;
	char *hdrdata = NULL;

	int i;
	int hdrcount = 0;

	struct http_request *req = &cl->request;


	/* terminate initial header line */
	if ((headers = strfind(buffer, buflen, "\r\n", 2)) != NULL)
	{
		buffer[buflen-1] = 0;

		*headers++ = 0;
		*headers++ = 0;

		/* find request path */
		if ((path = strchr(buffer, ' ')) != NULL)
			*path++ = 0;

		/* find http version */
		if ((path != NULL) && ((version = strchr(path, ' ')) != NULL))
			*version++ = 0;


		/* check method */
		if (method && !strcmp(method, "GET"))
			req->method = UH_HTTP_MSG_GET;
		else if (method && !strcmp(method, "POST"))
			req->method = UH_HTTP_MSG_POST;
		else if (method && !strcmp(method, "HEAD"))
			req->method = UH_HTTP_MSG_HEAD;
		else if (method && !strcmp(method, "PUT"))
			req->method = UH_HTTP_MSG_PUT;
		else
		{
			/* invalid method */
			uh_http_response(cl, 405, "Method Not Allowed");
			return NULL;
		}

		/* check path */
		if (!path || !strlen(path))
		{
			/* malformed request */
			uh_http_response(cl, 400, "Bad Request");
			return NULL;
		}
		else
		{
			req->url = path;
		}

		/* check version */
		/* removed by wanghao, This would be CWE-757 weakness, may cause downgrade attack.
		CVE-2017-7656 is a similar issue. Here we refuse the HTTP/0.9 request, it's obsolete. */
		/*if (version && !strcmp(version, "HTTP/0.9"))
			req->version = UH_HTTP_VER_0_9;
		else*/ 
		if (version && !strcmp(version, "HTTP/1.0"))
			req->version = UH_HTTP_VER_1_0;
		else if (version && !strcmp(version, "HTTP/1.1"))
			req->version = UH_HTTP_VER_1_1;
		else
		{
			/* unsupported version */
			uh_http_response(cl, 400, "Bad Request");
			return NULL;
		}

		D("SRV: %s %s %s\n",
		  http_methods[req->method], req->url, http_versions[req->version]);

		/* process header fields */
		for (i = (int)(headers - buffer); i < buflen; i++)
		{
			/* found eol and have name + value, push out header tuple */
			if (hdrname && hdrdata && (buffer[i] == '\r' || buffer[i] == '\n'))
			{
				buffer[i] = 0;

				/* store */
				if ((hdrcount + 1) < array_size(req->headers))
				{
					D("SRV: HTTP: %s: %s\n", hdrname, hdrdata);

					req->headers[hdrcount++] = hdrname;
					req->headers[hdrcount++] = hdrdata;

					hdrname = hdrdata = NULL;
				}

				/* too large */
				else
				{
					D("SRV: HTTP: header too big (too many headers)\n");
					uh_http_response(cl, 413, "Request Entity Too Large");
					return NULL;
				}
			}

			/* have name but no value and found a colon, start of value */
			else if (hdrname && !hdrdata &&
					 ((i+1) < buflen) && (buffer[i] == ':'))
			{
				buffer[i] = 0;
				hdrdata = &buffer[i+1];

				while ((hdrdata + 1) < (buffer + buflen) && *hdrdata == ' ')
					hdrdata++;
			}

			/* have no name and found [A-Za-z], start of name */
			else if (!hdrname && isalpha(buffer[i]))
			{
				hdrname = &buffer[i];
			}
		}

		/* valid enough */
		req->redirect_status = 200;
		return req;
	}

	/* Malformed request */
	uh_http_response(cl, 400, "Bad Request");
	return NULL;
}

static bool uh_http_header_check_method(const char *buf, ssize_t rlen)
{
	int i;

	for (i = 0; i < sizeof(http_methods)/sizeof(http_methods[0]); i++)
		if (!strncmp(buf, http_methods[i], min(rlen, strlen(http_methods[i]))))
			return true;

	return false;
}


static struct http_request * uh_http_header_recv(struct client *cl)
{
	char *bufptr = cl->httpbuf.buf;
	char *idxptr = NULL;

	ssize_t blen = sizeof(cl->httpbuf.buf)-1;
	ssize_t rlen = 0;

	memset(bufptr, 0, sizeof(cl->httpbuf.buf));

	while (blen > 0)
	{
		/* receive data */
		ensure_out(rlen = uh_tcp_recv(cl, bufptr, blen));
		D("SRV: Client(%d) peek(%d) = %d\n", cl->fd.fd, blen, rlen);

		if (rlen <= 0)
		{
			D("SRV: Client(%d) dead [%s]\n", cl->fd.fd, strerror(errno));
			return NULL;
		}

		/* first read attempt, check for valid method signature */
		if ((bufptr == cl->httpbuf.buf) &&
		    !uh_http_header_check_method(bufptr, rlen))
		{
			D("SRV: Client(%d) no valid HTTP method, abort\n", cl->fd.fd);
			uh_http_response(cl, 400, "Bad Request");
			return NULL;
		}

		blen -= rlen;
		bufptr += rlen;

		if ((idxptr = strfind(cl->httpbuf.buf, sizeof(cl->httpbuf.buf),
							  "\r\n\r\n", 4)))
		{
			/* header read complete ... */
			cl->httpbuf.ptr = idxptr + 4;
			cl->httpbuf.len = bufptr - cl->httpbuf.ptr;

			return uh_http_header_parse(cl, cl->httpbuf.buf,
										(cl->httpbuf.ptr - cl->httpbuf.buf));
		}
	}

	/* request entity too large */
	D("SRV: HTTP: header too big (buffer exceeded)\n");
	uh_http_response(cl, 413, "Request Entity Too Large");

out:
	return NULL;
}

#if defined(HAVE_LUA) || defined(HAVE_CGI)
static int uh_path_match(const char *prefix, const char *url)
{
	if ((strstr(url, prefix) == url) &&
		((prefix[strlen(prefix)-1] == '/') ||
		 (strlen(url) == strlen(prefix))   ||
		 (url[strlen(prefix)] == '/')))
	{
		return 1;
	}

	return 0;
}
#endif

static bool uh_is_guest_block_url(char *url)
{
	char tmp_path[UH_MAX_PATH_LEN] = {0};
	
	memset(tmp_path, 0, sizeof(tmp_path));
	sprintf(tmp_path, "webpages/init.%shtml", webpage_time);
	if(uh_path_match("webpages/pages", url)
			|| uh_path_match("webpages/login.html", url)
			|| uh_path_match(tmp_path, url))
	{
		return true;
	}

	return false;
}

#ifdef HAVE_TLS
static void uh_check_client_access(struct client *cl, struct sockaddr_in *addr)
{
    /* check remote or local access */
    if((MODE_AP == g_admin_cfg.mode)
			|| uh_check_local_access(&g_admin_cfg, &(addr->sin_addr)))
    {
        cl->local_access = true;
        
        /* check if local https is on */
        cl->local_https_on = uh_check_local_https_enable(&g_admin_cfg);
    }
    else
    {
        cl->local_access = false;
    }
	
	HTTPS_D(" *** SRV: %s cl->local_access %u, cl->local_https_on %u\n", 
						cl->server->tls ? "HTTPS" : "HTTP",
						cl->local_access, cl->local_https_on);
    
}

static void uh_check_client_access_v6(struct client *cl, struct sockaddr_in6 *addr6)
{
    /* check remote or local access */
    if((MODE_AP == g_admin_cfg.mode) || uh_check_local_access_v6(&g_admin_cfg, &addr6->sin6_addr))
    {
        cl->local_access = true;
        
        /* check if local https is on */
        cl->local_https_on = uh_check_local_https_enable(&g_admin_cfg);
    }
    else
    {
        cl->local_access = false;
    }

    HTTPS_D(" *** SRV: %s cl->local_access %u, cl->local_https_on %u\n", 
						cl->server->tls ? "HTTPS" : "HTTP",
						cl->local_access, cl->local_https_on);
    
}

static bool uh_new_session_url_match(char *url, char *target_uls[], bool match_root)
{
	int index = 0;
	char tmp_path[UH_MAX_PATH_LEN] = {0};

	if(NULL == url)
	{
		HTTPS_D(" *** SRV: url is empty!!!\n");
		return false;
	}
	
	if(NULL == target_uls)
	{
		/* match all */
		HTTPS_D(" *** SRV: url = %s, always match\n", url);
		return true;
	}
	
	if(0 == strcmp(url, "/"))
	{
		HTTPS_D(" *** SRV: url = %s, match %s\n", url, "/");
		return match_root;
	}
	
	index = 0;
	while(target_uls[index])
	{
		memset(tmp_path, 0, sizeof(tmp_path));
		sprintf(tmp_path, target_uls[index], webpage_time);
		//HTTPS_D(" *** SRV: try to match tmp url = %s\n", tmp_path);
		if(strstr(url, tmp_path))
		//if(uh_path_match(tmp_path, url))
		{
			HTTPS_D(" *** SRV: url = %s, match %s\n", url, tmp_path);
			return true;
		}
		index++;
	}

	return false;
}

static bool uh_check_https_redirect(struct client *cl, struct http_request *req, struct path_info *pin)
{
	struct config *conf = cl->server->conf;
	int header_index = 0;
	char *req_host = NULL;
    char *host_tok = NULL;
    char *addr6_tok = NULL;
	char *cmp_path = NULL;
	char  *userAgent = NULL;
		
    if(conf->tls && cl->server && !cl->server->tls)
    {
        HTTPS_D(" *** SRV: http request url = %s\n", req->url);
		/* redict all remote http access to remote https,
        		and redict new local http access to local https login if local https is on */
        HTTPS_D(" *** SRV: cl->local_access %u, cl->local_https_on %u\n", 
							cl->local_access, cl->local_https_on);
		//cmp_path = (NULL == pin) ? req->url : pin->phys;
		if(pin)
		{
			cmp_path = pin->phys;
			HTTPS_D(" *** SRV: http compare pin phys = %s\n", cmp_path);
		}
		else
		{
			cmp_path = req->url;
			HTTPS_D(" *** SRV: http compare req url = %s\n", cmp_path);
		}
        if((!cl->local_access && uh_new_session_url_match(req->url, NULL, true))
			|| (cl->local_access && cl->local_https_on
					&& uh_new_session_url_match(cmp_path, new_session_urls, true)))
        {
            /* get HOST */
            foreach_header(header_index, req->headers)
            {
                if ( !strcasecmp(req->headers[header_index], "Host") )
                {
				    req_host = strdup(req->headers[header_index + 1]);
					continue;
                }
				if ( !strcasecmp(req->headers[header_index], "User-Agent") )
				{
					userAgent = strdup(req->headers[header_index + 1]);
				}
            }

            if(req_host)
            {
                HTTPS_D(" *** SRV: http request Host = %s\n", req_host);
                if (cl->peeraddr.sin6_family == AF_INET6)
                {
	                addr6_tok = strchr(req_host, ']');
	                if (addr6_tok)
	                {
	                    host_tok = strchr(addr6_tok, ':');
	                    if(host_tok)
	                    {
	                        *host_tok = '\0';
	                        HTTPS_D("ipv6 has configured the port\n");
	                    }
	                }
                }
                else
                {
	                host_tok = strchr(req_host, ':');
	                if(host_tok)
	                {
	                    *host_tok = '\0';
	                }
                }
            }
			else
			{
				return false;
			}

			if(!cl->local_access)
			{
	            HTTPS_D(" *** SRV: redirect remote http request to https://%s:%d%s\n", 
	                req_host, conf->admin_cfg->remote_https_port, req->url);
	            
	            uh_http_sendf(cl, NULL,
					"HTTP/1.1 302 Found\r\n"
					"Location: https://%s:%d%s\r\n"
					"Connection: close\r\n\r\n",
					    req_host, conf->admin_cfg->remote_https_port, req->url
				);
			}
			else
			{
		#if defined(SUPPORT_WEBMOBILE)
				/* ����"/"ʱ������User-Agent�ж�webmobile����webpages */
				if ( strstr(req->url, "webpages") || ( !strcmp(req->url, "/") && !matchMobile(userAgent) ) )
				{
					HTTPS_D(" *** SRV: redirect local http request to https://%s%s\n", 
					req_host, UH_LOGIN_PAGE);				 
				
					uh_http_sendf(cl, NULL,
						"HTTP/1.1 302 Found\r\n"
						"Location: https://%s%s\r\n"
						"Connection: close\r\n\r\n",
							req_host, UH_LOGIN_PAGE
					);
				}
				else
				{
					HTTPS_D(" *** SRV: redirect local http request to https://%s%s\n", 
					req_host, UH_WEBMOBILE_LOGIN_PAGE); 			  
				
					uh_http_sendf(cl, NULL,
						"HTTP/1.1 302 Found\r\n"
						"Location: https://%s%s\r\n"
						"Connection: close\r\n\r\n",
							req_host, UH_WEBMOBILE_LOGIN_PAGE
					);
				}
		#else

				HTTPS_D(" *** SRV: redirect local http request to https://%s%s\n", 
				req_host, UH_LOGIN_PAGE);				 
			
				uh_http_sendf(cl, NULL,
					"HTTP/1.1 302 Found\r\n"
					"Location: https://%s%s\r\n"
					"Connection: close\r\n\r\n",
						req_host, UH_LOGIN_PAGE
				);

		#endif

				
			}
            free(req_host);
			free(userAgent);
		    return true;
		}
    }

	return false;
}

static bool uh_check_https_disabled(struct client *cl, struct http_request *req, struct path_info *pin)
{
	struct config *conf = cl->server->conf;
	char* cmp_path = NULL;
	char path_phys[UH_MAX_PATH_LEN] = {0};
	
    if(conf->tls && cl->server && cl->server->tls)
    {
        HTTPS_D(" *** SRV: https request url = %s\n", req->url);
		/* forbidden new local https access if local https is off */
		HTTPS_D(" *** SRV: cl->local_access %u, cl->local_https_on %u\n", 
							cl->local_access, cl->local_https_on);
		//cmp_path = (NULL == pin) ? req->url : pin->phys;
		if(pin)
		{
			cmp_path = pin->phys;
			HTTPS_D(" *** SRV: https compare pin phys = %s\n", cmp_path);
		}
		else
		{
			cmp_path = req->url;
			HTTPS_D(" *** SRV: https compare req url = %s\n", cmp_path);
		}
		
        if( cl->local_access && (!cl->local_https_on)
			&& uh_new_session_url_match(cmp_path, new_session_urls, false) )
        {
			HTTPS_D(" *** SRV: local https disabled, block %s.\n", req->url);
	#if defined(SUPPORT_WEBMOBILE)
			if ( strstr(req->url, "webpages") )
			{					
				sprintf(path_phys, "%s" UH_HTTPS_403PAGE, conf->docroot, webpage_time);
			}
			else
			{
				sprintf(path_phys, "%s" UH_WEBMOBILE_HTTPS_403PAGE, conf->docroot);
			}
	#else
			sprintf(path_phys, "%s" UH_HTTPS_403PAGE, conf->docroot, webpage_time);
	#endif
			uh_http_sendht(cl, 403, "Forbidden", path_phys);
			return true;
		}
    }
	return false;
}
#endif

#if 0
/*#ifdef CLOUD_SUPPORT*/
static bool isGuestClient(struct sockaddr_in6 * client)
{
	static struct sockaddr_in6 lastAccessClient;
	static struct timespec lastAccessTime;
	static int guest = 0;
	struct sockaddr_in * pl = NULL;
	struct sockaddr_in * pc = NULL;
	struct timespec now;
	char cmd[256] = {0};
	int retCode = 0;
	int match = 0;

	/*Just block this request when client ip is NULL.*/
	if (!client)
	{
		return 1;
	}

	clock_gettime(CLOCK_MONOTONIC, &now);

	if (client->sin6_family == AF_INET)
	{
		pl = (struct sockaddr_in *)&lastAccessClient;
		pc = (struct sockaddr_in *)client;
		if (pl->sin_addr.s_addr == pc->sin_addr.s_addr)
		{
			match = 1;
		}
	}
	else if (!memcmp(&lastAccessClient.sin6_addr, &client->sin6_addr, 16))
	{
		match = 1;
	}

	/*we need to flush guset information after 2 second .*/
	if (!match || now.tv_sec - lastAccessTime.tv_sec > 2)
	{
		/*when the cmd exec success, means the ip is from guestnetwork*/
		sprintf(cmd,"ubus call client_mgmt get {\\\"request_type\\\":0} | "
			"grep -A 4 \'\"%s\"\' |"
			"grep \'\"GUEST\"\'",sa_straddr(client));
		retCode = system(cmd);
		guest = WEXITSTATUS(retCode)? 0 : 1;
		if (!match)
		{
			memcpy(&lastAccessClient, client, sizeof(struct sockaddr_in6));
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &lastAccessTime);

	return guest;
}
#endif /*CLOUD_SUPPORT*/

#ifdef SUPPORT_MESH_WEB_DEBUG
static bool uh_check_repeater_disabled(struct client *cl, struct http_request *req)
{
	HTTPS_D(" *** SRV: https request url = %s\n", req->url);
	if (re_access == 0)
	{
		uh_http_sendhf(cl, 403, "Forbidden",
						   "Rejected request under RE mode.");
		return true;
	}
	return false;
}
#endif

static bool uh_dispatch_request(struct client *cl, struct http_request *req)
{
	struct path_info *pin = NULL;
#ifdef HAVE_CGI
	struct interpreter *ipr = NULL;
#endif
	struct config *conf = cl->server->conf;
	bool no_cache = false;

#ifdef SUPPORT_MESH_WEB_DEBUG
	if (MODE_RE == g_admin_cfg.mode)
	{
		if (!strcmp(req->url, MESH_WEB_DEBUG_ENABLE_URL))
		{
			re_access = 1;
			uh_http_sendhf(cl, 200, "OK", "200");
		}
		else if (!strcmp(req->url, MESH_WEB_DEBUG_DISABLE_URL))
		{
			re_access = 0;
			uh_http_sendhf(cl, 200, "OK", "200");
		}

		// if (uh_check_repeater_disabled(cl,req))
		// {
		// 	return false;
		// }
	}
#endif
#ifdef SUPPORT_AGILE_CONFIG
	if(!strcmp(req->url, AGILE_CONFIG_RESET_URL) && cl->local_access == true)
	{
		int countnum = 0;
		long nowtime = 0;
		struct timespec time;
		FILE *fd_agile;
		fd_agile = fopen(AGILE_CONFIG_RESET_FILE, "r");
		//clock_gettime(CLOCK_REALTIME, &time);
		clock_gettime(CLOCK_MONOTONIC, &time);
		nowtime = time.tv_sec;
		if(!fd_agile)
		{
			fd_agile = fopen(AGILE_CONFIG_RESET_FILE, "w+");
			fprintf(fd_agile, "%d\n", 1);
			fprintf(fd_agile, "%ld\n", nowtime);
			fclose(fd_agile);
			uh_http_sendhf(cl, 200, "OK", "200");
		}
		else
		{
			fscanf(fd_agile, "%d\n", &countnum);
			fclose(fd_agile);
			if(countnum > 5)
			{
				uh_http_sendhf(cl, 403, "OK", "403");
			}
			else
			{
				countnum++;
				fd_agile = fopen(AGILE_CONFIG_RESET_FILE, "w+");	
				fprintf(fd_agile, "%d\n", countnum);
				fprintf(fd_agile, "%ld\n", nowtime);
				fclose(fd_agile);
				if(countnum > 5)
				{
					uh_http_sendhf(cl, 403, "OK", "403");
				}
				else
				{
					uh_http_sendhf(cl, 200, "OK", "200");
				}
			}
		}
	}
#endif

#if 0
/*#ifdef CLOUD_SUPPORT*/
	int isguest = 0;
#endif

#if 0
/*#ifdef CLOUD_SUPPORT*/
	if(isGuestClient(&cl->peeraddr))
	{
		isguest = 1;
		if(uh_is_guest_block_url(req->url))
		{
			return false;
		}
	}
#endif

#ifdef HAVE_TLS		
    if(uh_check_https_disabled(cl, req, NULL))
    {
		return false;
    }		
     		
    if(uh_check_https_redirect(cl, req, NULL))
    {
		return false;
	}
#endif

#ifdef HAVE_LUA
	/* Lua request? */
	if (conf->lua_state &&
		uh_path_match(conf->lua_prefix, req->url))
	{
		return conf->lua_request(cl, conf->lua_state);
	}
	else
#endif

#ifdef HAVE_UBUS
	/* ubus request? */
	if (conf->ubus_state &&
		uh_path_match(conf->ubus_prefix, req->url))
	{
		return conf->ubus_request(cl, conf->ubus_state);
	}
	else
#endif

	/* dispatch request */
	if ((pin = uh_path_lookup(cl, req->url)) != NULL)
	{
		/* auth ok? */
		if (!pin->redirected && uh_auth_check(cl, req, pin))
		{
#ifdef HAVE_CGI
			if (uh_path_match(conf->cgi_prefix, pin->name) ||
				(ipr = uh_interpreter_lookup(pin->phys)) != NULL)
			{
#if 0
/*#ifdef CLOUD_SUPPORT*/
				if(isguest)
				{
					if(!( 
						strstr(req->url, "stok=/wan_error") || 
						strstr(req->url, "stok=/upgrade")
						))
					{
						return false;
					}
				}
#endif

				return uh_cgi_request(cl, pin, ipr);
			}
#endif
#if 0
/*#ifdef CLOUD_SUPPORT*/
			if(isguest)
			{
				char tmp_path[128];
				memset(tmp_path, 0, sizeof(tmp_path));
				sprintf(tmp_path, "webpages/init.%shtml", webpage_time);
				if( 
					strstr(pin->phys, "webpages/pages") || 
					strstr(pin->phys, "webpages/login.html") || 
					strstr(pin->phys, tmp_path)
					)
				{
					return false;
				}
			}
#endif

#ifdef HAVE_TLS		
			if(uh_check_https_disabled(cl, req, pin))
			{
				return false;
			}		
							
			if(uh_check_https_redirect(cl, req, pin))
			{
				return false;
			}

			no_cache = uh_new_session_url_match(pin->phys, no_cache_files, true);
#endif

			return uh_file_request(cl, pin, no_cache);
		}
	}

	/* 404 - pass 1 */
	else
	{
		/* Try to invoke an error handler */
		if ((pin = uh_path_lookup(cl, conf->error_handler)) != NULL)
		{
			/* auth ok? */
			if (uh_auth_check(cl, req, pin))
			{
				req->redirect_status = 404;
#ifdef HAVE_CGI
				if (uh_path_match(conf->cgi_prefix, pin->name) ||
					(ipr = uh_interpreter_lookup(pin->phys)) != NULL)
				{
					return uh_cgi_request(cl, pin, ipr);
				}
#endif
				return uh_file_request(cl, pin, no_cache);
			}
		}

		/* 404 - pass 2 */
		else
		{
			uh_http_sendhf(cl, 404, "Not Found", "No such file or directory");
		}
	}

	return false;
}

static void uh_socket_cb(struct uloop_fd *u, unsigned int events);

#if defined(HAVE_TLS) && defined(TLS_ACCEPT_ASYNC)
static void uh_tls_accept_timeout_cb(struct uloop_timeout *t)
{
	struct client *cl = container_of(t, struct client, timeout);
	struct listener *serv = cl->server;
	struct config *conf = serv->conf;
	
	HTTPS_ASYNC("SRV: Client(%d) tls handshake timed out, remove\n", cl->fd.fd);

	/* NOTICE: tls handshake is not finished, stop it */
	conf->tls_timeout(cl);
	
	uh_http_response(cl, 400, "Bad Request");
	
	/* remove from global client list */
	uh_client_remove(cl);
}

static void uh_listener_accept_async(struct uloop_fd *u, unsigned int events)
{
	struct client *cl = container_of(u, struct client, fd);	
	struct listener *serv = cl->server;
	struct config *conf = serv->conf;
	int state = TLS_ACCEPTED;
	unsigned int event = 0;
	
	HTTPS_ASYNC("SRV: Client(%d) socket readable\n", cl->fd.fd);
	
	state = conf->tls_accept(cl);
	if (TLS_FAILED == state)
	{
		HTTPS_ASYNC("SRV: Client(%d) SSL handshake failed, drop\n", cl->fd.fd);

		/* remove from global client list */
		uh_http_response(cl, 400, "Bad Request");
		uh_client_remove(cl);
		return;
	}
	else if(TLS_ACCEPTED == state)
	{
		/* cancel tls handshake timeout */
		if (cl->timeout.pending)
		{
			HTTPS_ASYNC("SRV: Client(%d) SSL accepted, cancel *** TIMEOUT ***\n", cl->fd.fd);
			uloop_timeout_cancel(&cl->timeout);
		}
		
		/* add client socket to global fdset */
		uh_ufd_add(&cl->fd, uh_socket_cb, ULOOP_READ | ULOOP_ERROR_CB);		
	}
	else	/* accepting */
	{	
		if(TLS_WANT_READ == state)
		{
			HTTPS_ASYNC("SRV: Client(%d) SSL accepting, WANT READ\n", cl->fd.fd);
			event = ULOOP_READ | ULOOP_ERROR_CB;
		}
		else
		{
			HTTPS_ASYNC("SRV: Client(%d) SSL accepting, WANT WRITE\n", cl->fd.fd);
			event = ULOOP_WRITE | ULOOP_ERROR_CB;
		}
		uh_ufd_add(&cl->fd, uh_listener_accept_async, event);

		/* set tls handshake timeout */
		if (!cl->timeout.pending)
		{
			HTTPS_ASYNC("SRV: Client(%d) SSL accepting, set *** TIMEOUT ***\n", cl->fd.fd);
			cl->timeout.cb = uh_tls_accept_timeout_cb;
			uloop_timeout_set(&cl->timeout, conf->network_timeout * 1000);
		}
	}
}

static void uh_listener_cb(struct uloop_fd *u, unsigned int events)
{
	int new_fd;
	struct listener *serv;
	struct client *cl;
	struct config *conf;

	struct sockaddr_in6 sa;
	socklen_t sl = sizeof(sa);
    
	serv = container_of(u, struct listener, fd);
	conf = serv->conf;

	/* defer client if maximum number of requests is exceeded */
	if (serv->n_clients >= conf->max_requests)
		return;

	/* handle new connections */
	if ((new_fd = accept(u->fd, (struct sockaddr *)&sa, &sl)) != -1)
	{
		HTTPS_ASYNC("SRV: Server(%d) accept => Client(%d)\n", u->fd, new_fd);

		/* add to global client list */
		if ((cl = uh_client_add(new_fd, serv, &sa)) != NULL)
		{
			if(!conf->tls)
			{
				/* add client socket to global fdset */
				uh_ufd_add(&cl->fd, uh_socket_cb, ULOOP_READ | ULOOP_ERROR_CB);
				fd_cloexec(cl->fd.fd);
			}
			else
			{
				/* add for https redirect check */
				if (sa.sin6_family == AF_INET)
				{
					HTTPS_ASYNC("IPv4 adress access\n");
					uh_check_client_access(cl, (struct sockaddr_in *)&sa);
				}
				else if (sa.sin6_family == AF_INET6)
				{
					HTTPS_ASYNC("IPv6 adress access\n");
					uh_check_client_access_v6(cl, (struct sockaddr_in6 *)&sa);
				}
			    /* add ended */	

				fd_nonblock(cl->fd.fd);	
				/* close on exec */
				fd_cloexec(cl->fd.fd);

				uh_listener_accept_async(&cl->fd, ULOOP_READ);				
			}
		}

		/* insufficient resources */
		else
		{
			fprintf(stderr, "uh_client_add(): Cannot allocate memory\n");
			close(new_fd);
		}
	}
}
#else
static void uh_listener_cb(struct uloop_fd *u, unsigned int events)
{
	int new_fd;
	struct listener *serv;
	struct client *cl;
	struct config *conf;

	struct sockaddr_in6 sa;
	socklen_t sl = sizeof(sa);

	serv = container_of(u, struct listener, fd);
	conf = serv->conf;

	/* defer client if maximum number of requests is exceeded */
	if (serv->n_clients >= conf->max_requests)
		return;

	/* handle new connections */
	if ((new_fd = accept(u->fd, (struct sockaddr *)&sa, &sl)) != -1)
	{
		D("SRV: Server(%d) accept => Client(%d)\n", u->fd, new_fd);

		/* add to global client list */
		if ((cl = uh_client_add(new_fd, serv, &sa)) != NULL)
		{
			/* add client socket to global fdset */
			uh_ufd_add(&cl->fd, uh_socket_cb, ULOOP_READ|ULOOP_ERROR_CB);
			fd_cloexec(cl->fd.fd);

#ifdef HAVE_TLS
			/* setup client tls context */
			if (conf->tls)
			{
				if (conf->tls_accept(cl) < 1)
				{
					D("SRV: Client(%d) SSL handshake failed, drop\n", new_fd);

					/* remove from global client list */
					uh_http_response(cl, 400, "Bad Request");
					uh_client_remove(cl);
					return;
				}
                              
				uh_check_client_access(cl, (struct sockaddr_in *)&sa);               
			}
#endif
		}

		/* insufficient resources */
		else
		{
			fprintf(stderr, "uh_client_add(): Cannot allocate memory\n");
			close(new_fd);
		}
	}
}
#endif

static void uh_client_cb(struct client *cl, unsigned int events);

static void uh_rpipe_cb(struct uloop_fd *u, unsigned int events)
{
	struct client *cl = container_of(u, struct client, rpipe);

	D("SRV: Client(%d) rpipe readable\n", cl->fd.fd);

	uh_client_cb(cl, ULOOP_WRITE);
}

static void uh_socket_cb(struct uloop_fd *u, unsigned int events)
{
	struct client *cl = container_of(u, struct client, fd);

	D("SRV: Client(%d) socket readable\n", cl->fd.fd);

	uh_client_cb(cl, ULOOP_READ);
}

#if defined(HAVE_CGI) || defined(HAVE_LUA) || defined(HAVE_UBUS)
static void uh_child_cb(struct uloop_process *p, int rv)
{
	struct client *cl = container_of(p, struct client, proc);

	D("SRV: Client(%d) child(%d) dead\n", cl->fd.fd, cl->proc.pid);

	uh_client_cb(cl, ULOOP_READ | ULOOP_WRITE);
}

static void uh_kill9_cb(struct uloop_timeout *t)
{
	struct client *cl = container_of(t, struct client, timeout);

	if (!kill(cl->proc.pid, 0))
	{
		D("SRV: Client(%d) child(%d) kill(SIGKILL)...\n",
		  cl->fd.fd, cl->proc.pid);

		kill(cl->proc.pid, SIGKILL);
	}
}

static void uh_timeout_cb(struct uloop_timeout *t)
{
	struct client *cl = container_of(t, struct client, timeout);

	D("SRV: Client(%d) child(%d) timed out\n", cl->fd.fd, cl->proc.pid);

	if (!kill(cl->proc.pid, 0))
	{
		D("SRV: Client(%d) child(%d) kill(SIGTERM)...\n",
		  cl->fd.fd, cl->proc.pid);

		kill(cl->proc.pid, SIGTERM);

		cl->timeout.cb = uh_kill9_cb;
		uloop_timeout_set(&cl->timeout, 1000);
	}
}
#endif

static void uh_client_cb(struct client *cl, unsigned int events)
{
	int i = 0;
	struct config *conf = NULL;
	struct http_request *req = NULL;

	conf = cl->server->conf;

	D("SRV: Client(%d) enter callback\n", cl->fd.fd);

	/* undispatched yet */
	if (!cl->dispatched)
	{
		/* we have no headers yet and this was a write event, ignore... */
		if (!(events & ULOOP_READ))
		{
			D("SRV: Client(%d) ignoring write event before headers\n", cl->fd.fd);
			return;
		}

		/* attempt to receive and parse headers */
		if (!(req = uh_http_header_recv(cl)))
		{
			D("SRV: Client(%d) failed to receive header\n", cl->fd.fd);
			uh_client_shutdown(cl);
			return;
		}

		/* process expect headers */
		foreach_header(i, req->headers)
		{
			if (strcasecmp(req->headers[i], "Expect"))
				continue;

			if (strcasecmp(req->headers[i+1], "100-continue"))
			{
				D("SRV: Client(%d) unknown expect header (%s)\n",
				  cl->fd.fd, req->headers[i+1]);

				uh_http_response(cl, 417, "Precondition Failed");
				uh_client_shutdown(cl);
				return;
			}
			else
			{
				D("SRV: Client(%d) sending HTTP/1.1 100 Continue\n", cl->fd.fd);

				uh_http_sendf(cl, NULL, "HTTP/1.1 100 Continue\r\n\r\n");
				cl->httpbuf.len = 0; /* client will re-send the body */
				break;
			}
		}

		/* RFC1918 filtering */
		if (conf->rfc1918_filter &&
			sa_rfc1918(&cl->peeraddr) && !sa_rfc1918(&cl->servaddr))
		{
			uh_http_sendhf(cl, 403, "Forbidden",
						   "Rejected request from RFC1918 IP "
						   "to public server address");

			uh_client_shutdown(cl);
			return;
		}

		/* dispatch request */
		if (!uh_dispatch_request(cl, req))
		{
			D("SRV: Client(%d) failed to dispach request\n", cl->fd.fd);
			uh_client_shutdown(cl);
			return;
		}

		/* request handler spawned a pipe, register handler */
		if (cl->rpipe.fd > -1)
		{
			D("SRV: Client(%d) pipe(%d) spawned\n", cl->fd.fd, cl->rpipe.fd);
			uh_ufd_add(&cl->rpipe, uh_rpipe_cb, ULOOP_READ|ULOOP_ERROR_CB);
		}

		/* request handler spawned a child, register handler */
#if defined(HAVE_CGI) || defined(HAVE_LUA) || defined(HAVE_UBUS)
		if (cl->proc.pid)
		{
			D("SRV: Client(%d) child(%d) spawned\n", cl->fd.fd, cl->proc.pid);

			cl->proc.cb = uh_child_cb;
			uloop_process_add(&cl->proc);

			cl->timeout.cb = uh_timeout_cb;
			uloop_timeout_set(&cl->timeout,
#ifdef CONFIG_MEM_PROTECT
					cl->long_term ? LONG_TERM_PROCESS_TIMEOUT * 1000: conf->script_timeout * 1000
#else
					conf->script_timeout * 1000
#endif
					);
		}
#endif

		/* header processing complete */
		D("SRV: Client(%d) dispatched\n", cl->fd.fd);
		cl->dispatched = true;
	}

	if (!cl->cb(cl))
	{
		D("SRV: Client(%d) response callback signalized EOF\n", cl->fd.fd);
		uh_client_shutdown(cl);
		return;
	}
}

#ifdef HAVE_TLS
static int uh_inittls(struct config *conf)
{
	/* library handle */
	void *lib;

	/* already loaded */
	if (conf->tls != NULL)
		return 0;

	/* load TLS plugin */
	if (!(lib = dlopen("uhttpd_tls.so", RTLD_LAZY | RTLD_GLOBAL)))
	{
		fprintf(stderr,
				"Notice: Unable to load TLS plugin - disabling SSL support! "
				"(Reason: %s)\n", dlerror()
		);

		return 1;
	}
	else
	{
		/* resolve functions */
		if (!(conf->tls_init   = dlsym(lib, "uh_tls_ctx_init"))      ||
		    !(conf->tls_cert   = dlsym(lib, "uh_tls_ctx_cert"))      ||
		    !(conf->tls_key    = dlsym(lib, "uh_tls_ctx_key"))       ||
		    !(conf->tls_ciphers = dlsym(lib, "uh_tls_ctx_ciphers"))  ||
		    !(conf->tls_free   = dlsym(lib, "uh_tls_ctx_free"))      ||
#if defined(TLS_ACCEPT_ASYNC)
		    !(conf->tls_timeout = dlsym(lib, "uh_tls_client_timeout")) ||
#endif
		    !(conf->tls_accept = dlsym(lib, "uh_tls_client_accept")) ||
		    !(conf->tls_close  = dlsym(lib, "uh_tls_client_close"))  ||
		    !(conf->tls_recv   = dlsym(lib, "uh_tls_client_recv"))   ||
		    !(conf->tls_send   = dlsym(lib, "uh_tls_client_send")))
		{
			fprintf(stderr,
					"Error: Failed to lookup required symbols "
					"in TLS plugin: %s\n", dlerror()
			);
			exit(1);
		}

		/* init SSL context */
		if (!(conf->tls = conf->tls_init()))
		{
			fprintf(stderr, "Error: Failed to initalize SSL context\n");
			exit(1);
		}
	}

	return 0;
}
#endif

#ifdef HAVE_LUA
static int uh_initlua(struct config *conf)
{
	/* library handle */
	void *lib;

	/* already loaded */
	if (conf->lua_state != NULL)
		return 0;

	/* load Lua plugin */
	if (!(lib = dlopen("uhttpd_lua.so", RTLD_LAZY | RTLD_GLOBAL)))
	{
		fprintf(stderr,
				"Notice: Unable to load Lua plugin - disabling Lua support! "
				"(Reason: %s)\n", dlerror());

		return 1;
	}
	else
	{
		/* resolve functions */
		if (!(conf->lua_init    = dlsym(lib, "uh_lua_init"))    ||
		    !(conf->lua_close   = dlsym(lib, "uh_lua_close"))   ||
		    !(conf->lua_request = dlsym(lib, "uh_lua_request")))
		{
			fprintf(stderr,
					"Error: Failed to lookup required symbols "
					"in Lua plugin: %s\n", dlerror()
			);
			exit(1);
		}

		/* init Lua runtime if handler is specified */
		if (conf->lua_handler)
		{
			/* default lua prefix */
			if (!conf->lua_prefix)
				conf->lua_prefix = "/lua";

			conf->lua_state = conf->lua_init(conf);
		}
	}

	return 0;
}
#endif

#ifdef HAVE_UBUS
static int uh_initubus(struct config *conf)
{
	/* library handle */
	void *lib;

	/* already loaded */
	if (conf->ubus_state != NULL)
		return 0;

	/* load ubus plugin */
	if (!(lib = dlopen("uhttpd_ubus.so", RTLD_LAZY | RTLD_GLOBAL)))
	{
		fprintf(stderr,
				"Notice: Unable to load ubus plugin - disabling ubus support! "
				"(Reason: %s)\n", dlerror());

		return 1;
	}
	else if (conf->ubus_prefix)
	{
		/* resolve functions */
		if (!(conf->ubus_init    = dlsym(lib, "uh_ubus_init"))    ||
		    !(conf->ubus_close   = dlsym(lib, "uh_ubus_close"))   ||
		    !(conf->ubus_request = dlsym(lib, "uh_ubus_request")))
		{
			fprintf(stderr,
					"Error: Failed to lookup required symbols "
					"in ubus plugin: %s\n", dlerror()
			);
			exit(1);
		}

		/* initialize ubus */
		conf->ubus_state = conf->ubus_init(conf);
	}

	return 0;
}
#endif

static int get_modified_time(char *ctm)
{
	FILE *f = NULL;
	int err = 1;

	FILE *fp_webpagesTime = NULL;
	char manifest_path[128];
	memset(webpage_time, 0, sizeof(webpage_time));
	fp_webpagesTime = fopen("/etc/webpage_time", "r");
	if(fp_webpagesTime)
	{
		fgets(webpage_time, sizeof(webpage_time)-2, fp_webpagesTime);
		if (strlen(webpage_time) > 0)
		{
			webpage_time[strlen(webpage_time)] = '.';
		}	
		fclose(fp_webpagesTime);
	}

	f = fopen(CONFIG_FILE, "r");

	if(!f)
	{
		memset(manifest_path, 0, 128);
		sprintf(manifest_path, "/www/webpages/app.%smanifest", webpage_time);
		f = fopen(manifest_path, "r");
		if(!f)
		{
			return err;
		}
	}
	while (!feof(f))
	{
		if(fgets(ctm, 30, f) == NULL)
			continue;
		if (ctm[0] != '#' || ctm[5] != '.' || ctm[8] != '.' ||
			ctm[11] != '-' || ctm[14] != ':')
			continue;
		err = 0;
		break;
	}
	
	fclose(f);
	return err;
}

int main (int argc, char **argv)
{
	/* working structs */
	struct addrinfo hints;
	struct sigaction sa;
	struct config conf;

	/* maximum file descriptor number */
	int cur_fd = 0;

#ifdef HAVE_TLS
	int tls = 0;
	int keys = 0;
	int ret = 0;
#endif

	int bound = 0;
	int nofork = 0;

	/* args */
	int opt;
	char addr[128];
	char *port = NULL;

	/*the compile time*/
	char ctm[30];
	int err;

	memset(ctm, 0, 30);
	err = get_modified_time(ctm);
	if(err ==0)
	{
		struct tm m_tm;

		sscanf(ctm, "#%d.%d.%d-%d:%d:%d", &m_tm.tm_year, &m_tm.tm_mon, &m_tm.tm_mday,
			&m_tm.tm_hour, &m_tm.tm_min, &m_tm.tm_sec);
		m_tm.tm_year -= 1900;
		m_tm.tm_mon -= 1;
		m_tm.tm_isdst = -1;
		m_modified_time = mktime(&m_tm);
	}
	else
		m_modified_time = time(NULL);

	/* handle SIGPIPE, SIGINT, SIGTERM */
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGILL, &sa, NULL);

	sa.sa_handler = uh_sigterm;
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	/* prepare addrinfo hints */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE;

	/* parse args */
	memset(&conf, 0, sizeof(conf));
#ifdef HAVE_TLS
	memset(&g_admin_cfg, 0, sizeof(g_admin_cfg));
	conf.admin_cfg = &g_admin_cfg;
#endif

	uloop_init();

	while ((opt = getopt(argc, argv,
						 "fSDRC:K:N:E:I:p:s:h:c:l:L:d:r:m:n:x:i:t:T:A:u:U:")) > 0)
	{
		switch(opt)
		{
			/* [addr:]port */
			case 'p':
			case 's':
				memset(addr, 0, sizeof(addr));

				if ((port = strrchr(optarg, ':')) != NULL)
				{
					if ((optarg[0] == '[') && (port > optarg) && (port[-1] == ']'))
						memcpy(addr, optarg + 1,
							min(sizeof(addr), (int)(port - optarg) - 2));
					else
						memcpy(addr, optarg,
							min(sizeof(addr), (int)(port - optarg)));

					port++;
				}
				else
				{
					port = optarg;
				}

#ifdef HAVE_TLS
				if (opt == 's')
				{
					if (uh_inittls(&conf))
					{
						fprintf(stderr,
							"Notice: TLS support is disabled, "
							"ignoring '-s %s'\n", optarg
						);
						continue;
					}

					tls = 1;
				}
#endif

				/* bind sockets */
				bound += uh_socket_bind(addr[0] ? addr : NULL, port, &hints,
				                        (opt == 's'), &conf);
				break;

#ifdef HAVE_TLS
			/* certificate */
			case 'C':
				if (!uh_inittls(&conf))
				{
					if (conf.tls_cert(conf.tls, optarg) < 1)
					{
						fprintf(stderr,
								"Error: Invalid certificate file given\n");
						exit(1);
					}

					keys++;
				}

				break;

			/* key */
			case 'K':
				if (!uh_inittls(&conf))
				{
					if (conf.tls_key(conf.tls, optarg) < 1)
					{
						fprintf(stderr,
								"Error: Invalid private key file given\n");
						exit(1);
					}

					keys++;
				}

				break;

			/* cipher suite */
			case 'N':
				HTTPS_D("ciphers: %s \n", optarg);
				if (!uh_inittls(&conf))
				{
					if ((ret = conf.tls_ciphers(conf.tls, optarg)) < 1)
					{
						fprintf(stderr,
								"Error: Invalid cipher suites given, ret = %d\n", ret);
						exit(1);
					}
				}

				break;
#else
			case 'C':
			case 'K':
			case 'N':
				fprintf(stderr,
				        "Notice: TLS support not compiled, ignoring -%c\n",
				        opt);
				break;
#endif

			/* docroot */
			case 'h':
				if (! realpath(optarg, conf.docroot))
				{
					fprintf(stderr, "Error: Invalid directory %s: %s\n",
							optarg, strerror(errno));
					exit(1);
				}
				break;

			/* error handler */
			case 'E':
				if ((strlen(optarg) == 0) || (optarg[0] != '/'))
				{
					fprintf(stderr, "Error: Invalid error handler: %s\n",
							optarg);
					exit(1);
				}
				conf.error_handler = optarg;
				break;

			/* index file */
			case 'I':
				if ((strlen(optarg) == 0) || (optarg[0] == '/'))
				{
					fprintf(stderr, "Error: Invalid index page: %s\n",
							optarg);
					exit(1);
				}
				uh_index_add(optarg);
				break;

			/* don't follow symlinks */
			case 'S':
				conf.no_symlinks = 1;
				break;

			/* don't list directories */
			case 'D':
				conf.no_dirlists = 1;
				break;

			case 'R':
				conf.rfc1918_filter = 1;
				break;

			case 'n':
				conf.max_requests = atoi(optarg);
				break;

#ifdef HAVE_CGI
			/* cgi prefix */
			case 'x':
				conf.cgi_prefix = optarg;
				break;

			/* interpreter */
			case 'i':
				if ((optarg[0] == '.') && (port = strchr(optarg, '=')))
				{
					*port++ = 0;
					uh_interpreter_add(optarg, port);
				}
				else
				{
					fprintf(stderr, "Error: Invalid interpreter: %s\n",
							optarg);
					exit(1);
				}
				break;
#else
			case 'x':
			case 'i':
				fprintf(stderr,
				        "Notice: CGI support not compiled, ignoring -%c\n",
				        opt);
				break;
#endif

#ifdef HAVE_LUA
			/* lua prefix */
			case 'l':
				conf.lua_prefix = optarg;
				break;

			/* lua handler */
			case 'L':
				conf.lua_handler = optarg;
				break;
#else
			case 'l':
			case 'L':
				fprintf(stderr,
				        "Notice: Lua support not compiled, ignoring -%c\n",
				        opt);
				break;
#endif

#ifdef HAVE_UBUS
			/* ubus prefix */
			case 'u':
				conf.ubus_prefix = optarg;
				break;

			/* ubus socket */
			case 'U':
				conf.ubus_socket = optarg;
				break;
#else
			case 'u':
			case 'U':
				fprintf(stderr,
				        "Notice: UBUS support not compiled, ignoring -%c\n",
				        opt);
				break;
#endif

#if defined(HAVE_CGI) || defined(HAVE_LUA)
			/* script timeout */
			case 't':
				conf.script_timeout = atoi(optarg);
				break;
#endif

			/* network timeout */
			case 'T':
				conf.network_timeout = atoi(optarg);
				break;

			/* tcp keep-alive */
			case 'A':
				conf.tcp_keepalive = atoi(optarg);
				break;

			/* no fork */
			case 'f':
				nofork = 1;
				break;

			/* urldecode */
			case 'd':
				if ((port = malloc(strlen(optarg)+1)) != NULL)
				{
					/* "decode" plus to space to retain compat */
					for (opt = 0; optarg[opt]; opt++)
						if (optarg[opt] == '+')
							optarg[opt] = ' ';
					/* opt now contains strlen(optarg) -- no need to re-scan */
					memset(port, 0, opt+1);
					if (uh_urldecode(port, opt, optarg, opt) < 0)
					    fprintf(stderr, "uhttpd: invalid encoding\n");

					printf("%s", port);
					free(port);
					exit(0);
				}
				break;

			/* basic auth realm */
			case 'r':
				conf.realm = optarg;
				break;

			/* md5 crypt */
			case 'm':
				printf("%s\n", crypt(optarg, "$1$"));
				exit(0);
				break;

			/* config file */
			case 'c':
				conf.file = optarg;
				break;

			default:
				fprintf(stderr,
					"Usage: %s -p [addr:]port [-h docroot]\n"
					"	-f              Do not fork to background\n"
					"	-c file         Configuration file, default is '/etc/httpd.conf'\n"
					"	-p [addr:]port  Bind to specified address and port, multiple allowed\n"
#ifdef HAVE_TLS
					"	-s [addr:]port  Like -p but provide HTTPS on this port\n"
					"	-C file         ASN.1 server certificate file\n"
					"	-K file         ASN.1 server private key file\n"
					"	-N string       Openssl Cipher Suites\n"
#endif
					"	-h directory    Specify the document root, default is '.'\n"
					"	-E string       Use given virtual URL as 404 error handler\n"
					"	-I string       Use given filename as index for directories, multiple allowed\n"
					"	-S              Do not follow symbolic links outside of the docroot\n"
					"	-D              Do not allow directory listings, send 403 instead\n"
					"	-R              Enable RFC1918 filter\n"
					"	-n count        Maximum allowed number of concurrent requests\n"
#ifdef HAVE_LUA
					"	-l string       URL prefix for Lua handler, default is '/lua'\n"
					"	-L file         Lua handler script, omit to disable Lua\n"
#endif
#ifdef HAVE_UBUS
					"	-u string       URL prefix for HTTP/JSON handler\n"
					"	-U file         Override ubus socket path\n"
#endif
#ifdef HAVE_CGI
					"	-x string       URL prefix for CGI handler, default is '/cgi-bin'\n"
					"	-i .ext=path    Use interpreter at path for files with the given extension\n"
#endif
#if defined(HAVE_CGI) || defined(HAVE_LUA) || defined(HAVE_UBUS)
					"	-t seconds      CGI, Lua and UBUS script timeout in seconds, default is 60\n"
#endif
					"	-T seconds      Network timeout in seconds, default is 30\n"
					"	-d string       URL decode given string\n"
					"	-r string       Specify basic auth realm\n"
					"	-m string       MD5 crypt given string\n"
					"\n", argv[0]
				);

				exit(1);
		}
	}

#ifdef HAVE_TLS
	if ((tls == 1) && (keys < 2))
	{
		fprintf(stderr, "Error: Missing private key or certificate file\n");
		exit(1);
	}
#endif

	if (bound < 1)
	{
		fprintf(stderr, "Error: No sockets bound, unable to continue\n");
		exit(1);
	}

	/* default docroot */
	if (!conf.docroot[0] && !realpath(".", conf.docroot))
	{
		fprintf(stderr, "Error: Can not determine default document root: %s\n",
			strerror(errno));
		exit(1);
	}

	/* default realm */
	if (!conf.realm)
		conf.realm = "Protected Area";

	/* config file */
	uh_config_parse(&conf);

#ifdef HAVE_TLS
	uh_get_operation_mode(&g_admin_cfg);
	uh_get_local_addr(&g_admin_cfg);
	uh_get_admin_config(&g_admin_cfg);
#endif

	/* default max requests */
	if (conf.max_requests <= 0)
	{
		conf.max_requests = UH_MAX_REQUESTS;
	}

	/* default network timeout */
	if (conf.network_timeout <= 0)
		conf.network_timeout = 30;

	/* default index files */
	if (!uh_index_files)
	{
		//add for cache bug
		if (strlen(webpage_time) > 0)
		{
			char tmp_path[128];
			memset(tmp_path, 0, sizeof(tmp_path));
			sprintf(tmp_path, "index.%shtml", webpage_time);
			uh_index_add(tmp_path);
		}

		uh_index_add("index.html");
		uh_index_add("index.htm");
		uh_index_add("default.html");
		uh_index_add("default.htm");
		
	}

#if defined(HAVE_CGI) || defined(HAVE_LUA) || defined(HAVE_UBUS)
	/* default script timeout */
	if (conf.script_timeout <= 0)
		conf.script_timeout = 60;
#endif

#ifdef HAVE_CGI
	/* default cgi prefix */
	if (!conf.cgi_prefix)
		conf.cgi_prefix = "/cgi-bin";
#endif

#ifdef HAVE_LUA
	/* initialize Lua runtime */
	if (conf.lua_handler)
		uh_initlua(&conf);
#endif

#ifdef HAVE_UBUS
	/* initialize ubus client */
	if (conf.ubus_prefix)
		uh_initubus(&conf);
#endif

#ifdef HAVE_TLS    
    uh_cfg_ctx = ubus_connect(UBUS_PATH);
    if (!uh_cfg_ctx)
    {
        fprintf(stderr, "Failed to connect to ubus\n");
        exit(1);
    }

    ubus_add_uloop(uh_cfg_ctx);
    ret = ubus_add_object(uh_cfg_ctx, &uh_cfg_obj);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
        ubus_free(uh_cfg_ctx);
        exit(1);
    }    
#endif

	/* fork (if not disabled) */
	if (!nofork)
	{
		switch (fork())
		{
			case -1:
				perror("fork()");
				exit(1);

			case 0:
				/* daemon setup */
				if (chdir("/"))
					perror("chdir()");

				if ((cur_fd = open("/dev/null", O_WRONLY)) > -1)
					dup2(cur_fd, 0);

				if ((cur_fd = open("/dev/null", O_RDONLY)) > -1)
					dup2(cur_fd, 1);

				if ((cur_fd = open("/dev/null", O_RDONLY)) > -1)
					dup2(cur_fd, 2);

				break;

			default:
				exit(0);
		}
	}

	/* server main loop */
	uloop_run();

#ifdef HAVE_LUA
	/* destroy the Lua state */
	if (conf.lua_state != NULL)
		conf.lua_close(conf.lua_state);
#endif

#ifdef HAVE_UBUS
	/* destroy the ubus state */
	if (conf.ubus_state != NULL)
		conf.ubus_close(conf.ubus_state);
#endif

#ifdef HAVE_TLS
	ubus_free(uh_cfg_ctx);
#endif
	uloop_done();

	return 0;
}
