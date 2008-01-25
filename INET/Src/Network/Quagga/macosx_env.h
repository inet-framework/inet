#ifndef __MACOSX_ENV_H__
#define	__MACOSX_ENV_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <sys/malloc.h>
#include <sys/un.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>

#undef sigfillset

typedef int8_t    s8;
typedef uint8_t   u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef int32_t  s32;
typedef uint32_t u32;
typedef int64_t  s64;
typedef uint64_t u64;

#define IP_PKTINFO      19


#undef SO_REUSEPORT

#define struct_sigaction  struct sigaction

#define CMSG_ALIGN ALIGN

// netlink & messages

struct sockaddr_nl
{
        __uint8_t sa_len; /* unlike on Linux, sockaddr begins on OS/X with sa_len! */
        sa_family_t nl_family;
        unsigned short nl_pad;
        u32 nl_pid;
        u32 nl_groups;
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

#define RTA_MAX		RTA_SESSION

#define RTN_MAX		RTN_XRESOLVE

#define IFLA_COST IFLA_COST
#define IFLA_PRIORITY IFLA_PRIORITY
#define IFLA_MASTER IFLA_MASTER
#define IFLA_WIRELESS IFLA_WIRELESS
#define IFLA_PROTINFO IFLA_PROTINFO

#define IFLA_MAX		IFLA_PROTINFO

#define IFA_MAX			IFA_CACHEINFO

#define IFA_F_SECONDARY			0x01
#define IFA_F_TEMPORARY			IFA_F_SECONDARY

#define IFA_F_DEPRECATED		0x20
#define IFA_F_TENTATIVE			0x40
#define IFA_F_PERMANENT			0x80

#define RTM_F_NOTIFY            0x100
#define RTM_F_CLONED            0x200
#define RTM_F_EQUALIZE          0x400
#define RTM_F_PREFIX            0x800

#define RTPROT_UNSPEC		0
#define RTPROT_REDIRECT		1
#define RTPROT_KERNEL		2
#define RTPROT_BOOT			3
#define RTPROT_STATIC		4

#define RTPROT_GATED		8
#define RTPROT_RA			9
#define RTPROT_MRT			10
#define RTPROT_ZEBRA		11
#define RTPROT_BIRD			12
#define RTPROT_DNROUTED		13

#define RTMGRP_LINK			1
#define RTMGRP_NOTIFY		2
#define RTMGRP_NEIGH		4
#define RTMGRP_TC			8

#define RTMGRP_IPV4_IFADDR		0x10
#define RTMGRP_IPV4_MROUTE		0x20
#define RTMGRP_IPV4_ROUTE		0x40

#define RTMGRP_IPV6_IFADDR		0x100
#define RTMGRP_IPV6_MROUTE		0x200
#define RTMGRP_IPV6_ROUTE		0x400

#define RTMGRP_DECnet_IFADDR	0x1000
#define RTMGRP_DECnet_ROUTE		0x4000

#define RTA_ALIGNTO				4
#define RTA_ALIGN(len)			( ((len)+RTA_ALIGNTO-1) & ~(RTA_ALIGNTO-1) )
#define RTA_OK(rta,len)			((len) > 0 && (rta)->rta_len >= sizeof(struct rtattr) && \
									(rta)->rta_len <= (len))
#define RTA_NEXT(rta,attrlen)	((attrlen) -= RTA_ALIGN((rta)->rta_len), \
									(struct rtattr*)(((char*)(rta)) + RTA_ALIGN((rta)->rta_len))) 

#define RTA_LENGTH(len)			(RTA_ALIGN(sizeof(struct rtattr)) + (len))
#define RTA_SPACE(len)			RTA_ALIGN(RTA_LENGTH(len))
#define RTA_DATA(rta)			((void*)(((char*)(rta)) + RTA_LENGTH(0))) 
#define RTA_PAYLOAD(rta)		((int)((rta)->rta_len) - RTA_LENGTH(0)) 

#define IFLA_RTA(r)				((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
#define IFLA_PAYLOAD(n)			NLMSG_PAYLOAD(n,sizeof(struct ifinfomsg))

#define IFA_RTA(r)				((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
#define IFA_PAYLOAD(n)			NLMSG_PAYLOAD(n,sizeof(struct ifaddrmsg))

#define RTM_RTA(r)				((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct rtmsg))))
#define RTM_PAYLOAD(n) 			NLMSG_PAYLOAD(n,sizeof(struct rtmsg))

#define RTNH_ALIGNTO			4
#define RTNH_ALIGN(len)			(((len)+RTNH_ALIGNTO-1) & ~(RTNH_ALIGNTO-1))
#define RTNH_NEXT(rtnh)			((struct rtnexthop*)(((char*)(rtnh)) + RTNH_ALIGN((rtnh)->rtnh_len)))

#define NETLINK_ROUTE		0

#define NLM_F_REQUEST		1
#define NLM_F_MULTI			2

#define NLM_F_ROOT			0x100
#define NLM_F_MATCH			0x200
#define NLM_F_ACK			4
#define NLM_F_ECHO			8

#define NLM_F_REPLACE		0x100
#define NLM_F_EXCL			0x200
#define NLM_F_CREATE		0x400
#define NLM_F_APPEND		0x800

#define NLMSG_ALIGNTO			4
#define NLMSG_ALIGN(len)		( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )
#define NLMSG_LENGTH(len)		((len)+NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define NLMSG_SPACE(len)		NLMSG_ALIGN(NLMSG_LENGTH(len)) 
#define NLMSG_DATA(nlh)			((void*)(((char*)nlh) + NLMSG_LENGTH(0)))
#define NLMSG_NEXT(nlh,len)		((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
									(struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))
#define NLMSG_OK(nlh,len)		((len) > 0 && (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
                           			(nlh)->nlmsg_len <= (len)) 
#define NLMSG_PAYLOAD(nlh,len)	((nlh)->nlmsg_len - NLMSG_SPACE((len)))

#define NLMSG_NOOP			0x1
#define NLMSG_ERROR			0x2
#define NLMSG_DONE			0x3
#define NLMSG_OVERRUN		0x4

#define	ARPHRD_ETHER	1
#define ARPHRD_PPP      512
#define ARPHRD_LOOPBACK	772

#define	AF_NETLINK		PF_NETLINK
#define	AF_PACKET		PF_PACKET
#define PF_NETLINK		16
#define PF_PACKET		17

// arpa/telnet.h

#define IAC				255
#define DONT			254
#define DO				253
#define WILL			251
#define SB				250
#define SE				240

#define TELOPT_ECHO		1
#define TELOPT_SGA		3
#define TELOPT_NAWS		31
#define TELOPT_LINEMODE	34

// sys/syslog.h

#define LOG_EMERG		0
#define LOG_ALERT		1
#define LOG_CRIT		2
#define LOG_ERR			3
#define LOG_WARNING		4
#define LOG_NOTICE		5
#define LOG_INFO		6
#define LOG_DEBUG		7

#define LOG_KERN		(0<<3)
#define LOG_USER		(1<<3)
#define LOG_MAIL		(2<<3)
#define LOG_DAEMON		(3<<3)
#define LOG_AUTH		(4<<3)
#define LOG_SYSLOG		(5<<3)
#define LOG_LPR			(6<<3)
#define LOG_NEWS		(7<<3)
#define LOG_UUCP		(8<<3)
#define LOG_CRON		(9<<3)
#define LOG_AUTHPRIV	(10<<3)
#define LOG_FTP			(11<<3)

#define LOG_LOCAL0		(16<<3)
#define LOG_LOCAL1		(17<<3)
#define LOG_LOCAL2		(18<<3)
#define LOG_LOCAL3		(19<<3)
#define LOG_LOCAL4		(20<<3)
#define LOG_LOCAL5		(21<<3)
#define LOG_LOCAL6		(22<<3)
#define LOG_LOCAL7		(23<<3)

#define LOG_PID			0x01
#define LOG_CONS		0x02
#define LOG_ODELAY		0x04
#define LOG_NDELAY		0x08
#define LOG_NOWAIT		0x10
#define LOG_PERROR		0x20

// bits/ioctls.h

#define SIOCADDRT			0x890B
#define SIOCDELRT			0x890C
#define SIOCRTMSG			0x890D

#define SIOCGIFFLAGS		0x8913
#define SIOCSIFFLAGS		0x8914

#endif
