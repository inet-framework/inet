#ifndef __GENERIC_ENV_H__
#define __GENERIC_ENV_H__

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

/*
 * Instead of providing specific hacks for each OS, we choose the solution of 
 * insulating our code completely from the underlying OS specifics. 
 * So we undefine everything that might get into our way, then define it
 * in the way we like.
 */
#undef __inline__
#undef CMSG_FIRSTHDR
#undef CMSG_NXTHDR
#undef CMSG_ALIGN
#undef CMSG_DATA
#undef _CMSG_DATA_ALIGN
#undef _CMSG_HDR_ALIGN
#undef CMSG_SPACE
#undef CMSG_LEN
#undef IP_HDRINCL
#undef IP_PKTINFO
#undef IN_CLASSD
#undef IN_MULTICAST
#undef MSG_TRUNC
#undef MAXPATHLEN
#undef EWOULDBLOCK
#undef EAFNOSUPPORT
#undef EINPROGRESS
#undef IPVERSION
#undef caddr_t
#undef mode_t
#undef uid_t
#undef gid_t
#undef pid_t
#undef time_t
#undef socklen_t
#undef ssize_t
#undef u_char
#undef u_short
#undef u_int
#undef u_long
#undef useconds_t
#undef suseconds_t
#undef ino64_t
#undef nlink_t
#undef off_t
#undef off64_t
#undef blksize_t
#undef blkcnt_t
#undef blkcnt64_t
#undef clock_t
#undef fd_mask
#undef s8
#undef u8
#undef s16
#undef u16
#undef s32
#undef u32
#undef s64
#undef u64
#undef in_addr
#undef s_addr
#undef s_host
#undef s_net
#undef s_imp
#undef s_impno
#undef s_lh
#undef sockaddr
#undef sockaddr_in
#undef SOCKET
#undef FD_SETSIZE
#undef fd_set
#undef FD_CLR
#undef FD_SET
#undef FD_ZERO
#undef FD_ISSET
#undef TCP_NODELAY
#undef TCP_BSDURGENT
#undef AF_UNSPEC
#undef AF_UNIX
#undef AF_INET
#undef AF_INET6
#undef IPPROTO_IP
#undef IPPROTO_ICMP
#undef IPPROTO_IGMP
#undef IPPROTO_GGP
#undef IPPROTO_TCP
#undef IPPROTO_PUP
#undef IPPROTO_UDP
#undef IPPROTO_IDP
#undef IPPROTO_ND
#undef IPPROTO_RAW
#undef IPPROTO_MAX
#undef SOCK_STREAM
#undef SOCK_DGRAM
#undef SOCK_RAW
#undef SOCK_RDM
#undef SOCK_SEQPACKET
#undef INADDR_ANY
#undef INADDR_LOOPBACK
#undef INADDR_BROADCAST
#undef INADDR_NONE
#undef IN_CLASSA
#undef IN_CLASSA_NET
#undef IN_CLASSA_NSHIFT
#undef IN_CLASSA_HOST
#undef IN_CLASSA_MAX
#undef IN_CLASSB
#undef IN_CLASSB_NET
#undef IN_CLASSB_NSHIFT
#undef IN_CLASSB_HOST
#undef IN_CLASSB_MAX
#undef IN_CLASSC
#undef IN_CLASSC_NET
#undef IN_CLASSC_NSHIFT
#undef IN_CLASSC_HOST
#undef SOL_SOCKET
#undef SO_DEBUG
#undef SO_ACCEPTCONN
#undef SO_REUSEADDR
#undef SO_KEEPALIVE
#undef SO_DONTROUTE
#undef SO_BROADCAST
#undef SO_USELOOPBACK
#undef SO_LINGER
#undef SO_OOBINLINE
#undef SO_DONTLINGER
#undef SO_SNDBUF
#undef SO_RCVBUF
#undef SO_SNDLOWAT
#undef SO_RCVLOWAT
#undef SO_SNDTIMEO
#undef SO_RCVTIMEO
#undef SO_ERROR
#undef SO_TYPE
#undef SO_CONNDATA
#undef SO_CONNOPT
#undef SO_DISCDATA
#undef SO_DISCOPT
#undef SO_CONNDATALEN
#undef SO_CONNOPTLEN
#undef SO_DISCDATALEN
#undef SO_DISCOPTLEN
#undef SO_OPENTYPE
#undef SO_SYNCHRONOUS_ALERT
#undef SO_SYNCHRONOUS_NONALERT
#undef SO_MAXDG
#undef SO_MAXPATHDG
#undef SO_UPDATE_ACCEPT_CONTEXT
#undef SO_CONNECT_TIME
#undef IP_OPTIONS
#undef IP_MULTICAST_IF
#undef IP_MULTICAST_TTL
#undef IP_MULTICAST_LOOP
#undef IP_ADD_MEMBERSHIP
#undef IP_DROP_MEMBERSHIP
#undef IP_TTL
#undef IP_TOS
#undef IP_DONTFRAGMENT
#undef IP_DEFAULT_MULTICAST_TTL
#undef IP_DEFAULT_MULTICAST_LOOP
#undef IP_MAX_MEMBERSHIPS
#undef ip_mreq
#undef servent
#undef MSG_OOB
#undef MSG_PEEK
#undef MSG_DONTROUTE
#undef MSG_MAXIOVLEN
#undef MSG_PARTIAL
#undef timeval
#undef timespec
#undef group
#undef passwd
#undef iovec
#undef sig_atomic_t
#undef sigset_t
#undef sigval_t
#undef siginfo_t
#undef struct_sigaction
#undef SA_SIGINFO
#undef sa_sigaction
#undef sa_handler
#undef sa_mask
#undef sa_flags
#undef sa_restorer
#undef utsname
#undef in_addr_t
#undef sa_family_t
#undef sockaddr_nl
#undef ip
#undef sockaddr_un
#undef in_port_t
#undef in6_addr
#undef nlmsghdr
#undef rtgenmsg
#undef ifinfomsg
#undef nlmsgerr
#undef rtattr
#undef rtmsg
#undef ifaddrmsg
#undef rtnexthop
#undef ifmap
#undef ifreq
#undef in_pktinfo
#undef RT_TABLE_MAX
#undef RTM_BASE
#undef RTM_NEWLINK
#undef RTM_DELLINK
#undef RTM_GETLINK
#undef RTM_SETLINK
#undef RTM_NEWADDR
#undef RTM_DELADDR
#undef RTM_GETADDR
#undef RTM_NEWROUTE
#undef RTM_DELROUTE
#undef RTM_GETROUTE
#undef RTM_NEWNEIGH
#undef RTM_DELNEIGH
#undef RTM_GETNEIGH
#undef RTM_NEWRULE
#undef RTM_DELRULE
#undef RTM_GETRULE
#undef RTM_NEWQDISC
#undef RTM_DELQDISC
#undef RTM_GETQDISC
#undef RTM_NEWTCLASS
#undef RTM_DELTCLASS
#undef RTM_GETTCLASS
#undef RTM_NEWTFILTER
#undef RTM_DELTFILTER
#undef RTM_GETTFILTER
#undef RTM_MAX
#undef RTA_MAX
#undef RTN_MAX
#undef IFLA_COST
#undef IFLA_PRIORITY
#undef IFLA_MASTER
#undef IFLA_WIRELESS
#undef IFLA_PROTINFO
#undef IFLA_MAX
#undef IFA_MAX
#undef IFA_F_SECONDARY
#undef IFA_F_TEMPORARY
#undef IFA_F_DEPRECATED
#undef IFA_F_TENTATIVE
#undef IFA_F_PERMANENT
#undef RTM_F_NOTIFY
#undef RTM_F_CLONED
#undef RTM_F_EQUALIZE
#undef RTM_F_PREFIX
#undef RTPROT_UNSPEC
#undef RTPROT_REDIRECT
#undef RTPROT_KERNEL
#undef RTPROT_BOOT
#undef RTPROT_STATIC
#undef RTPROT_GATED
#undef RTPROT_RA
#undef RTPROT_MRT
#undef RTPROT_ZEBRA
#undef RTPROT_BIRD
#undef RTPROT_DNROUTED
#undef RTMGRP_LINK
#undef RTMGRP_NOTIFY
#undef RTMGRP_NEIGH
#undef RTMGRP_TC
#undef RTMGRP_IPV4_IFADDR
#undef RTMGRP_IPV4_MROUTE
#undef RTMGRP_IPV4_ROUTE
#undef RTMGRP_IPV6_IFADDR
#undef RTMGRP_IPV6_MROUTE
#undef RTMGRP_IPV6_ROUTE
#undef RTMGRP_DECnet_IFADDR
#undef RTMGRP_DECnet_ROUTE
#undef RTA_ALIGNTO
#undef RTA_ALIGN
#undef RTA_OK
#undef RTA_NEXT
#undef RTA_LENGTH
#undef RTA_SPACE
#undef RTA_DATA
#undef RTA_PAYLOAD
#undef IFLA_RTA
#undef IFLA_PAYLOAD
#undef IFA_RTA
#undef IFA_PAYLOAD
#undef RTM_RTA
#undef RTM_PAYLOAD
#undef RTNH_ALIGNTO
#undef RTNH_ALIGN
#undef RTNH_NEXT
#undef NETLINK_ROUTE
#undef NLM_F_REQUEST
#undef NLM_F_MULTI
#undef NLM_F_ROOT
#undef NLM_F_MATCH
#undef NLM_F_ACK
#undef NLM_F_ECHO
#undef NLM_F_REPLACE
#undef NLM_F_EXCL
#undef NLM_F_CREATE
#undef NLM_F_APPEND
#undef NLMSG_ALIGNTO
#undef NLMSG_ALIGN
#undef NLMSG_LENGTH
#undef NLMSG_SPACE
#undef NLMSG_DATA
#undef NLMSG_NEXT
#undef NLMSG_OK
#undef NLMSG_PAYLOAD
#undef NLMSG_NOOP
#undef NLMSG_ERROR
#undef NLMSG_DONE
#undef NLMSG_OVERRUN
#undef ARPHRD_ETHER
#undef ARPHRD_PPP
#undef ARPHRD_LOOPBACK
#undef AF_NETLINK
#undef AF_PACKET
#undef PF_NETLINK
#undef PF_PACKET
#undef IAC
#undef DONT
#undef DO
#undef WILL
#undef SB
#undef SE
#undef TELOPT_ECHO
#undef TELOPT_SGA
#undef TELOPT_NAWS
#undef TELOPT_LINEMODE
#undef LOG_EMERG
#undef LOG_ALERT
#undef LOG_CRIT
#undef LOG_ERR
#undef LOG_WARNING
#undef LOG_NOTICE
#undef LOG_INFO
#undef LOG_DEBUG
#undef LOG_KERN
#undef LOG_USER
#undef LOG_MAIL
#undef LOG_DAEMON
#undef LOG_AUTH
#undef LOG_SYSLOG
#undef LOG_LPR
#undef LOG_NEWS
#undef LOG_UUCP
#undef LOG_CRON
#undef LOG_AUTHPRIV
#undef LOG_FTP
#undef LOG_LOCAL0
#undef LOG_LOCAL1
#undef LOG_LOCAL2
#undef LOG_LOCAL3
#undef LOG_LOCAL4
#undef LOG_LOCAL5
#undef LOG_LOCAL6
#undef LOG_LOCAL7
#undef LOG_PID
#undef LOG_CONS
#undef LOG_ODELAY
#undef LOG_NDELAY
#undef LOG_NOWAIT
#undef LOG_PERROR
#undef F_GETFL
#undef F_SETFL
#undef O_NONBLOCK
#undef SIGHUP
#undef SIGINT
#undef SIGQUIT
#undef SIGILL
#undef SIGBUS
#undef SIGFPE
#undef SIGUSR1
#undef SIGSEGV
#undef SIGUSR2
#undef SIGPIPE
#undef SIGALRM
#undef SIGTERM
#undef SIG_DFL
#undef SIG_IGN
#undef SIG_SGE
#undef SIG_ACK
#undef SIOCADDRT
#undef SIOCDELRT
#undef SIOCRTMSG
#undef SIOCGIFFLAGS
#undef SIOCSIFFLAGS
#undef IF_NAMESIZE
#undef IFHWADDRLEN
#undef IFNAMSIZ
#undef ifr_name
#undef ifr_hwaddr
#undef ifr_addr
#undef ifr_dstaddr
#undef ifr_broadaddr
#undef ifr_netmask
#undef ifr_flags
#undef ifr_metric
#undef ifr_mtu
#undef ifr_map
#undef ifr_slave
#undef ifr_data
#undef ifr_ifindex
#undef ifr_bandwidth
#undef ifr_qlen
#undef ifr_newname
#undef _IOT_ifreq
#undef _IOT_ifreq_short
#undef _IOT_ifreq_int
#undef htonl
#undef htons
#undef ntohl
#undef ntohs
#undef si_pid
#undef si_uid
#undef si_status
#undef si_value
#undef si_band
#undef si_addr

#define __inline__  __inline

#ifdef __cplusplus
extern "C" {
#endif

int snprintf (char *s, size_t maxlen, const char *format, ...);
int vsnprintf(char *s, size_t maxlen, const char *format, va_list arg);
int strncasecmp(const char *s1, const char *s2, size_t n);

#ifdef __cplusplus
};
#endif

/*
 * htonl() etc maybe used Netlink.cc, RawSocket.cc, etc as well, so we have to define them here
 */
#define htonl  oppsim_htonl
#define htons  oppsim_htons
#define inet_ntoa  oppsim_inet_ntoa
#define ntohl  oppsim_ntohl
#define ntohs  oppsim_ntohs
#define inet_addr  oppsim_inet_addr
#define inet_aton  oppsim_inet_aton
#define inet_pton  oppsim_inet_pton
#define inet_ntop  oppsim_inet_ntop


// disable VC8.0 warnings on strdup() etc usage
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif


// in WS2tcpip.h, IP_HDRINCL==2 while in Winsock.h that's IP_MULTICAST_IF -- define as 25 to avoid confusion
#define IP_HDRINCL  25

#define IP_PKTINFO  19

#define IN_CLASSD(a)            ((((in_addr_t)(a)) & 0xf0000000) == 0xe0000000)

#define IN_MULTICAST(a)         IN_CLASSD(a)

#define MSG_TRUNC   0x0100

#define MAXPATHLEN  4096

#define EWOULDBLOCK     EAGAIN
#define EAFNOSUPPORT    97
#define EINPROGRESS     115

#define IPVERSION   4

// basic types (we use #defines to mask accidentally included native definitions)

#define mode_t      unsigned int
#define uid_t       unsigned int
#define gid_t       unsigned int
#define pid_t       int
#define time_t      long int
#define socklen_t   unsigned int
#define ssize_t     long int
#define u_char      unsigned char
#define u_short     unsigned short int
#define u_int       unsigned int
#define u_long      unsigned long int
#define useconds_t  unsigned int
#define suseconds_t long int
#define ino64_t     unsigned long int
#define nlink_t     unsigned long int
#define off_t       long int
#define off64_t     long int
#define blksize_t   long int
#define blkcnt_t    long int
#define blkcnt64_t  long int
#define clock_t     long int
#define fd_mask     long int

#if defined(_MSC_VER)
#  define int8_t      __int8
#  define int16_t     __int16
#  define int32_t     __int32
#  define int64_t     __int64
#  define uint8_t     unsigned __int8
#  define uint16_t    unsigned __int16
#  define uint32_t    unsigned __int32
#  define uint64_t    unsigned __int64
#  define u_int8_t    unsigned __int8
#  define u_int16_t   unsigned __int16
#  define u_int32_t   unsigned __int32
#  define u_int64_t   unsigned __int64
#elif defined(int8_t)
  /* 
   * assume u_int8_t, int8_t etc are all defined
   */
#else /* a guess known to work on almost all (or all?) platforms */
#  define int8_t      signed char
#  define int16_t     short
#  define int32_t     int
#  define int64_t     long long
#  define uint8_t     unsigned char
#  define uint16_t    unsigned short
#  define uint32_t    unsigned int
#  define uint64_t    unsigned long long
#  define u_int8_t    unsigned char
#  define u_int16_t   unsigned short
#  define u_int32_t   unsigned int
#  define u_int64_t   unsigned long long
#endif

#define s8          int8_t
#define u8          uint8_t
#define s16         int16_t
#define u16         uint16_t
#define s32         int32_t
#define u32         uint32_t
#define s64         int32_t
#define u64         uint32_t

#define caddr_t  oppsimt_caddr_t
typedef char *oppsimt_caddr_t;

// in_addr

#define in_addr  oppsimt_in_addr
struct oppsimt_in_addr
{
        union {
                struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { u_short s_w1,s_w2; } S_un_w;
                u_long S_addr;
        } S_un;
#define s_addr  S_un.S_addr
#define s_host  S_un.S_un_b.s_b2
#define s_net   S_un.S_un_b.s_b1
#define s_imp   S_un.S_un_w.s_w2
#define s_impno S_un.S_un_b.s_b4
#define s_lh    S_un.S_un_b.s_b3
};

#define sockaddr  oppsimt_sockaddr
struct oppsimt_sockaddr
{
        u_short sa_family;
        char    sa_data[14];
};

#define sockaddr_in  oppsimt_sockaddr_in
struct oppsimt_sockaddr_in
{
        short   sin_family;
        u_short sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
};


#define SOCKET  u_int64_t
#define FD_SETSIZE      64

#define fd_set struct oppsimt_fd_set
struct oppsimt_fd_set
{
        u_int   fd_count;
        SOCKET  fd_array[FD_SETSIZE];
};


#define FD_CLR(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set *)(set))->fd_count ; __i++) { \
        if (((fd_set *)(set))->fd_array[__i] == fd) { \
            while (__i < ((fd_set *)(set))->fd_count-1) { \
                ((fd_set *)(set))->fd_array[__i] = \
                    ((fd_set *)(set))->fd_array[__i+1]; \
                __i++; \
            } \
            ((fd_set *)(set))->fd_count--; \
            break; \
        } \
    } \
} while(0)

#define FD_SET(fd, set) do { \
    if (((fd_set *)(set))->fd_count < FD_SETSIZE) \
        ((fd_set *)(set))->fd_array[((fd_set *)(set))->fd_count++]=(fd);\
} while(0)

#define FD_ZERO(set) (((fd_set *)(set))->fd_count=0)

#define FD_ISSET(fd, set) oppsim_FD_IS_SET((SOCKET)(fd), (fd_set *)(set))

static int oppsim_FD_IS_SET(SOCKET fd, fd_set *set)
{
    u_int i;
    for (i = 0; i < set->fd_count; i++)
        if (set->fd_array[i] == fd)
            return 1;
    return 0;
}

/*
 * TCP options.
 */
#define TCP_NODELAY     0x0001
#define TCP_BSDURGENT   0x7000

/*
 * Address families.
 */
#define AF_UNSPEC       0
#define AF_UNIX         1
#define AF_INET         2

/*
 * Protocols
 */
#define IPPROTO_IP              0               /* dummy for IP */
#define IPPROTO_ICMP            1               /* control message protocol */
#define IPPROTO_IGMP            2               /* group management protocol */
#define IPPROTO_GGP             3               /* gateway^2 (deprecated) */
#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_PUP             12              /* pup */
#define IPPROTO_UDP             17              /* user datagram protocol */
#define IPPROTO_IDP             22              /* xns idp */
#define IPPROTO_ND              77              /* UNOFFICIAL net disk proto */

#define IPPROTO_RAW             255             /* raw IP packet */
#define IPPROTO_MAX             256

/*
 * Types
 */
#define SOCK_STREAM     1               /* stream socket */
#define SOCK_DGRAM      2               /* datagram socket */
#define SOCK_RAW        3               /* raw-protocol interface */
#define SOCK_RDM        4               /* reliably-delivered message */
#define SOCK_SEQPACKET  5               /* sequenced packet stream */

#define INADDR_ANY              (u_long)0x00000000
#define INADDR_LOOPBACK         0x7f000001
#define INADDR_BROADCAST        (u_long)0xffffffff
#define INADDR_NONE             0xffffffff

#define IN_CLASSA(i)            (((long)(i) & 0x80000000) == 0)
#define IN_CLASSA_NET           0xff000000
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          0x00ffffff
#define IN_CLASSA_MAX           128

#define IN_CLASSB(i)            (((long)(i) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          0x0000ffff
#define IN_CLASSB_MAX           65536

#define IN_CLASSC(i)            (((long)(i) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET           0xffffff00
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          0x000000ff

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SOL_SOCKET      0xffff          /* options for socket level */

/*
 * Option flags per-socket.
 */
#define SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SO_LINGER       0x0080          /* linger on close if data present */
#define SO_OOBINLINE    0x0100          /* leave received OOB data in line */

#define SO_DONTLINGER   (u_int)(~SO_LINGER)

/*
 * Additional options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */

/*
 * Options for connect and disconnect data and options.  Used only by
 * non-TCP/IP transports such as DECNet, OSI TP4, etc.
 */
#define SO_CONNDATA     0x7000
#define SO_CONNOPT      0x7001
#define SO_DISCDATA     0x7002
#define SO_DISCOPT      0x7003
#define SO_CONNDATALEN  0x7004
#define SO_CONNOPTLEN   0x7005
#define SO_DISCDATALEN  0x7006
#define SO_DISCOPTLEN   0x7007

/*
 * Option for opening sockets for synchronous access.
 */
#define SO_OPENTYPE     0x7008

#define SO_SYNCHRONOUS_ALERT    0x10
#define SO_SYNCHRONOUS_NONALERT 0x20

/*
 * Other NT-specific options.
 */
#define SO_MAXDG        0x7009
#define SO_MAXPATHDG    0x700A
#define SO_UPDATE_ACCEPT_CONTEXT 0x700B
#define SO_CONNECT_TIME 0x700C


/*
 * Options for use with [gs]etsockopt at the IP level.
 */
#define IP_OPTIONS          1           /* set/get IP per-packet options    */
#define IP_MULTICAST_IF     2           /* set/get IP multicast interface   */
#define IP_MULTICAST_TTL    3           /* set/get IP multicast timetolive  */
#define IP_MULTICAST_LOOP   4           /* set/get IP multicast loopback    */
#define IP_ADD_MEMBERSHIP   5           /* add  an IP group membership      */
#define IP_DROP_MEMBERSHIP  6           /* drop an IP group membership      */
#define IP_TTL              7           /* set/get IP Time To Live          */
#define IP_TOS              8           /* set/get IP Type Of Service       */
#define IP_DONTFRAGMENT     9           /* set/get IP Don't Fragment flag   */


#define IP_DEFAULT_MULTICAST_TTL   1    /* normally limit m'casts to 1 hop  */
#define IP_DEFAULT_MULTICAST_LOOP  1    /* normally hear sends if a member  */
#define IP_MAX_MEMBERSHIPS         20   /* per socket; must fit in one mbuf */

/*
 * Argument structure for IP_ADD_MEMBERSHIP and IP_DROP_MEMBERSHIP.
 */
#define ip_mreq  oppsimt_ip_mreq
struct oppsimt_ip_mreq
{
        struct in_addr  imr_multiaddr;  /* IP multicast address of group */
        struct in_addr  imr_interface;  /* local IP address of interface */
};

#define servent  oppsimt_servent
struct oppsimt_servent
{
        char  * s_name;           /* official service name */
        char ** s_aliases;        /* alias list */
        short   s_port;           /* port # */
        char  * s_proto;          /* protocol to use */
};

#define MSG_OOB         0x1             /* process out-of-band data */
#define MSG_PEEK        0x2             /* peek at incoming message */
#define MSG_DONTROUTE   0x4             /* send without using routing tables */

#define MSG_MAXIOVLEN   16

#define MSG_PARTIAL     0x8000          /* partial send or recv for message xport */

// time

#define timeval  oppsimt_timeval
struct oppsimt_timeval
{
        long    tv_sec;
        long    tv_usec;
};

#define timespec  oppsimt_timespec
struct oppsimt_timespec
{
    time_t tv_sec;
    long int tv_nsec;
};

// passwd

#define group  oppsimt_group
struct oppsimt_group
{
    char *gr_name;
    char *gr_passwd;
    gid_t gr_gid;
    char **gr_mem;
};

#define passwd  oppsimt_passwd
struct oppsimt_passwd
{
    char *pw_name;
    char *pw_passwd;
    uid_t pw_uid;
    gid_t pw_gid;
    char *pw_gecos;
    char *pw_dir;
    char *pw_shell;
};

// writev etc

#define iovec  oppsimt_iovec
struct oppsimt_iovec
{
    void *iov_base;
    size_t iov_len;
};

// signal

#define sig_atomic_t        int


typedef void (*sighandler_t) (int);

#define sigset_t  struct oppsimt_sigset
struct oppsimt_sigset
{
    unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
};


#define sigval_t  union oppsimt_sigval
union oppsimt_sigval
{
    int sival_int;
    void *sival_ptr;
};

#define siginfo_t  struct oppsimt_siginfo
struct oppsimt_siginfo {
        int     si_signo;
        int     si_errno;
        int     si_code;
        pid_t   si_pid;
        uid_t   si_uid;
        int     si_status;
        void    *si_addr;
        sigval_t si_value;
        long    si_band;
        unsigned long   pad[7];
};


// FIXME add this to globalwhitelist.lst as well?
#define struct_sigaction  struct oppsimt_sigaction
struct oppsimt_sigaction
{
    sighandler_t sa_handler;

    sigset_t sa_mask;

    int sa_flags;

    void (*sa_restorer) (void);
};

// uname

#define utsname  oppsimt_utsname
struct oppsimt_utsname
{
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
    char __domainname[65];
};

// networking stuff

#define in_addr_t       uint32_t

#define sa_family_t     unsigned short int

#define sockaddr_nl  oppsimt_sockaddr_nl
struct oppsimt_sockaddr_nl
{
        sa_family_t nl_family;
        unsigned short nl_pad;
        u32 nl_pid;
        u32 nl_groups;
};

#define ip  oppsimt_ip
struct oppsimt_ip
{
    unsigned int ip_hl:4;
    unsigned int ip_v:4;

    u_int8_t ip_tos;
    u_short ip_len;
    u_short ip_id;
    u_short ip_off;

    u_int8_t ip_ttl;
    u_int8_t ip_p;
    u_short ip_sum;
    struct in_addr ip_src, ip_dst;
};

#define sockaddr_un  oppsimt_sockaddr_un
struct oppsimt_sockaddr_un
{
    sa_family_t sun_family;
    char sun_path[108];
};

#define in_port_t       uint16_t

#define in6_addr  oppsimt_in6_addr
struct oppsimt_in6_addr
{
    union
    {
        uint8_t u6_addr8[16];
        uint16_t u6_addr16[8];
        uint32_t u6_addr32[4];
    } in6_u;
};

enum
{
    IFF_UP = 0x1,
    IFF_BROADCAST = 0x2,
    IFF_DEBUG = 0x4,
    IFF_LOOPBACK = 0x8,
    IFF_POINTOPOINT = 0x10,
    IFF_NOTRAILERS = 0x20,
    IFF_RUNNING = 0x40,
    IFF_NOARP = 0x80,
    IFF_PROMISC = 0x100,
    IFF_ALLMULTI = 0x200,
    IFF_MASTER = 0x400,
    IFF_SLAVE = 0x800,
    IFF_MULTICAST = 0x1000,
    IFF_PORTSEL = 0x2000,
    IFF_AUTOMEDIA = 0x4000
};

// netlink & messages

// FIXME (WinXP): if we map cmsghdr and msghdr as well, something gets messed up and we get ___cmsg_nxthdr as undefined symbol

//#define cmsghdr  oppsimt_cmsghdr
//struct oppsimt_cmsghdr
struct cmsghdr
{
    size_t cmsg_len;

    int cmsg_level;
    int cmsg_type;

    unsigned char __cmsg_data [];
};

//#define msghdr  oppsimt_msghdr
//struct oppsimt_msghdr
struct msghdr
{
    void *msg_name;
    socklen_t msg_namelen;

    struct iovec *msg_iov;
    size_t msg_iovlen;

    void *msg_control;
    size_t msg_controllen;

    int msg_flags;
};

#ifdef __cplusplus
extern "C" {
#endif

struct cmsghdr * __cmsg_nxthdr (struct msghdr *__mhdr, struct cmsghdr *__cmsg);

#ifdef __cplusplus
};
#endif


#define CMSG_FIRSTHDR(mhdr) \
    ((size_t) (mhdr)->msg_controllen >= sizeof (struct cmsghdr) \
     ? (struct cmsghdr *) (mhdr)->msg_control : (struct cmsghdr *) NULL)

#define CMSG_NXTHDR(mhdr, cmsg) __cmsg_nxthdr (mhdr, cmsg)

#define CMSG_ALIGN(len) (((len) + sizeof (size_t) - 1) \
                                 & (size_t) ~(sizeof (size_t) - 1))

#define CMSG_DATA(cmsg)             ((cmsg)->__cmsg_data)

#define _CMSG_DATA_ALIGN(n)           (((n) + 3) & ~3)

#define _CMSG_HDR_ALIGN(n)            (((n) + 3) & ~3)

#define CMSG_SPACE(l)       (_CMSG_DATA_ALIGN(sizeof(struct cmsghdr)) + \
                              _CMSG_HDR_ALIGN(l))

#define CMSG_LEN(l)         (_CMSG_DATA_ALIGN(sizeof(struct cmsghdr)) + (l))


enum rt_class_t
{
        RT_TABLE_UNSPEC=0,
        RT_TABLE_DEFAULT=253,
        RT_TABLE_MAIN=254,
        RT_TABLE_LOCAL=255
};

enum rtattr_type_t
{
        RTA_UNSPEC,
        RTA_DST,
        RTA_SRC,
        RTA_IIF,
        RTA_OIF,
        RTA_GATEWAY,
        RTA_PRIORITY,
        RTA_PREFSRC,
        RTA_METRICS,
        RTA_MULTIPATH,
        RTA_PROTOINFO,
        RTA_FLOW,
        RTA_CACHEINFO,
        RTA_SESSION,
};

#define nlmsghdr  oppsimt_nlmsghdr
struct oppsimt_nlmsghdr
{
        u32 nlmsg_len;
        u16 nlmsg_type;
        u16 nlmsg_flags;
        u32 nlmsg_seq;
        u32 nlmsg_pid;
};

#define rtgenmsg  oppsimt_rtgenmsg
struct oppsimt_rtgenmsg
{
    unsigned char rtgen_family;
};

#define ifinfomsg  oppsimt_ifinfomsg
struct oppsimt_ifinfomsg
{
    unsigned char ifi_family;
    unsigned char __ifi_pad;
    unsigned short ifi_type;
    int ifi_index;
    unsigned ifi_flags;
    unsigned ifi_change;
};

#define nlmsgerr  oppsimt_nlmsgerr
struct oppsimt_nlmsgerr
{
    int error;
    struct nlmsghdr msg;
};

#define rtattr  oppsimt_rtattr
struct oppsimt_rtattr
{
    unsigned short rta_len;
    unsigned short rta_type;
};

#define rtmsg  oppsimt_rtmsg
struct oppsimt_rtmsg
{
    unsigned char rtm_family;
    unsigned char rtm_dst_len;
    unsigned char rtm_src_len;
    unsigned char rtm_tos;

    unsigned char rtm_table;
    unsigned char rtm_protocol;
    unsigned char rtm_scope;
    unsigned char rtm_type;

    unsigned rtm_flags;
};

enum
{
        IFLA_UNSPEC,
        IFLA_ADDRESS,
        IFLA_BROADCAST,
        IFLA_IFNAME,
        IFLA_MTU,
        IFLA_LINK,
        IFLA_QDISC,
        IFLA_STATS,
        IFLA_COST,
        IFLA_PRIORITY,
        IFLA_MASTER,
        IFLA_WIRELESS,
        IFLA_PROTINFO,
};

enum
{
        IFA_UNSPEC,
        IFA_ADDRESS,
        IFA_LOCAL,
        IFA_LABEL,
        IFA_BROADCAST,
        IFA_ANYCAST,
        IFA_CACHEINFO
};

#define ifaddrmsg  oppsimt_ifaddrmsg
struct oppsimt_ifaddrmsg
{
    unsigned char ifa_family;
    unsigned char ifa_prefixlen;
    unsigned char ifa_flags;
    unsigned char ifa_scope;
    int ifa_index;
};

enum
{
    RTN_UNSPEC,
    RTN_UNICAST,
    RTN_LOCAL,
    RTN_BROADCAST,
    RTN_ANYCAST,
    RTN_MULTICAST,
    RTN_BLACKHOLE,
    RTN_UNREACHABLE,
    RTN_PROHIBIT,
    RTN_THROW,
    RTN_NAT,
    RTN_XRESOLVE,
};

enum rt_scope_t
{
    RT_SCOPE_UNIVERSE=0,
    RT_SCOPE_SITE=200,
    RT_SCOPE_LINK=253,
    RT_SCOPE_HOST=254,
    RT_SCOPE_NOWHERE=255
};

#define rtnexthop  oppsimt_rtnexthop
struct oppsimt_rtnexthop
{
    unsigned short rtnh_len;
    unsigned char rtnh_flags;
    unsigned char rtnh_hops;
    int rtnh_ifindex;
};

#define ifmap  oppsimt_ifmap
struct oppsimt_ifmap
{
    unsigned long int mem_start;
    unsigned long int mem_end;
    unsigned short int base_addr;
    unsigned char irq;
    unsigned char dma;
    unsigned char port;
};

#define ifreq  oppsimt_ifreq
struct oppsimt_ifreq
{
    union
    {
        char ifrn_name[16];
    } ifr_ifrn;

    union
    {
        struct sockaddr ifru_addr;
        struct sockaddr ifru_dstaddr;
        struct sockaddr ifru_broadaddr;
        struct sockaddr ifru_netmask;
        struct sockaddr ifru_hwaddr;
        short int ifru_flags;
        int ifru_ivalue;
        int ifru_mtu;
        struct ifmap ifru_map;
        char ifru_slave[16];
        char ifru_newname[16];
        caddr_t ifru_data;
    } ifr_ifru;
};

typedef int (*compar_fn_t) (const void *, const void *);

#define in_pktinfo  oppsimt_in_pktinfo
struct oppsimt_in_pktinfo
{
  int ipi_ifindex;
  struct in_addr ipi_spec_dst;
  struct in_addr ipi_addr;
};

#define RT_TABLE_MAX RT_TABLE_LOCAL

#define RTM_BASE        0x10

#define RTM_NEWLINK     (RTM_BASE+0)
#define RTM_DELLINK     (RTM_BASE+1)
#define RTM_GETLINK     (RTM_BASE+2)
#define RTM_SETLINK     (RTM_BASE+3)

#define RTM_NEWADDR     (RTM_BASE+4)
#define RTM_DELADDR     (RTM_BASE+5)
#define RTM_GETADDR     (RTM_BASE+6)

#define RTM_NEWROUTE    (RTM_BASE+8)
#define RTM_DELROUTE    (RTM_BASE+9)
#define RTM_GETROUTE    (RTM_BASE+10)

#define RTM_NEWNEIGH    (RTM_BASE+12)
#define RTM_DELNEIGH    (RTM_BASE+13)
#define RTM_GETNEIGH    (RTM_BASE+14)

#define RTM_NEWRULE     (RTM_BASE+16)
#define RTM_DELRULE     (RTM_BASE+17)
#define RTM_GETRULE     (RTM_BASE+18)

#define RTM_NEWQDISC    (RTM_BASE+20)
#define RTM_DELQDISC    (RTM_BASE+21)
#define RTM_GETQDISC    (RTM_BASE+22)

#define RTM_NEWTCLASS   (RTM_BASE+24)
#define RTM_DELTCLASS   (RTM_BASE+25)
#define RTM_GETTCLASS   (RTM_BASE+26)

#define RTM_NEWTFILTER  (RTM_BASE+28)
#define RTM_DELTFILTER  (RTM_BASE+29)
#define RTM_GETTFILTER  (RTM_BASE+30)

#define RTM_MAX         (RTM_BASE+31)

#define RTA_MAX     RTA_SESSION

#define RTN_MAX     RTN_XRESOLVE

#define IFLA_COST IFLA_COST
#define IFLA_PRIORITY IFLA_PRIORITY
#define IFLA_MASTER IFLA_MASTER
#define IFLA_WIRELESS IFLA_WIRELESS
#define IFLA_PROTINFO IFLA_PROTINFO

#define IFLA_MAX        IFLA_PROTINFO

#define IFA_MAX         IFA_CACHEINFO

#define IFA_F_SECONDARY         0x01
#define IFA_F_TEMPORARY         IFA_F_SECONDARY

#define IFA_F_DEPRECATED        0x20
#define IFA_F_TENTATIVE         0x40
#define IFA_F_PERMANENT         0x80

#define RTM_F_NOTIFY            0x100
#define RTM_F_CLONED            0x200
#define RTM_F_EQUALIZE          0x400
#define RTM_F_PREFIX            0x800

#define RTPROT_UNSPEC       0
#define RTPROT_REDIRECT     1
#define RTPROT_KERNEL       2
#define RTPROT_BOOT         3
#define RTPROT_STATIC       4

#define RTPROT_GATED        8
#define RTPROT_RA           9
#define RTPROT_MRT          10
#define RTPROT_ZEBRA        11
#define RTPROT_BIRD         12
#define RTPROT_DNROUTED     13

#define RTMGRP_LINK         1
#define RTMGRP_NOTIFY       2
#define RTMGRP_NEIGH        4
#define RTMGRP_TC           8

#define RTMGRP_IPV4_IFADDR      0x10
#define RTMGRP_IPV4_MROUTE      0x20
#define RTMGRP_IPV4_ROUTE       0x40

#define RTMGRP_IPV6_IFADDR      0x100
#define RTMGRP_IPV6_MROUTE      0x200
#define RTMGRP_IPV6_ROUTE       0x400

#define RTMGRP_DECnet_IFADDR    0x1000
#define RTMGRP_DECnet_ROUTE     0x4000

#define RTA_ALIGNTO             4
#define RTA_ALIGN(len)          ( ((len)+RTA_ALIGNTO-1) & ~(RTA_ALIGNTO-1) )
#define RTA_OK(rta,len)         ((len) > 0 && (rta)->rta_len >= sizeof(struct rtattr) && \
                                    (rta)->rta_len <= (len))
#define RTA_NEXT(rta,attrlen)   ((attrlen) -= RTA_ALIGN((rta)->rta_len), \
                                    (struct rtattr*)(((char*)(rta)) + RTA_ALIGN((rta)->rta_len)))

#define RTA_LENGTH(len)         (RTA_ALIGN(sizeof(struct rtattr)) + (len))
#define RTA_SPACE(len)          RTA_ALIGN(RTA_LENGTH(len))
#define RTA_DATA(rta)           ((void*)(((char*)(rta)) + RTA_LENGTH(0)))
#define RTA_PAYLOAD(rta)        ((int)((rta)->rta_len) - RTA_LENGTH(0))

#define IFLA_RTA(r)             ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
#define IFLA_PAYLOAD(n)         NLMSG_PAYLOAD(n,sizeof(struct ifinfomsg))

#define IFA_RTA(r)              ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
#define IFA_PAYLOAD(n)          NLMSG_PAYLOAD(n,sizeof(struct ifaddrmsg))

#define RTM_RTA(r)              ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct rtmsg))))
#define RTM_PAYLOAD(n)          NLMSG_PAYLOAD(n,sizeof(struct rtmsg))

#define RTNH_ALIGNTO            4
#define RTNH_ALIGN(len)         (((len)+RTNH_ALIGNTO-1) & ~(RTNH_ALIGNTO-1))
#define RTNH_NEXT(rtnh)         ((struct rtnexthop*)(((char*)(rtnh)) + RTNH_ALIGN((rtnh)->rtnh_len)))

#define NETLINK_ROUTE       0

#define NLM_F_REQUEST       1
#define NLM_F_MULTI         2

#define NLM_F_ROOT          0x100
#define NLM_F_MATCH         0x200
#define NLM_F_ACK           4
#define NLM_F_ECHO          8

#define NLM_F_REPLACE       0x100
#define NLM_F_EXCL          0x200
#define NLM_F_CREATE        0x400
#define NLM_F_APPEND        0x800

#define NLMSG_ALIGNTO           4
#define NLMSG_ALIGN(len)        ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )
#define NLMSG_LENGTH(len)       ((len)+NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define NLMSG_SPACE(len)        NLMSG_ALIGN(NLMSG_LENGTH(len))
#define NLMSG_DATA(nlh)         ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))
#define NLMSG_NEXT(nlh,len)     ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
                                    (struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))
#define NLMSG_OK(nlh,len)       ((len) > 0 && (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
                                    (nlh)->nlmsg_len <= (len))
#define NLMSG_PAYLOAD(nlh,len)  ((nlh)->nlmsg_len - NLMSG_SPACE((len)))

#define NLMSG_NOOP          0x1
#define NLMSG_ERROR         0x2
#define NLMSG_DONE          0x3
#define NLMSG_OVERRUN       0x4

#define ARPHRD_ETHER    1
#define ARPHRD_PPP      512
#define ARPHRD_LOOPBACK 772

#define AF_NETLINK      PF_NETLINK
#define AF_PACKET       PF_PACKET
#define PF_NETLINK      16
#define PF_PACKET       17

// arpa/telnet.h

#define IAC             255
#define DONT            254
#define DO              253
#define WILL            251
#define SB              250
#define SE              240

#define TELOPT_ECHO     1
#define TELOPT_SGA      3
#define TELOPT_NAWS     31
#define TELOPT_LINEMODE 34

// sys/syslog.h

#define LOG_EMERG       0
#define LOG_ALERT       1
#define LOG_CRIT        2
#define LOG_ERR         3
#define LOG_WARNING     4
#define LOG_NOTICE      5
#define LOG_INFO        6
#define LOG_DEBUG       7

#define LOG_KERN        (0<<3)
#define LOG_USER        (1<<3)
#define LOG_MAIL        (2<<3)
#define LOG_DAEMON      (3<<3)
#define LOG_AUTH        (4<<3)
#define LOG_SYSLOG      (5<<3)
#define LOG_LPR         (6<<3)
#define LOG_NEWS        (7<<3)
#define LOG_UUCP        (8<<3)
#define LOG_CRON        (9<<3)
#define LOG_AUTHPRIV    (10<<3)
#define LOG_FTP         (11<<3)

#define LOG_LOCAL0      (16<<3)
#define LOG_LOCAL1      (17<<3)
#define LOG_LOCAL2      (18<<3)
#define LOG_LOCAL3      (19<<3)
#define LOG_LOCAL4      (20<<3)
#define LOG_LOCAL5      (21<<3)
#define LOG_LOCAL6      (22<<3)
#define LOG_LOCAL7      (23<<3)

#define LOG_PID         0x01
#define LOG_CONS        0x02
#define LOG_ODELAY      0x04
#define LOG_NDELAY      0x08
#define LOG_NOWAIT      0x10
#define LOG_PERROR      0x20

// asm/fcntl.h


#define F_GETFL     3
#define F_SETFL     4
#define O_NONBLOCK  04000

// asm/signal.h

#define SIGHUP          1
#define SIGINT          2
#define SIGQUIT         3
#define SIGILL          4
#define SIGBUS          7
#define SIGFPE          8
#define SIGUSR1         10
#define SIGSEGV         11
#define SIGUSR2         12
#define SIGPIPE         13
#define SIGALRM         14
#define SIGTERM         15

#define SIG_DFL   (sighandler_t)0
#define SIG_IGN   (sighandler_t)1
#define SIG_SGE   (sighandler_t)3
#define SIG_ACK   (sighandler_t)4

// bits/ioctls.h

#define SIOCADDRT           0x890B
#define SIOCDELRT           0x890C
#define SIOCRTMSG           0x890D

#define SIOCGIFFLAGS        0x8913
#define SIOCSIFFLAGS        0x8914

// net/if.h

#define IF_NAMESIZE     16

#define IFHWADDRLEN    6
#define IFNAMSIZ       IF_NAMESIZE

#define ifr_name        ifr_ifrn.ifrn_name
#define ifr_hwaddr      ifr_ifru.ifru_hwaddr
#define ifr_addr        ifr_ifru.ifru_addr
#define ifr_dstaddr     ifr_ifru.ifru_dstaddr
#define ifr_broadaddr   ifr_ifru.ifru_broadaddr
#define ifr_netmask     ifr_ifru.ifru_netmask
#define ifr_flags       ifr_ifru.ifru_flags
#define ifr_metric      ifr_ifru.ifru_ivalue
#define ifr_mtu         ifr_ifru.ifru_mtu
#define ifr_map         ifr_ifru.ifru_map
#define ifr_slave       ifr_ifru.ifru_slave
#define ifr_data        ifr_ifru.ifru_data
#define ifr_ifindex     ifr_ifru.ifru_ivalue
#define ifr_bandwidth   ifr_ifru.ifru_ivalue
#define ifr_qlen        ifr_ifru.ifru_ivalue
#define ifr_newname     ifr_ifru.ifru_newname
#define _IOT_ifreq     _IOT(_IOTS(char),IFNAMSIZ,_IOTS(char),16,0,0)
#define _IOT_ifreq_short _IOT(_IOTS(char),IFNAMSIZ,_IOTS(short),1,0,0)
#define _IOT_ifreq_int _IOT(_IOTS(char),IFNAMSIZ,_IOTS(int),1,0,0)

#endif
