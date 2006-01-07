#ifndef OPPSIM_KERNEL_H_
#define OPPSIM_KERNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "zebra_env.h"

// pieces from quaggasrc needed in Daemon

extern struct GlobalVars * __activeVars;

extern void GlobalVars_initializeActiveSet_ripd();
extern void GlobalVars_initializeActiveSet_ospfd();
extern void GlobalVars_initializeActiveSet_zebra();
extern void GlobalVars_initializeActiveSet_lib();

extern struct GlobalVars * GlobalVars_createActiveSet();
extern int GlobalVars_errno();

//

#define	HASFLAG(a, b)	((a & b) == b)


struct timezone;

long int oppsim_random(void);

ssize_t nl_request(int socket, const void *message, size_t length, int flags);

// borrowed from zebra
extern void netlink_parse_rtattr (struct rtattr **tb, int max, struct rtattr *rta, int len);

int oppsim_open(const char *path, int oflag, ...);
struct servent *oppsim_getservbyname(const char *name, const char *proto);

char *oppsim_getenv(const char *name);
time_t oppsim_time(time_t *tloc);
char *oppsim_crypt(const char *key, const char *salt);
int oppsim_setregid(gid_t rgid, gid_t egid);
int oppsim_setreuid(uid_t ruid, uid_t euid);
int oppsim_seteuid(uid_t uid);
uid_t oppsim_getuid(void);
uid_t oppsim_geteuid(void);
mode_t oppsim_umask(mode_t cmask);
void oppsim_exit(int status);
pid_t oppsim_getpid(void);
int oppsim_daemon (int nochdir, int noclose);
void oppsim_srand(unsigned seed);
int oppsim_gettimeofday(struct timeval *tp, struct timezone *tzp);
void oppsim_sync(void);
int oppsim_setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
int oppsim_socket(int domain, int type, int protocol);

int oppsim_bind(int socket, const struct sockaddr *address, socklen_t address_len);
int oppsim_listen(int socket, int backlog);
void oppsim_openlog(const char *ident, int logopt, int facility);
void oppsim_closelog();
void oppsim_vsyslog(int priority, const char *format, va_list ap);
int oppsim_fcntl(int fildes, int cmd, ...);
int oppsim_getsockname(int socket, struct sockaddr *address, socklen_t *address_len);
ssize_t oppsim_sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);
ssize_t oppsim_recvmsg(int socket, struct msghdr *message, int flags);
FILE* oppsim_fopen(const char * filename, const char * mode);
int oppsim_getpagesize();
ssize_t oppsim_write(int fildes, const void *buf, size_t nbyte);
int oppsim_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout);

int oppsim_close(int fildes);
void oppsim_perror(const char *s);
int oppsim_accept(int socket, struct sockaddr *address, socklen_t *address_len);
int oppsim_unlink(const char *path);
int oppsim_getsockopt(int socket, int level, int option_name, void *option_value, socklen_t *option_len);
ssize_t oppsim_sendmsg(int socket, const struct msghdr *message, int flags);
int oppsim_ioctl(int fildes, int request, ...);
ssize_t oppsim_read(int fildes, void *buf, size_t nbyte);
int oppsim_uname(struct utsname *name);
int oppsim_mkstemp(char *tmpl);
int oppsim_chmod(const char *path, mode_t mode);
int oppsim_unlink(const char *path);
int oppsim_link(const char *path1, const char *path2);
char *oppsim_getcwd(char *buf, size_t size);
int oppsim_chdir(const char *path);
ssize_t oppsim_writev(int fildes, const struct iovec *iov, int iovcnt);
struct passwd *oppsim_getpwnam(const char *name);
struct group *oppsim_getgrnam(const char *name);
int oppsim_setgroups(size_t size, const gid_t *list);
int oppsim_connect(int socket, const struct sockaddr *address, socklen_t address_len);
int oppsim_getpeername(int socket, struct sockaddr *address, socklen_t *address_len);
void oppsim_abort();
int oppsim_sigfillset(sigset_t *set);
void oppsim__exit(int status);
int oppsim_stat(const char *path, struct stat *buf);
int oppsim_sigaction(int sig, const struct_sigaction *act, struct_sigaction *oact);

ssize_t oppsim_recvfrom(int socket, void *buffer, size_t length, int flags, struct sockaddr *address, socklen_t *address_len);

u_long oppsim_htonl(u_long hostlong);
u_short oppsim_htons(u_short hostshort);
char *oppsim_inet_ntoa(struct in_addr in);
u_long oppsim_ntohl(u_long netlong);
u_short oppsim_ntohs(u_short netshort);
unsigned long oppsim_inet_addr(const char *str);
int oppsim_inet_aton(const char *cp, struct in_addr *addr);
int oppsim_inet_pton (int af, const char *strptr, void *addrptr);
char *oppsim_inet_ntop(int af, const void *src, char *dst, size_t size);

extern time_t zero_time;

#ifdef __cplusplus
};
#endif

#endif
