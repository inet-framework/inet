/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University and Ericsson AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Erik Nordström, <erik.nordstrom@it.uu.se>
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <linux/sockios.h>
#include <linux/wireless.h>
#include <getopt.h>
#include <ctype.h>

#include "defs.h"
#include "debug.h"
#include "timer_queue.h"
#include "params.h"
#include "aodv_socket.h"
#include "aodv_timeout.h"
#include "k_route.h"
#include "routing_table.h"
#include "aodv_hello.h"
#include "nl.h"

#ifdef LLFEEDBACK
#include "llf.h"
#endif

/* Global variables: */
int log_to_file = 0;
int rt_log_interval = 0;	/* msecs between routing table logging 0=off */
int unidir_hack = 0;
int rreq_gratuitous = 0;
int expanding_ring_search = 1;
int internet_gw_mode = 0;
int local_repair = 0;
int receive_n_hellos = 0;
int hello_jittering = 1;
int optimized_hellos = 0;
int ratelimit = 1;		/* Option for rate limiting RREQs and RERRs. */
char *progname;
int wait_on_reboot = 1;
int qual_threshold = 0;
int llfeedback = 0;
int gw_prefix = 1;
struct timer worb_timer;	/* Wait on reboot timer */

/* Dynamic configuration values */
int active_route_timeout = ACTIVE_ROUTE_TIMEOUT_HELLO;
int ttl_start = TTL_START_HELLO;
int delete_period = DELETE_PERIOD_HELLO;

static void cleanup();

struct option longopts[] = {
    {"interface", required_argument, NULL, 'i'},
    {"hello-jitter", no_argument, NULL, 'j'},
    {"log", no_argument, NULL, 'l'},
    {"n-hellos", required_argument, NULL, 'n'},
    {"daemon", no_argument, NULL, 'd'},
    {"force-gratuitous", no_argument, NULL, 'g'},
    {"opt-hellos", no_argument, NULL, 'o'},
    {"quality-threshold", required_argument, NULL, 'q'},
    {"log-rt-table", required_argument, NULL, 'r'},
    {"unidir_hack", no_argument, NULL, 'u'},
    {"gateway-mode", no_argument, NULL, 'w'},
    {"help", no_argument, NULL, 'h'},
    {"no-expanding-ring", no_argument, NULL, 'x'},
    {"no-worb", no_argument, NULL, 'D'},
    {"local-repair", no_argument, NULL, 'L'},
    {"rate-limit", no_argument, NULL, 'R'},
    {"version", no_argument, NULL, 'V'},
    {"llfeedback", no_argument, NULL, 'f'},
    {0}
};

void usage(int status)
{
    if (status != 0) {
	fprintf(stderr, "Try `%s --help' for more information.\n", progname);
	exit(status);
    }

    printf
	("\nUsage: %s [-dghjlouwxLDRV] [-i if0,if1,..] [-r N] [-n N] [-q THR]\n\n"
	 "-d, --daemon            Daemon mode, i.e. detach from the console.\n"
	 "-g, --force-gratuitous  Force the gratuitous flag to be set on all RREQ's.\n"
	 "-h, --help              This information.\n"
	 "-i, --interface         Network interfaces to attach to. Defaults to first\n"
	 "                        wireless interface.\n"
	 "-j, --hello-jitter      Toggle hello jittering (default ON).\n"
	 "-l, --log               Log debug output to %s.\n"
	 "-o, --opt-hellos        Send HELLOs only when forwarding data (experimental).\n"
	 "-r, --log-rt-table      Log routing table to %s every N secs.\n"
	 "-n, --n-hellos          Receive N hellos from host before treating as neighbor.\n"
	 "-u, --unidir-hack       Detect and avoid unidirectional links (experimental).\n"
	 "-w, --gateway-mode      Enable experimental Internet gateway support.\n"
	 "-x, --no-expanding-ring Disable expanding ring search for RREQs.\n"
	 "-D, --no-worb           Disable 15 seconds wait on reboot delay.\n"
	 "-L, --local-repair      Enable local repair.\n"
	 "-f, --llfeedback        Enable link layer feedback.\n"
	 "-R, --rate-limit        Toggle rate limiting of RREQs and RERRs (default ON).\n"
	 "-q, --quality-threshold Set a minimum signal quality threshold for control packets.\n"
	 "-V, --version           Show version.\n\n"
	 "Erik Nordström, <erik.nordstrom@it.uu.se>\n\n",
	 progname, AODV_LOG_PATH, AODV_RT_LOG_PATH);

    exit(status);
}

int set_kernel_options()
{
    int i, fd = -1;
    char on = '1';
    char off = '0';
    char command[64];

    if ((fd = open("/proc/sys/net/ipv4/ip_forward", O_WRONLY)) < 0)
	return -1;
    if (write(fd, &on, sizeof(char)) < 0)
    {
        close(fd);
        return -1;
    }
    close(fd);

    if ((fd = open("/proc/sys/net/ipv4/route/max_delay", O_WRONLY)) < 0)
	return -1;
    if (write(fd, &off, sizeof(char)) < 0)
    {
        close(fd);
        return -1;
    }
    close(fd);

    if ((fd = open("/proc/sys/net/ipv4/route/min_delay", O_WRONLY)) < 0)
	return -1;
    if (write(fd, &off, sizeof(char)) < 0)
    {
        close(fd);
        return -1;
    }
    close(fd);

    /* Disable ICMP redirects on all interfaces: */

    for (i = 0; i < MAX_NR_INTERFACES; i++) {
	if (!DEV_NR(i).enabled)
	    continue;

	memset(command, '\0', 64);
	sprintf(command, "/proc/sys/net/ipv4/conf/%s/send_redirects",
		DEV_NR(i).ifname);
	if ((fd = open(command, O_WRONLY)) < 0)
	    return -1;
	if (write(fd, &off, sizeof(char)) < 0)
    {
        close(fd);
        return -1;
    }
	close(fd);
	memset(command, '\0', 64);
	sprintf(command, "/proc/sys/net/ipv4/conf/%s/accept_redirects",
		DEV_NR(i).ifname);
	if ((fd = open(command, O_WRONLY)) < 0)
	    return -1;
	if (write(fd, &off, sizeof(char)) < 0)
    {
        close(fd);
        return -1;
    }
	close(fd);
    }
    memset(command, '\0', 64);
    sprintf(command, "/proc/sys/net/ipv4/conf/all/send_redirects");
    if ((fd = open(command, O_WRONLY)) < 0)
	return -1;
    if (write(fd, &off, sizeof(char)) < 0)
    {
        close(fd);
        return -1;
    }
    close(fd);


    memset(command, '\0', 64);
    sprintf(command, "/proc/sys/net/ipv4/conf/all/accept_redirects");
    if ((fd = open(command, O_WRONLY)) < 0)
	return -1;
    if (write(fd, &off, sizeof(char)) < 0)
    {
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}

int find_default_gw(void)
{
    FILE *route;
    char buf[100], *l;

    route = fopen("/proc/net/route", "r");

    if (route == NULL) {
	perror("open /proc/net/route");
	exit(-1);
    }

    while (fgets(buf, sizeof(buf), route)) {
	l = strtok(buf, " \t");
	l = strtok(NULL, " \t");
	if (l != NULL) {
	    if (strcmp("00000000", l) == 0) {
		l = strtok(NULL, " \t");
		l = strtok(NULL, " \t");
		if (strcmp("0003", l) == 0) {
		    fclose(route);
		    return 1;
		}
	    }
	}
    }
    fclose(route);
    return 0;
}

/*
 * Returns information on a network interface given its name...
 */
struct sockaddr_in *get_if_info(char *ifname, int type)
{
    int skfd;
    struct sockaddr_in *ina;
    static struct ifreq ifr;

    /* Get address of interface... */
    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, type, &ifr) < 0) {
	alog(LOG_ERR, errno, __FUNCTION__,
	     "Could not get address of %s ", ifname);
	close(skfd);
	return NULL;
    } else {
	ina = (struct sockaddr_in *) &ifr.ifr_addr;
	close(skfd);
	return ina;
    }
}

/* This will limit the number of handler functions we can have for
   sockets and file descriptors and so on... */
#define CALLBACK_FUNCS 5
static struct callback {
    int fd;
    callback_func_t func;
} callbacks[CALLBACK_FUNCS];

static int nr_callbacks = 0;

int attach_callback_func(int fd, callback_func_t func)
{
    if (nr_callbacks >= CALLBACK_FUNCS) {
	fprintf(stderr, "callback attach limit reached!!\n");
	exit(-1);
    }
    callbacks[nr_callbacks].fd = fd;
    callbacks[nr_callbacks].func = func;
    nr_callbacks++;
    return 0;
}

/* Here we find out how to load the kernel modules... If the modules
   are located in the current directory. use those. Otherwise fall
   back to modprobe. */

void load_modules(char *ifname)
{
    struct stat st;
    char buf[1024], *l = NULL;
    int found = 0;
    FILE *m;

    memset(buf, '\0', 64);

    if (stat("./kaodv.ko", &st) == 0)
	sprintf(buf, "/sbin/insmod kaodv.ko ifname=%s &>/dev/null", ifname);
    else if (stat("./kaodv.o", &st) == 0)
	sprintf(buf, "/sbin/insmod kaodv.o ifname=%s &>/dev/null", ifname);
    else
	sprintf(buf, "/sbin/modprobe kaodv ifname=%s &>/dev/null", ifname);

    system(buf);

    usleep(100000);

    /* Check result */
    m = fopen("/proc/modules", "r");
    while (fgets(buf, sizeof(buf), m)) {
	l = strtok(buf, " \t");
	if (!strcmp(l, "kaodv"))
	    found++;
	if (!strcmp(l, "ipchains")) {
	    fprintf(stderr,
		    "The ipchains kernel module is loaded and prevents AODV-UU from functioning properly.\n");
	    exit(-1);
	}
    }
    fclose(m);

    if (found < 1) {
	fprintf(stderr,
		"A kernel module could not be loaded, check your installation... %d\n",
		found);
	exit(-1);
    }
}

void remove_modules(void)
{
    system("/sbin/rmmod kaodv &>/dev/null");
}

void host_init(char *ifname)
{
    struct sockaddr_in *ina;
    char buf[1024], tmp_ifname[IFNAMSIZ],
	ifnames[(IFNAMSIZ + 1) * MAX_NR_INTERFACES], *iface;
    struct ifconf ifc;
    struct ifreq ifreq, *ifr;
    int i, iw_sock, if_sock = 0;

    memset(&this_host, 0, sizeof(struct host_info));
    memset(dev_indices, 0, sizeof(unsigned int) * MAX_NR_INTERFACES);

    if (!ifname) {
	/* No interface was given... search for first wireless. */
	iw_sock = socket(PF_INET, SOCK_DGRAM, 0);
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(iw_sock, SIOCGIFCONF, &ifc) < 0) {
	    fprintf(stderr, "Could not get wireless info\n");
	    exit(-1);
	}
	ifr = ifc.ifc_req;
	for (i = ifc.ifc_len / sizeof(struct ifreq); i >= 0; i--, ifr++) {
	    struct iwreq req;

	    strcpy(req.ifr_name, ifr->ifr_name);
	    if (ioctl(iw_sock, SIOCGIWNAME, &req) >= 0) {
		strcpy(tmp_ifname, ifr->ifr_name);
		break;
	    }
	}
	/* Did we find a wireless interface? */
	if (!strlen(tmp_ifname)) {
	    fprintf(stderr, "\nCould not find a wireless interface!\n");
	    fprintf(stderr, "Use -i <interface> to override...\n\n");
	    exit(-1);
	}
	strcpy(ifreq.ifr_name, tmp_ifname);
	if (ioctl(iw_sock, SIOCGIFINDEX, &ifreq) < 0) {
	    alog(LOG_ERR, errno, __FUNCTION__,
		 "Could not get index of %s", tmp_ifname);
	    close(if_sock);
	    exit(-1);
	}
	close(iw_sock);

	ifname = tmp_ifname;

	alog(LOG_NOTICE, 0, __FUNCTION__,
	     "Attaching to %s, override with -i <if1,if2,...>.", tmp_ifname);
    }

    strcpy(ifnames, ifname);

    /* Intitialize the local sequence number an rreq_id to zero */
    this_host.seqno = 1;
    this_host.rreq_id = 0;

    /* Zero interfaces enabled so far... */
    this_host.nif = 0;

    gettimeofday(&this_host.bcast_time, NULL);

    /* Find the indices of all interfaces to broadcast on... */
    if_sock = socket(AF_INET, SOCK_DGRAM, 0);

    iface = strtok(ifname, ",");

    /* OK, now lookup interface information, and store it... */
    do {
	strcpy(ifreq.ifr_name, iface);
	if (ioctl(if_sock, SIOCGIFINDEX, &ifreq) < 0) {
	    alog(LOG_ERR, errno, __FUNCTION__, "Could not get index of %s",
		 iface);
	    close(if_sock);
	    exit(-1);
	}
	this_host.devs[this_host.nif].ifindex = ifreq.ifr_ifindex;

	dev_indices[this_host.nif++] = ifreq.ifr_ifindex;

	strcpy(DEV_IFINDEX(ifreq.ifr_ifindex).ifname, iface);

	/* Get IP-address of interface... */
	ina = get_if_info(iface, SIOCGIFADDR);
	if (ina == NULL)
	    exit(-1);

	DEV_IFINDEX(ifreq.ifr_ifindex).ipaddr = ina->sin_addr;

	/* Get netmask of interface... */
	ina = get_if_info(iface, SIOCGIFNETMASK);
	if (ina == NULL)
	    exit(-1);

	DEV_IFINDEX(ifreq.ifr_ifindex).netmask = ina->sin_addr;

	ina = get_if_info(iface, SIOCGIFBRDADDR);
	if (ina == NULL)
	    exit(-1);

	DEV_IFINDEX(ifreq.ifr_ifindex).broadcast = ina->sin_addr;

	DEV_IFINDEX(ifreq.ifr_ifindex).enabled = 1;

	if (this_host.nif >= MAX_NR_INTERFACES)
	    break;

    } while ((iface = strtok(NULL, ",")));

    close(if_sock);

    /* Load kernel modules */
    load_modules(ifnames);

    /* Enable IP forwarding and set other kernel options... */
    if (set_kernel_options() < 0) {
	fprintf(stderr, "Could not set kernel options!\n");
	exit(-1);
    }
}

/* This signal handler ensures clean exits */
void signal_handler(int type)
{

    switch (type) {
    case SIGSEGV:
	alog(LOG_ERR, 0, __FUNCTION__, "SEGMENTATION FAULT!!!! Exiting!!! "
	     "To get a core dump, compile with DEBUG option.");
    case SIGINT:
    case SIGHUP:
    case SIGTERM:
    default:
	exit(0);
    }
}

int main(int argc, char **argv)
{
    static char *ifname = NULL;	/* Name of interface to attach to */
    fd_set rfds, readers;
    int n, nfds = 0, i;
    int daemonize = 0;
    struct timeval *timeout;

    /* Remember the name of the executable... */
    progname = strrchr(argv[0], '/');

    if (progname)
	progname++;
    else
	progname = argv[0];

    /* Use debug output as default */
    debug = 1;

    /* Parse command line: */

    while (1) {
	int opt;

	opt = getopt_long(argc, argv, "i:fjln:dghoq:r:s:uwxDLRV", longopts, 0);

	if (opt == EOF)
	    break;

	switch (opt) {
	case 0:
	    break;
	case 'd':
	    debug = 0;
	    daemonize = 1;
	    break;
	case 'f':
	    llfeedback = 1;
	    active_route_timeout = ACTIVE_ROUTE_TIMEOUT_LLF;
	    break;
	case 'g':
	    rreq_gratuitous = !rreq_gratuitous;
	    break;
	case 'i':
	    ifname = optarg;
	    break;
	case 'j':
	    hello_jittering = !hello_jittering;
	    break;
	case 'l':
	    log_to_file = !log_to_file;
	    break;
	case 'n':
	    if (optarg && isdigit(*optarg)) {
		receive_n_hellos = atoi(optarg);
		if (receive_n_hellos < 2) {
		    fprintf(stderr, "-n should be at least 2!\n");
		    exit(-1);
		}
	    }
	    break;
	case 'o':
	    optimized_hellos = !optimized_hellos;
	    break;
	case 'q':
	    if (optarg && isdigit(*optarg))
		qual_threshold = atoi(optarg);
	    break;
	case 'r':
	    if (optarg && isdigit(*optarg))
		rt_log_interval = atof(optarg) * 1000;
	    break;
	case 'u':
	    unidir_hack = !unidir_hack;
	    break;
	case 'w':
	    internet_gw_mode = !internet_gw_mode;
	    break;
	case 'x':
	    expanding_ring_search = !expanding_ring_search;
	    break;
	case 'L':
	    local_repair = !local_repair;
	    break;
	case 'D':
	    wait_on_reboot = !wait_on_reboot;
	    break;
	case 'R':
	    ratelimit = !ratelimit;
	    break;
	case 'V':
	    printf
		("\nAODV-UU v%s, %s © Uppsala University & Ericsson AB.\nAuthor: Erik Nordström, <erik.nordstrom@it.uu.se>\n\n",
		 AODV_UU_VERSION, DRAFT_VERSION);
	    exit(0);
	    break;
	case '?':
	case ':':
	    exit(0);
	default:
	    usage(0);
	}
    }
    /* Check that we are running as root */
    if (geteuid() != 0) {
	fprintf(stderr, "must be root\n");
	exit(1);
    }

    /* Detach from terminal */
    if (daemonize) {
	if (fork() != 0)
	    exit(0);
	/* Close stdin, stdout and stderr... */
	/*  close(0); */
	close(1);
	close(2);
	setsid();
    }
    /* Make sure we cleanup at exit... */
    atexit((void *) &cleanup);

    /* Initialize data structures and services... */
    rt_table_init();
    log_init();
    /*   packet_queue_init(); */
    host_init(ifname);
    /*   packet_input_init(); */
    nl_init();
    nl_send_conf_msg();
    aodv_socket_init();
#ifdef LLFEEDBACK
    if (llfeedback) {
	llf_init();
    }
#endif

    /* Catch SIGHUP, SIGINT and SIGTERM type signals */
    signal(SIGHUP, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Only capture segmentation faults when we are not debugging... */
#ifndef DEBUG
    signal(SIGSEGV, signal_handler);
#endif
    /* Set sockets to watch... */
    FD_ZERO(&readers);
    for (i = 0; i < nr_callbacks; i++) {
	FD_SET(callbacks[i].fd, &readers);
	if (callbacks[i].fd >= nfds)
	    nfds = callbacks[i].fd + 1;
    }

    /* Set the wait on reboot timer... */
    if (wait_on_reboot) {
	timer_init(&worb_timer, wait_on_reboot_timeout, &wait_on_reboot);
	timer_set_timeout(&worb_timer, DELETE_PERIOD);
	alog(LOG_NOTICE, 0, __FUNCTION__,
	     "In wait on reboot for %d milliseconds. Disable with \"-D\".",
	     DELETE_PERIOD);
    }

    /* Schedule the first Hello */
    if (!optimized_hellos && !llfeedback)
	hello_start();

    if (rt_log_interval)
	log_rt_table_init();

    while (1) {
	memcpy((char *) &rfds, (char *) &readers, sizeof(rfds));

	timeout = timer_age_queue();

	if ((n = select(nfds, &rfds, NULL, NULL, timeout)) < 0) {
	    if (errno != EINTR)
		alog(LOG_WARNING, errno, __FUNCTION__,
		     "Failed select (main loop)");
	    continue;
	}

	if (n > 0) {
	    for (i = 0; i < nr_callbacks; i++) {
		if (FD_ISSET(callbacks[i].fd, &rfds)) {
		    /* We don't want any timer SIGALRM's while executing the
		       callback functions, therefore we block the timer... */
		    (*callbacks[i].func) (callbacks[i].fd);
		}
	    }
	}
    }				/* Main loop */
    return 0;
}

static void cleanup(void)
{
    DEBUG(LOG_DEBUG, 0, "CLEANING UP!");
    remove_modules();
    rt_table_destroy();
    /*  packet_input_cleanup(); */
/*     packet_queue_destroy(); */
    aodv_socket_cleanup();
    nl_cleanup();
#ifdef LLFEEDBACK
    if (llfeedback)
	llf_cleanup();
#endif
    log_cleanup();
}
