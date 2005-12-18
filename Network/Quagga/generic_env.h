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
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

//#include <Winsock.h>

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

#define uint8_t     unsigned char
#define uint16_t    unsigned short int
#define uint32_t    unsigned int

#define s8          signed char
#define u8          unsigned char
#define s16         signed short
#define u16         unsigned short
#define s32         signed int
#define u32         unsigned int
#define s64         signed long long
#define u64         unsigned long long

typedef char *caddr_t;

#ifdef _MSC_VER
#  define u_int8_t    unsigned __int8
#  define u_int16_t   unsigned __int16
#  define u_int32_t   unsigned __int32
#  define u_int64_t   unsigned __int64
#  define int32_t     __int32
#else
// FIXME what are the gnu-ish equivalents?
#  define u_int8_t    unsigned char
#  define u_int16_t   unsigned short
#  define u_int32_t   unsigned int
#  define u_int64_t   unsigned long long
#  define int32_t     int
#endif

// in_addr (from winsock.h)

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
        u_short sa_family;              /* address family */
        char    sa_data[14];            /* up to 14 bytes of direct address */
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
    //FIXME to be implemented
    ASSERT(0);
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
#define AF_UNSPEC       0               /* unspecified */
#define AF_UNIX         1               /* local to host (pipes, portals) */
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define AF_IMPLINK      3               /* arpanet imp addresses */
#define AF_PUP          4               /* pup protocols: e.g. BSP */
#define AF_CHAOS        5               /* mit CHAOS protocols */
#define AF_IPX          6               /* IPX and SPX */
#define AF_NS           6               /* XEROX NS protocols */
#define AF_ISO          7               /* ISO protocols */
#define AF_OSI          AF_ISO          /* OSI is ISO */
#define AF_ECMA         8               /* european computer manufacturers */
#define AF_DATAKIT      9               /* datakit protocols */
#define AF_CCITT        10              /* CCITT protocols, X.25 etc */
#define AF_SNA          11              /* IBM SNA */
#define AF_DECnet       12              /* DECnet */
#define AF_DLI          13              /* Direct data link interface */
#define AF_LAT          14              /* LAT */
#define AF_HYLINK       15              /* NSC Hyperchannel */
#define AF_APPLETALK    16              /* AppleTalk */
#define AF_NETBIOS      17              /* NetBios-style addresses */
#define AF_VOICEVIEW    18              /* VoiceView */
#define AF_FIREFOX      19              /* FireFox */
#define AF_UNKNOWN1     20              /* Somebody is using this! */
#define AF_BAN          21              /* Banyan */

#define AF_MAX          22

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
        char ** s_aliases;      /* alias list */
        short   s_port;           /* port # */
        char  * s_proto;          /* protocol to use */
};

#define MSG_OOB         0x1             /* process out-of-band data */
#define MSG_PEEK        0x2             /* peek at incoming message */
#define MSG_DONTROUTE   0x4             /* send without using routing tables */

#define MSG_MAXIOVLEN   16

#define MSG_PARTIAL     0x8000          /* partial send or recv for message xport */

// FIXME to be emulated:
#define htonl      oppsim_htonl
#define htons      oppsim_htons
#define inet_ntoa  oppsim_inet_ntoa
#define ntohl      oppsim_ntohl
#define ntohs      oppsim_ntohs

static u_long oppsim_htonl(u_long hostlong) {return 0;}
static u_short oppsim_htons(u_short hostshort) {return 0;}
static char *oppsim_inet_ntoa(struct in_addr in) {return "fixme";}
static u_long oppsim_ntohl(u_long netlong) {return 0;}
static u_short oppsim_ntohs(u_short netshort) {return 0;}

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
