/*
 * uhttpd - Tiny single-threaded httpd - Main header
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

#ifndef _UHTTPD_

#define _BSD_SOURCE
#define _XOPEN_SOURCE 700
/*add by zhuyu@16.06.03 for gcc 3.x compile error*/
#define _DEFAULT_SOURCE 1
/*end add*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>
#include <dlfcn.h>

#include <libubox/list.h>
#include <libubox/uloop.h>


#ifdef HAVE_LUA
#include <lua.h>
#endif

#ifdef HAVE_TLS
#include <openssl/ssl.h>
#endif

/* uClibc... */
#ifndef SOL_TCP
#define SOL_TCP	6
#endif

#ifdef DEBUG
//#define D(...) fprintf(stderr, __VA_ARGS__)
#define D(fmt, args...) \
    do{ \
        fprintf(stderr, "[%d] DEBUG[%s %d] "fmt, getpid(), __func__, __LINE__, ##args); \
    }while(0)
#else
#define D(...)
#endif


#ifdef HTTPS_DEBUG
#define HTTPS_D(fmt, args...) \
    do{ \
        fprintf(stderr, "[%d] DEBUG[%s %d] "fmt, getpid(), __func__, __LINE__, ##args); \
    }while(0)
#else
#define HTTPS_D(...)
#endif

#if defined(DEBUG) || defined(HTTPS_DEBUG)
#define HTTPS_ASYNC_ON 	1
#endif
#ifdef HTTPS_ASYNC_ON
#define HTTPS_ASYNC(fmt, args...) \
    do{ \
        fprintf(stderr, "[%d] DEBUG[%s %d] "fmt, getpid(), __func__, __LINE__, ##args); \
    }while(0)
#else
#define HTTPS_ASYNC(...)
#endif

#ifdef HAVE_TLS
#define UBUS_PATH      	"/var/run/ubus.sock"

#define UH_LOGIN_PAGE				"/webpages/login.html"
#define UH_HTTPS_403PAGE			"/webpages/https_blocking.%shtml"
#define UH_WEBMOBILE_LOGIN_PAGE 	"/webmobile/login.html"
#define UH_WEBMOBILE_HTTPS_403PAGE	"/webmobile/https_error.html"

#define UH_SSL_CIPHERS_SERVER_PREFERENCE
#define UH_SSL_DEFAULT_CIPHERS     	"ALL:!aNULL:!MD5"
#define UH_SSL_DEFAULT_ECDH_CURVE  	"prime256v1"
#endif

#ifdef SUPPORT_MESH_WEB_DEBUG
#define MESH_WEB_DEBUG_ENABLE_URL		"/mesh_debug_enable"
#define MESH_WEB_DEBUG_DISABLE_URL		"/mesh_debug_disable"
#endif

#ifdef SUPPORT_AGILE_CONFIG
#define AGILE_CONFIG_RESET_URL		"/agileconfigreset"
#define AGILE_CONFIG_RESET_FILE		"/tmp/agile_config_reset"
#endif

#ifdef HAVE_TLS
#define UH_MAX_REQUESTS		(8)
#else
#define UH_MAX_REQUESTS		(3)
#endif

#define UH_MAX_PATH_LEN		(128)

#define UH_LIMIT_MSGHEAD	4096
#define UH_LIMIT_HEADERS	64
#define UH_LIMIT_CLIENTS	64

#ifdef CONFIG_MEM_PROTECT
#define LONG_TERM_PROCESS_TIMEOUT 900
#endif

extern time_t m_modified_time;

struct listener;
struct client;
struct interpreter;
struct http_request;
struct uh_ubus_state;

#ifdef HAVE_TLS
typedef enum _operation_mode{
	MODE_ROUTER		= 0,
	MODE_AP			= 1,
#ifdef SUPPORT_MESH_WEB_DEBUG
	MODE_RE			= 2,
#endif
	MODE_UNKNOWN
} OP_MODE;

struct admin_config {
	OP_MODE mode;
	struct in6_addr lan_ipv6;
	struct in6_addr prefix;
	unsigned int prefixlen;
    unsigned int lan_ip;
    unsigned int lan_mask;
    bool local_https_on;
    bool remote_on;
    unsigned int remote_http_port;
    unsigned int remote_https_port;
};
#endif

struct config {
	char docroot[PATH_MAX];
	char *realm;
	char *file;
	char *error_handler;
	int no_symlinks;
	int no_dirlists;
	int network_timeout;
	int rfc1918_filter;
	int tcp_keepalive;
	int max_requests;
#ifdef HAVE_CGI
	char *cgi_prefix;
#endif
#ifdef HAVE_LUA
	char *lua_prefix;
	char *lua_handler;
	lua_State *lua_state;
	lua_State * (*lua_init) (const struct config *conf);
	void (*lua_close) (lua_State *L);
	bool (*lua_request) (struct client *cl, lua_State *L);
#endif
#ifdef HAVE_UBUS
	char *ubus_prefix;
	char *ubus_socket;
	void *ubus_state;
	struct uh_ubus_state * (*ubus_init) (const struct config *conf);
	void (*ubus_close) (struct uh_ubus_state *state);
	bool (*ubus_request) (struct client *cl, struct uh_ubus_state *state);
#endif
#if defined(HAVE_CGI) || defined(HAVE_LUA) || defined(HAVE_UBUS)
	int script_timeout;
#endif
#ifdef HAVE_TLS
	char *cert;
	char *key;
	char *ciphers;
	SSL_CTX *tls;
	SSL_CTX * (*tls_init) (void);
	int (*tls_cert) (SSL_CTX *c, const char *file);
	int (*tls_key) (SSL_CTX *c, const char *file);
	int (*tls_ciphers) (SSL_CTX *c, const char *ciphers);
	void (*tls_free) (struct listener *l);
#if defined(TLS_ACCEPT_ASYNC)
	void (*tls_timeout) (struct client *c);
#endif
	int (*tls_accept) (struct client *c);
	void (*tls_close) (struct client *c);
	int (*tls_recv) (struct client *c, char *buf, int len);
	int (*tls_send) (struct client *c, const char *buf, int len);
	struct admin_config *admin_cfg;    
#endif
};

enum http_method {
	UH_HTTP_MSG_GET,
	UH_HTTP_MSG_POST,
	UH_HTTP_MSG_HEAD,
	UH_HTTP_MSG_PUT,
};

extern const char *http_methods[];

enum http_version {
	UH_HTTP_VER_0_9,
	UH_HTTP_VER_1_0,
	UH_HTTP_VER_1_1,
};

extern const char *http_versions[];

struct http_request {
	enum http_method method;
	enum http_version version;
	int redirect_status;
	char *url;
	char *headers[UH_LIMIT_HEADERS];
	struct auth_realm *realm;
};

struct http_response {
	int statuscode;
	char *statusmsg;
	char *headers[UH_LIMIT_HEADERS];
};

struct listener {
	struct uloop_fd fd;
	int socket;
	int n_clients;
	struct sockaddr_in6 addr;
	struct config *conf;
#ifdef HAVE_TLS
	SSL_CTX *tls;
#endif
	struct listener *next;
};

struct client {
#ifdef HAVE_TLS
	SSL *tls;
#endif
	struct uloop_fd fd;
	struct uloop_fd rpipe;
	struct uloop_fd wpipe;
	struct uloop_process proc;
	struct uloop_timeout timeout;
	bool (*cb)(struct client *);
	void *priv;
	bool dispatched;
	struct {
		char buf[UH_LIMIT_MSGHEAD];
		char *ptr;
		int len;
	} httpbuf;
	struct listener *server;
	struct http_request request;
	struct http_response response;
	struct sockaddr_in6 servaddr;
	struct sockaddr_in6 peeraddr;
	struct client *next;
#ifdef HAVE_TLS        
	bool local_access;
	bool local_https_on;
#endif

#ifdef CONFIG_MEM_PROTECT
	bool long_term;
#endif

};

struct client_light {
#ifdef HAVE_TLS
	SSL *tls;
#endif
	struct uloop_fd fd;
};

struct auth_realm {
	char path[PATH_MAX];
	char user[32];
	char pass[128];
	struct auth_realm *next;
};

struct index_file {
	const char *name;
	struct index_file *next;
};

#ifdef HAVE_CGI
struct interpreter {
	char path[PATH_MAX];
	char extn[32];
	struct interpreter *next;
};
#endif

#endif
