#ifndef __MSVCENV_H__
#define __MSVCENV_H__

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <stddef.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include <Winsock.h>

#define __inline__  __inline

#ifdef __cplusplus
extern "C" {
#endif

int snprintf (char *s, size_t maxlen, const char *format, ...);
int vsnprintf(char *s, size_t maxlen, const char *format, va_list arg);
int strncasecmp(const char *s1, const char *s2, size_t n);

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

#undef CMSG_DATA
#define CMSG_DATA(cmsg)             ((cmsg)->__cmsg_data)

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

// basic types

typedef unsigned int mode_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef int pid_t;
typedef long int time_t;
typedef unsigned int socklen_t;
typedef long int ssize_t;
typedef unsigned char u_char;
typedef unsigned short int u_short;
typedef unsigned int u_int;
typedef unsigned long int u_long;
typedef unsigned int useconds_t;
typedef long int suseconds_t;
typedef unsigned long int ino64_t;
typedef unsigned long int nlink_t;
typedef long int off_t;
typedef long int off64_t;
typedef long int blksize_t;
typedef long int blkcnt_t;
typedef long int blkcnt64_t;
typedef long int clock_t;
typedef long int fd_mask;

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;

typedef char *caddr_t;

typedef unsigned __int8 u_int8_t;
typedef unsigned __int16 u_int16_t;
typedef unsigned __int32 u_int32_t;
typedef unsigned __int64 u_int64_t;

typedef __int32 int32_t;

// time

struct timespec
{
    time_t tv_sec;
    long int tv_nsec;
};

// passwd

struct group
{
    char *gr_name;
    char *gr_passwd;
    gid_t gr_gid;
    char **gr_mem;
};

struct passwd
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

struct iovec
{
    void *iov_base;
    size_t iov_len;
};

// signal

typedef int sig_atomic_t;

typedef void (*sighandler_t) (int);

typedef struct
{
    unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
} sigset_t;


typedef union sigval
{
    int sival_int;
    void *sival_ptr;
} sigval_t;

typedef struct siginfo
{
    int si_signo;
    int si_errno;
    int si_code;

    union
    {
        int _pad[((128 / sizeof (int)) - 4)];

        struct
        {
            pid_t si_pid;
            uid_t si_uid;
        } _kill;

        struct
        {
            int si_tid;
            int si_overrun;
            sigval_t si_sigval;
        } _timer;

        struct
        {
            pid_t si_pid;
            uid_t si_uid;
            sigval_t si_sigval;
        } _rt;

        struct
        {
            pid_t si_pid;
            uid_t si_uid;
            int si_status;
            clock_t si_utime;
            clock_t si_stime;
        } _sigchld;

        struct
        {
            void *si_addr;
        } _sigfault;

        struct
        {
            long int si_band;
            int si_fd;
        } _sigpoll;

    } _sifields;

} siginfo_t;

#define struct_sigaction  struct sigaction
struct sigaction
{
    sighandler_t sa_handler;

    sigset_t sa_mask;

    int sa_flags;

    void (*sa_restorer) (void);
};

// uname

struct utsname
{
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
    char __domainname[65];
};

// networking stuff

typedef uint32_t in_addr_t;

typedef unsigned short int sa_family_t;

struct sockaddr_nl
{
        sa_family_t nl_family;
        unsigned short nl_pad;
        u32 nl_pid;
        u32 nl_groups;
};

struct ip
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

struct sockaddr_un
{
    sa_family_t sun_family;
    char sun_path[108];
};

typedef uint16_t in_port_t;

struct in6_addr
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

struct cmsghdr
{
    size_t cmsg_len;

    int cmsg_level;
    int cmsg_type;

    unsigned char __cmsg_data [];
};

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

struct nlmsghdr
{
        u32 nlmsg_len;
        u16 nlmsg_type;
        u16 nlmsg_flags;
        u32 nlmsg_seq;
        u32 nlmsg_pid;
};

struct rtgenmsg
{
    unsigned char rtgen_family;
};

struct ifinfomsg
{
    unsigned char ifi_family;
    unsigned char __ifi_pad;
    unsigned short ifi_type;
    int ifi_index;
    unsigned ifi_flags;
    unsigned ifi_change;
};

struct nlmsgerr
{
    int error;
    struct nlmsghdr msg;
};

struct rtattr
{
    unsigned short rta_len;
    unsigned short rta_type;
};

struct rtmsg
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

struct ifaddrmsg
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

struct rtnexthop
{
    unsigned short rtnh_len;
    unsigned char rtnh_flags;
    unsigned char rtnh_hops;
    int rtnh_ifindex;
};

struct ifmap
{
    unsigned long int mem_start;
    unsigned long int mem_end;
    unsigned short int base_addr;
    unsigned char irq;
    unsigned char dma;
    unsigned char port;
};

struct ifreq
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

struct in_pktinfo
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
