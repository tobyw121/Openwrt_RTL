#ifndef __UDTDB_SESSION_H__
#define __UDTDB_SESSION_H__
/*
 * file		udtdb_session.h
 * brief	export functions and variables	
 * details	
 *
 * author	WangLian
 * version	1.0.0
 * date		29Jun17
 *
 * history 	\arg 1.0.0  29Jun17, wanglian, create the file	
 */

#include "udtdb_util.h"

/**************************************************************************************************/
/*                                           DEFINES                                              */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           TYPES                                                */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           VARIABLES                                            */
/**************************************************************************************************/
extern int sessinitdone; /* Is set to 0 somewhere */
extern int exitflag;

/**************************************************************************************************/
/*                                           FUNCTIONS                                            */
/**************************************************************************************************/

/* Server */
void svr_session(int sock, int childpipe, void* p2pses);
void svr_dropbear_cleanup(const char* format, va_list param);
void svr_dropbear_log(int priority, const char* format, va_list param);
void svr_dropbear_exit(int exitcode, const char* format, va_list param) ATTRIB_NORETURN;

#endif /* __UDTDB_SESSION_H__ */

