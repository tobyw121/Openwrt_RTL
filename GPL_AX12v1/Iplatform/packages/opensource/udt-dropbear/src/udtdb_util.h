#ifndef __UDTDB_UTIL_H__
#define __UDTDB_UTIL_H__
/*
 * file		udtdb_util.h
 * brief	export functions and variables	
 * details	
 *
 * author	WangLian
 * version	1.0.0
 * date		29Jun17
 *
 * history 	\arg 1.0.0  29Jun17, wanglian, create the file	
 */

#include <sys/types.h>
#include <syslog.h>
#include <stdarg.h>
#include <time.h>

/**************************************************************************************************/
/*                                           DEFINES                                              */
/**************************************************************************************************/
#ifdef __GNUC__
#define ATTRIB_PRINTF(fmt,args) __attribute__((format(printf, fmt, args))) 
#define ATTRIB_NORETURN __attribute__((noreturn))
#define ATTRIB_SENTINEL __attribute__((sentinel))
#else
#define ATTRIB_PRINTF(fmt,args)
#define ATTRIB_NORETURN
#define ATTRIB_SENTINEL
#endif

void fail_assert(const char* expr, const char* file, int line) ATTRIB_NORETURN;

/* Dropbear assertion */
#ifndef DROPBEAR_ASSERT_ENABLED
#define DROPBEAR_ASSERT_ENABLED 1
#endif

#define dropbear_assert(X) do { if (DROPBEAR_ASSERT_ENABLED && !(X)) { fail_assert(#X, __FILE__, __LINE__); } } while (0)

/**************************************************************************************************/
/*                                           TYPES                                                */
/**************************************************************************************************/
typedef void (*dbexit_func)(int exitcode, const char* format, va_list param) ATTRIB_NORETURN;
typedef void (*dblog_func)(int priority, const char* format, va_list param);

typedef int (*p2pauth_cb)(const unsigned char *username, const unsigned char *password, void* p2pses);

/**************************************************************************************************/
/*                                           VARIABLES                                            */
/**************************************************************************************************/
extern dbexit_func _dropbear_exit;
extern dblog_func _dropbear_log;

extern p2pauth_cb _p2p_check_passwd;

/**************************************************************************************************/
/*                                           FUNCTIONS                                            */
/**************************************************************************************************/

void dropbear_exit(const char* format, ...) ATTRIB_PRINTF(1,2) ATTRIB_NORETURN;

void dropbear_close(const char* format, ...) ATTRIB_PRINTF(1,2) ;
void dropbear_log(int priority, const char* format, ...) ATTRIB_PRINTF(2,3) ;


void commonsetup_func(dbexit_func exit_func, dblog_func log_func, p2pauth_cb p2p_auth);

void get_socket_address(int fd, char **local_host, char **local_port,
        char **remote_host, char **remote_port, int host_lookup);
void getaddrstring(struct sockaddr_storage* addr, 
        char **ret_host, char **ret_port, int host_lookup);

void udt_get_socket_address(int fd, char **local_host, char **local_port,
                        char **remote_host, char **remote_port, int host_lookup);


int udt_dropbear_listen(const char* address, const char* port,
        int *usocks, int *ssocks, unsigned int sockcount, char **errstring, int *maxfd);
int udt_connect_remote(const char* remotehost, const char* remoteport,
        int nonblocking, char ** errstring);
void udt_close_fd(int fd);

void m_close(int fd);
void * m_malloc(size_t size);
void * m_strdup(const char * str);
void * m_realloc(void* ptr, size_t size);
#define m_free(X) do {free(X); (X) = NULL;} while (0); 
void m_burn(void* data, unsigned int len);
void setnonblocking(int fd);
void setblocking(int fd);
void disallow_core();
int m_str_to_uint(const char* str, unsigned int *val);

/* Returns a time in seconds that doesn't go backwards - does not correspond to
a real-world clock */
time_t monotonic_now();

#endif /* __UDTDB_UTIL_H__ */

