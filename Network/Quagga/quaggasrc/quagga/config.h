
#define	MULTIPATH_NUM	1

#define	SYSCONFDIR	"/etc/quagga/"

#define DEFAULT_CONFIG_FILE "zebra.conf"

#define CONFIGFILE_MASK 0600

#define LOGFILE_MASK 0600

#define DAEMON_VTY_DIR "/var/run/quagga/"

#define ZEBRA_SERV_PATH "/var/run/quagga/zserv.api"

#define ZEBRA_VTYSH_PATH "/var/run/quagga/zebra.vty"

#define RIPNG_VTYSH_PATH "/var/run/quagga/ripngd.vty"

#define BGP_VTYSH_PATH "/var/run/quagga/bgpd.vty"

#define RIP_VTYSH_PATH "/var/run/quagga/ripd.vty"

#define OSPF6_VTYSH_PATH "/var/run/quagga/ospf6d.vty"

#define OSPF_VTYSH_PATH "/var/run/quagga/ospfd.vty"

#define ISIS_VTYSH_PATH "/var/run/quagga/isisd.vty"

//

#define VERSION "0.98.5"

#define PACKAGE "quagga"

#define PACKAGE_BUGREPORT "http://bugzilla.quagga.net"

#define PACKAGE_NAME "Quagga"

#define PACKAGE_STRING "Quagga 0.98.5"

#define PACKAGE_TARNAME "quagga"

#define PACKAGE_VERSION "0.98.5"

#define PATH_BGPD_PID "/var/run/quagga/bgpd.pid"

#define PATH_ISISD_PID "/var/run/quagga/isisd.pid"

#define PATH_OSPF6D_PID "/var/run/quagga/ospf6d.pid"

#define PATH_OSPFD_PID "/var/run/quagga/ospfd.pid"

#define PATH_RIPD_PID "/var/run/quagga/ripd.pid"

#define PATH_RIPNGD_PID "/var/run/quagga/ripngd.pid"

#define PATH_WATCHQUAGGA_PID "/var/run/quagga/watchquagga.pid"

#define PATH_ZEBRA_PID "/var/run/quagga/zebra.pid"

#define QUAGGA_GROUP "quagga"

#define QUAGGA_USER "quagga"

//

/* #undef DISABLE_BGP_ANNOUNCE */

// #define GNU_LINUX 

#define HAVE_ASM_TYPES_H 1

/* #undef HAVE_BROKEN_ALIASES */

/* #undef HAVE_BROKEN_CMSG_FIRSTHDR */

#define HAVE_DAEMON 1

#define HAVE_DLFCN_H 1

// #define HAVE_FCNTL 1

#define HAVE_GETADDRINFO 1

#define HAVE_GETIFADDRS 1

/* #undef HAVE_GLIBC_BACKTRACE */

// #define HAVE_GNU_REGEX 

/* #undef HAVE_IFALIASREQ */

/* #undef HAVE_IFRA_LIFETIME */

#define HAVE_IF_INDEXTONAME 1

#define HAVE_IF_NAMETOINDEX 1

/* #undef HAVE_IN6_ALIASREQ */


/* #undef HAVE_INET_ND_H */

/* #ifndef _MSC_VER */

#define HAVE_INET_ATON 1
#define HAVE_INET_NTOP 
#define HAVE_INET_PTON 

/* #endif */

#define HAVE_INPKTINFO 

#define HAVE_INTTYPES_H 1

/* #undef HAVE_IPV6 */

/* #undef HAVE_IRDP */

/* #undef HAVE_KVM_H */

/* #undef HAVE_LCAPS */

// #define HAVE_LIBCRYPT 1

/* #undef HAVE_LIBCURSES */

/* #undef HAVE_LIBKVM */

// #define HAVE_LIBM 1

/* #undef HAVE_LIBNCURSES */

/* #undef HAVE_LIBNSL */

/* #undef HAVE_LIBREADLINE */

/* #undef HAVE_LIBRESOLV */

/* #undef HAVE_LIBSOCKET */

/* #undef HAVE_LIBTERMCAP */

/* #undef HAVE_LIBTINFO */

/* #undef HAVE_LIBUMEM */

/* #undef HAVE_LIBUTIL_H */

/* #undef HAVE_LIBXNET */

#define HAVE_LIMITS_H 1

// #define HAVE_LINUX_VERSION_H 1

/* Define to 1 if you have the `memcpy' function. */
#define HAVE_MEMCPY 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet6/in6.h> header file. */
/* #undef HAVE_NETINET6_IN6_H */

/* Define to 1 if you have the <netinet6/in6_var.h> header file. */
/* #undef HAVE_NETINET6_IN6_VAR_H */

/* Define to 1 if you have the <netinet6/nd6.h> header file. */
/* #undef HAVE_NETINET6_ND6_H */

/* Define to 1 if you have the <netinet/icmp6.h> header file. */
/* #undef HAVE_NETINET_ICMP6_H */

/* Define to 1 if you have the <netinet/in6_var.h> header file. */
/* #undef HAVE_NETINET_IN6_VAR_H */

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <netinet/in_systm.h> header file. */
#define HAVE_NETINET_IN_SYSTM_H 1

/* Define to 1 if you have the <netinet/in_var.h> header file. */
/* #undef HAVE_NETINET_IN_VAR_H */

/* netlink */
#define HAVE_NETLINK 1

/* SNMP */
/* #undef HAVE_NETSNMP */

/* Define to 1 if you have the <net/if_dl.h> header file. */
/* #undef HAVE_NET_IF_DL_H */

/* Define to 1 if you have the <net/if.h> header file. */
#define HAVE_NET_IF_H 1

/* Define to 1 if you have the <net/if_var.h> header file. */
/* #undef HAVE_NET_IF_VAR_H */

/* Define to 1 if you have the <net/netopt.h> header file. */
/* #undef HAVE_NET_NETOPT_H */

/* Define to 1 if you have the <net/route.h> header file. */
#define HAVE_NET_ROUTE_H 1

/* NET_RT_IFLIST */
/* #undef HAVE_NET_RT_IFLIST */

/* SNMP */
/* #undef HAVE_NET_SNMP */

/* OSPF Opaque LSA */
/* #undef HAVE_OPAQUE_LSA */

/* Have openpam.h */
/* #undef HAVE_OPENPAM_H */

/* OSPF TE */
/* #undef HAVE_OSPF_TE */

/* Have pam_misc.h */
/* #undef HAVE_PAM_MISC_H */

/* /proc/net/dev */
#define HAVE_PROC_NET_DEV 

/* /proc/net/if_inet6 */
/* #undef HAVE_PROC_NET_IF_INET6 */

/* prctl */
#define HAVE_PR_SET_KEEPCAPS 

/* Enable IPv6 Routing Advertisement support */
#define HAVE_RTADV 

/* rt_addrinfo */
/* #undef HAVE_RT_ADDRINFO */

/* rusage */
/* #undef HAVE_RUSAGE */

/* sa_len */
/* #undef HAVE_SA_LEN */

/* Have setproctitle */
/* #undef HAVE_SETPROCTITLE */

/* scope id */
/* #undef HAVE_SIN6_SCOPE_ID */

/* sin_len */
/* #undef HAVE_SIN_LEN */

/* SNMP */
/* #undef HAVE_SNMP */

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* sockaddr_dl */
/* #undef HAVE_SOCKADDR_DL */

/* socklen_t */
#define HAVE_SOCKLEN_T 

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
// #define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcat' function. */
/* #undef HAVE_STRLCAT */

/* Define to 1 if you have the `strlcpy' function. */
/* #undef HAVE_STRLCPY */

/* Define to 1 if you have the <stropts.h> header file. */
#define HAVE_STROPTS_H 1

/* sun_len */
/* #undef HAVE_SUN_LEN */

/* Define to 1 if you have the <sys/capability.h> header file. */
/* #undef HAVE_SYS_CAPABILITY_H */

/* Define to 1 if you have the <sys/conf.h> header file. */
/* #undef HAVE_SYS_CONF_H */

/* Define to 1 if you have the <sys/ksym.h> header file. */
/* #undef HAVE_SYS_KSYM_H */

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/sockio.h> header file. */
/* #undef HAVE_SYS_SOCKIO_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/sysctl.h> header file. */
#define HAVE_SYS_SYSCTL_H 1

/* Define to 1 if you have the <sys/times.h> header file. */
#define HAVE_SYS_TIMES_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Use TCP for zebra communication */
#define HAVE_TCP_ZEBRA 

/* Define to 1 if you have the <ucontext.h> header file. */
// #define HAVE_UCONTEXT_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* INRIA IPv6 */
/* #undef INRIA_IPV6 */

/* IRIX 6.5 */
/* #undef IRIX_65 */

/* KAME IPv6 stack */
/* #undef KAME */

/* Linux IPv6 stack */
/* #undef LINUX_IPV6 */

/* Musica IPv6 stack */
/* #undef MUSICA */

/* NRL */
/* #undef NRL */

/* OpenBSD */
/* #undef OPEN_BSD */

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Solaris IPv6 */
/* #undef SOLARIS_IPV6 */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* SunOS 5 */
/* #undef SUNOS_5 */

/* SunOS 5.6 to 5.7 */
/* #undef SUNOS_56 */

/* SunOS 5.8 up */
/* #undef SUNOS_59 */

/* OSPFAPI */
/* #undef SUPPORT_OSPF_API */

/* SNMP */
/* #undef UCD_COMPATIBLE */

/* Use PAM for authentication */
/* #undef USE_PAM */

/* Version number of package */

/* VTY shell */
/* #undef VTYSH */

/* VTY Sockets Group */
/* #undef VTY_GROUP */

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Have openpam_ttyconv */
/* #undef PAM_CONV_FUNC */

/* Define to 1 if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Old readline */
/* #undef rl_completion_matches */

// taken from regex.c, must be defined for globalvars.h
#define CHAR_SET_SIZE 256

#include "zebra_env.h"

#include "syscalls.h"

#include "structs.h"

#include "oppsim_kernel.h"
