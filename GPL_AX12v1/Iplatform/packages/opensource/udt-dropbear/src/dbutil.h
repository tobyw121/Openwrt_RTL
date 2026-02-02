/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#ifndef _DBUTIL_H_

#define _DBUTIL_H_

#include "includes.h"
#include "buffer.h"
#include "udtdb_util.h"

#define UDT_REVENT (UDT_EPOLL_IN)
#define UDT_WEVENT (UDT_EPOLL_OUT)

#define UDT_SNDBUF_SIZE     (512 * 1024)
#define UDT_RCVBUF_SIZE     (512 * 1024)
#define UDT_MAX_SEG_SIZE    (1420)

#ifndef DISABLE_SYSLOG
void startsyslog();
#endif


#ifdef DEBUG_TRACE
void dropbear_trace(const char* format, ...) ATTRIB_PRINTF(1,2);
void dropbear_trace2(const char* format, ...) ATTRIB_PRINTF(1,2);
void printhex(const char * label, const unsigned char * buf, int len);
void printmpint(const char *label, mp_int *mp);
extern int debug_trace;
#endif

enum dropbear_prio {
    DROPBEAR_PRIO_DEFAULT = 10,
    DROPBEAR_PRIO_LOWDELAY = 11,
    DROPBEAR_PRIO_BULK = 12,
};

char * stripcontrol(const char * text);
void set_sock_nodelay(int sock);
void set_sock_priority(int sock, enum dropbear_prio prio);
int dropbear_listen(const char* address, const char* port,
        int *socks, unsigned int sockcount, char **errstring, int *maxfd);
int spawn_command(void(*exec_fn)(void *user_data), void *exec_data,
        int *writefd, int *readfd, int *errfd, pid_t *pid);
void run_shell_command(const char* cmd, unsigned int maxfd, char* usershell);
#ifdef ENABLE_CONNECT_UNIX
int connect_unix(const char* addr);
#endif
int connect_remote(const char* remotehost, const char* remoteport,
        int nonblocking, char ** errstring);


int buf_readfile(buffer* buf, const char* filename);
int buf_getline(buffer * line, FILE * authfile);


/* Used to force mp_ints to be initialised */
#define DEF_MP_INT(X) mp_int X = {0, 0, 0, NULL}


/* Returns 0 if a and b have the same contents */
int constant_time_memcmp(const void* a, const void *b, size_t n);

char * expand_tilde(const char *inpath);

#endif /* _DBUTIL_H_ */
