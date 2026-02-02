/*****************************************************************************
Copyright (c) 2001 - 2011, The Board of Trustees of the University of Illinois.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the
  above copyright notice, this list of conditions
  and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the University of Illinois
  nor the names of its contributors may be used to
  endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu, last updated 01/18/2011
*****************************************************************************/

#ifndef __UDT_C_H__
#define __UDT_C_H__


#ifndef WIN32
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
#else
   #ifdef __MINGW__
      #include <stdint.h>
      #include <ws2tcpip.h>
   #endif
   #include <windows.h>
#endif

//////////////////////////////////// error code ///////////////////////////////////
#define UDT_SUCCESS			0
#define UDT_ECONNSETUP      1000 
#define UDT_ENOSERVER       1001 
#define UDT_ECONNREJ        1002 
#define UDT_ESOCKFAIL       1003 
#define UDT_ESECFAIL        1004
#define UDT_ECONNFAIL       2000 
#define UDT_ECONNLOST       2001 
#define UDT_ENOCONN         2002 
#define UDT_ERESOURCE       3000 
#define UDT_ETHREAD         3001 
#define UDT_ENOBUF          3002 
#define UDT_EFILE           4000 
#define UDT_EINVRDOFF       4001 
#define UDT_ERDPERM         4002 
#define UDT_EINVWROFF       4003 
#define UDT_EWRPERM         4004
#define UDT_EINVOP          5000 
#define UDT_EBOUNDSOCK      5001 
#define UDT_ECONNSOCK       5002 
#define UDT_EINVPARAM       5003 
#define UDT_EINVSOCK        5004 
#define UDT_EUNBOUNDSOCK    5005 
#define UDT_ENOLISTEN       5006 
#define UDT_ERDVNOSERV      5007 
#define UDT_ERDVUNBOUND     5008 
#define UDT_ESTREAMILL      5009 
#define UDT_EDGRAMILL       5010 
#define UDT_EDUPLISTEN      5011 
#define UDT_ELARGEMSG       5012 
#define UDT_EINVPOLLID      5013 
#define UDT_EASYNCFAIL      6000 
#define UDT_EASYNCSND       6001 
#define UDT_EASYNCRCV       6002 
#define UDT_ETIMEOUT        6003 
#define UDT_EPEERERR        7000 
#define UDT_EUNKNOWN        -1 

#define UDT_INVALID_SOCK	-1
#define UDT_ERROR			-1

////////////////////////////////////////////////////////////////////////////////

//if compiling on VC6.0 or pre-WindowsXP systems
//use -DLEGACY_WIN32

//if compiling with MinGW, it only works on XP or above
//use -D_WIN32_WINNT=0x0501


#ifdef WIN32
   #ifndef __MINGW__
      // Explicitly define 32-bit and 64-bit numbers
      typedef __int32 int32_t;
      typedef __int64 int64_t;
      typedef unsigned __int32 uint32_t;
      #ifndef LEGACY_WIN32
         typedef unsigned __int64 uint64_t;
      #else
         // VC 6.0 does not support unsigned __int64: may cause potential problems.
         typedef __int64 uint64_t;
      #endif

      #ifdef UDT_EXPORTS
         #define UDT_API __declspec(dllexport)
      #else
         #define UDT_API
      #endif
   #else
      #define UDT_API
   #endif
#else
   #define UDT_API __attribute__ ((visibility("default")))
#endif

#define NO_BUSY_WAITING

#ifdef WIN32
   #ifndef __MINGW__
      typedef SOCKET SYSSOCKET;
   #else
      typedef int SYSSOCKET;
   #endif
#else
   typedef int SYSSOCKET;
#endif

typedef SYSSOCKET UDPSOCKET;
typedef int UDTSOCKET;

////////////////////////////////////////////////////////////////////////////////

enum EPOLLOpt
{
   // this values are defined same as linux epoll.h
   // so that if system values are used by mistake, they should have the same effect
   UDT_EPOLL_IN = 0x1,
   UDT_EPOLL_OUT = 0x4,
   UDT_EPOLL_ERR = 0x8
};

enum UDTSTATUS {INIT = 1, OPENED, LISTENING, CONNECTING, CONNECTED, BROKEN, CLOSING, CLOSED, NONEXIST};

////////////////////////////////////////////////////////////////////////////////

enum UDTOpt
{
   UDT_MSS,             // the Maximum Transfer Unit
   UDT_SNDSYN,          // if sending is blocking
   UDT_RCVSYN,          // if receiving is blocking
   UDT_CC,              // custom congestion control algorithm
   UDT_FC,		// Flight flag size (window size)
   UDT_SNDBUF,          // maximum buffer in sending queue
   UDT_RCVBUF,          // UDT receiving buffer size
   UDT_LINGER,          // waiting for unsent data when closing
   UDP_SNDBUF,          // UDP sending buffer size
   UDP_RCVBUF,          // UDP receiving buffer size
   UDT_MAXMSG,          // maximum datagram message size
   UDT_MSGTTL,          // time-to-live of a datagram message
   UDT_RENDEZVOUS,      // rendezvous connection mode
   UDT_SNDTIMEO,        // send() timeout
   UDT_RCVTIMEO,        // recv() timeout
   UDT_REUSEADDR,	// reuse an existing port or create a new one
   UDT_MAXBW,		// maximum bandwidth (bytes per second) that the connection can use
   UDT_STATE,		// current socket state, see UDTSTATUS, read only
   UDT_EVENT,		// current avalable events associated with the socket
   UDT_SNDDATA,		// size of data in the sending buffer
   UDT_RCVDATA		// size of data available for recv
};

////////////////////////////////////////////////////////////////////////////////

struct CPerfMon
{
   // global measurements
   int64_t msTimeStamp;                 // time since the UDT entity is started, in milliseconds
   int64_t pktSentTotal;                // total number of sent data packets, including retransmissions
   int64_t pktRecvTotal;                // total number of received packets
   int pktSndLossTotal;                 // total number of lost packets (sender side)
   int pktRcvLossTotal;                 // total number of lost packets (receiver side)
   int pktRetransTotal;                 // total number of retransmitted packets
   int pktSentACKTotal;                 // total number of sent ACK packets
   int pktRecvACKTotal;                 // total number of received ACK packets
   int pktSentNAKTotal;                 // total number of sent NAK packets
   int pktRecvNAKTotal;                 // total number of received NAK packets
   int64_t usSndDurationTotal;		// total time duration when UDT is sending data (idle time exclusive)

   // local measurements
   int64_t pktSent;                     // number of sent data packets, including retransmissions
   int64_t pktRecv;                     // number of received packets
   int pktSndLoss;                      // number of lost packets (sender side)
   int pktRcvLoss;                      // number of lost packets (receiver side)
   int pktRetrans;                      // number of retransmitted packets
   int pktSentACK;                      // number of sent ACK packets
   int pktRecvACK;                      // number of received ACK packets
   int pktSentNAK;                      // number of sent NAK packets
   int pktRecvNAK;                      // number of received NAK packets
   double mbpsSendRate;                 // sending rate in Mb/s
   double mbpsRecvRate;                 // receiving rate in Mb/s
   int64_t usSndDuration;		// busy sending time (i.e., idle time exclusive)

   // instant measurements
   double usPktSndPeriod;               // packet sending period, in microseconds
   int pktFlowWindow;                   // flow window size, in number of packets
   int pktCongestionWindow;             // congestion window size, in number of packets
   int pktFlightSize;                   // number of packets on flight
   double msRTT;                        // RTT, in milliseconds
   double mbpsBandwidth;                // estimated bandwidth, in Mb/s
   int byteAvailSndBuf;                 // available UDT sender buffer size
   int byteAvailRcvBuf;                 // available UDT receiver buffer size
};

/////////////////////////////////////////////////////////////////////////////////////////////

typedef enum UDTOpt SOCKOPT;
typedef struct CPerfMon TRACEINFO;
typedef enum EPOLLOpt EPOLL_OPT;
typedef enum UDTSTATUS UDT_STATUS;

#ifdef __cplusplus
extern "C"
{
#endif

UDT_API int udt_startup();
UDT_API int udt_cleanup();
UDT_API UDTSOCKET udt_socket(int af, int type, int protocol);
UDT_API int udt_bind(UDTSOCKET u, const struct sockaddr* name, int namelen);
UDT_API int udt_bind2(UDTSOCKET u, UDPSOCKET udpsock);
UDT_API int udt_listen(UDTSOCKET u, int backlog);
UDT_API UDTSOCKET udt_accept(UDTSOCKET u, struct sockaddr* addr, int* addrlen);
UDT_API int udt_connect(UDTSOCKET u, const struct sockaddr* name, int namelen);
UDT_API int udt_close(UDTSOCKET u);
UDT_API int udt_getpeername(UDTSOCKET u, struct sockaddr* name, int* namelen);
UDT_API int udt_getsockname(UDTSOCKET u, struct sockaddr* name, int* namelen);
UDT_API int udt_getsockopt(UDTSOCKET u, int level, SOCKOPT optname, void* optval, int* optlen);
UDT_API int udt_setsockopt(UDTSOCKET u, int level, SOCKOPT optname, const void* optval, int optlen);
UDT_API int udt_send(UDTSOCKET u, const char* buf, int len, int flags);
UDT_API int udt_recv(UDTSOCKET u, char* buf, int len, int flags);
UDT_API int udt_sendmsg(UDTSOCKET u, const char* buf, int len, int ttl, int inorder);
UDT_API int udt_recvmsg(UDTSOCKET u, char* buf, int len);

UDT_API int64_t udt_sendfile2(UDTSOCKET u, const char* path, int64_t* offset, int64_t size, int block);
UDT_API int64_t udt_recvfile2(UDTSOCKET u, const char* path, int64_t* offset, int64_t size, int block);

UDT_API int udt_epoll_create();
UDT_API int udt_epoll_add_usock(int eid, UDTSOCKET u, const int* events);
UDT_API int udt_epoll_add_ssock(int eid, SYSSOCKET s, const int* events);
UDT_API int udt_epoll_remove_usock(int eid, UDTSOCKET u);
UDT_API int udt_epoll_remove_ssock(int eid, SYSSOCKET s);
UDT_API int udt_epoll_wait2(int eid, UDTSOCKET* readfds, int* rnum, UDTSOCKET* writefds,	
    int* wnum, int64_t msTimeOut, SYSSOCKET* lrfds, int* lrnum, SYSSOCKET* lwfds, int* lwnum );
UDT_API int udt_epoll_release(int eid);

UDT_API int udt_getlasterror_code();
UDT_API const char* udt_getlasterror_desc();
UDT_API int udt_perfmon(UDTSOCKET u, TRACEINFO* perf, int clear);
UDT_API UDT_STATUS udt_getsockstate(UDTSOCKET u);

#ifdef __cplusplus
}
#endif

#endif /* __UDT_C_H__ */
