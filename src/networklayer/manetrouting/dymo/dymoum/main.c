/***************************************************************************
 *   Copyright (C) 2005 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "defs.h"
#include "debug.h"
#include "rtable.h"
#include "timer_queue.h"
#include "dymo_socket.h"
#include "icmp_socket.h"
#include "dymo_netlink.h"
#include "dymo_hello.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <net/if.h>


#define MAX_CALLBACKS 9

static struct callback
{
	int		fd;
	callback_func_t	func;
} callbacks[MAX_CALLBACKS];

static int num_callbacks = 0;

char *progname		= NULL;
int debug		= 0;
int daemonize		= 0;
int no_path_acc		= 0;
int reissue_rreq	= 0;
int s_bit		= 0;
int hello_ival		= 0;


struct option longopts[] = {
	{"interface", required_argument, NULL, 'i'},
	{"debug", no_argument, NULL, 'v'},
	{"daemon", no_argument, NULL, 'd'},
	{"no_path_acc", no_argument, NULL, 'n'},
	{"reissue_rreq", no_argument, NULL, 'r'},
	{"s_bit", no_argument, NULL, 's'},
	{"hello", required_argument, NULL, 'm'},
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'V'},
	{0}
};


void usage()
{
	fprintf(stdout,
		"\nUsage: %s [-vdnrshV] [-m <ival>] -i <if0,if1,...>\n\n"
		"-v, --debug           Verbose output (for debugging purposes).\n"
		"-d, --daemon          Daemon mode.\n"
		"-n, --no_path_acc     Do not perform path accumulation.\n"
		"-r, --reissue_rreq    Retry a route discovery when a previous one did not success.\n"
		"-s, --s_bit           Set S-bit on RREPs to avoid unidirectional links.\n"
		"-m, --hello           Monitor link status with periodic HELLO messages every <ival> sec.\n"
		"-h, --help            Show this help and exit.\n"
		"-V, --version         Show version.\n\n"
		"Francisco J. Ros, <fjrm@dif.um.es>\n\n", progname);
}

int set_kernel_options()
{
	int i, fd;
	char command[64];
	char on		= '1';
	char off	= '0';
	
	// Enable IP forwarding
	if ((fd = open("/proc/sys/net/ipv4/ip_forward", O_WRONLY)) < 0)
		return -1;
	if (write(fd, &on, sizeof(char)) < 0)
		return -1;
	close(fd);
	
	// Deactivate route cache
	if ((fd = open("/proc/sys/net/ipv4/route/max_delay", O_WRONLY)) < 0)
		return -1;
	if (write(fd, &off, sizeof(char)) < 0)
		return -1;
	close(fd);
	
	// Deactivate route cache
	if ((fd = open("/proc/sys/net/ipv4/route/min_delay", O_WRONLY)) < 0)
		return -1;
	if (write(fd, &off, sizeof(char)) < 0)
		return -1;
	close(fd);
	
	// AODVUU disables ICMP redirects on all interfaces. I guess that this
	// may be also useful for DYMOUM in some scenarios, but I haven't
	// thought very much about this. I leave this code here...
	for (i = 0; i < DYMO_MAX_NR_INTERFACES; i++)
	{
		if (!DEV_NR(i).enabled)
			continue;
		
		memset(command, '\0', 64);
		sprintf(command, "/proc/sys/net/ipv4/conf/%s/send_redirects",
			DEV_NR(i).ifname);
		if ((fd = open(command, O_WRONLY)) < 0)
			return -1;
		if (write(fd, &off, sizeof(char)) < 0)
			return -1;
		close(fd);
		
		memset(command, '\0', 64);
		sprintf(command, "/proc/sys/net/ipv4/conf/%s/accept_redirects",
			DEV_NR(i).ifname);
		if ((fd = open(command, O_WRONLY)) < 0)
			return -1;
		if (write(fd, &off, sizeof(char)) < 0)
			return -1;
		close(fd);
	}
	memset(command, '\0', 64);
	sprintf(command, "/proc/sys/net/ipv4/conf/all/send_redirects");
	if ((fd = open(command, O_WRONLY)) < 0)
		return -1;
	if (write(fd, &off, sizeof(char)) < 0)
		return -1;
	close(fd);
	
	return 0;
}

int attach_callback_func(int fd, callback_func_t func)
{
	if (num_callbacks >= MAX_CALLBACKS)
		return -1;
	
	callbacks[num_callbacks].fd	= fd;
	callbacks[num_callbacks].func	= func;
	num_callbacks++;
	
	return 0;
}

void signal_handler(int type)
{
	switch (type)
	{
		case SIGSEGV:
			dlog(LOG_ERR, 0, __FUNCTION__, "segmentation fault signal received");
			exit(EXIT_FAILURE);
			break;
		default:
			exit(EXIT_SUCCESS);
	}
}

void get_if_info(struct ifreq *ifreq, char *ifname, int type)
{
	int sock;
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		dlog(LOG_ERR, errno, __FUNCTION__,
			"could not open a socket to obtain interfaces information");
		exit(EXIT_FAILURE);
	}
	
	memset(ifreq, 0, sizeof(struct ifreq));
	strncpy(ifreq->ifr_name, ifname, IFNAMSIZ * sizeof(char));
	if (ioctl(sock, type, ifreq) < 0)
	{
		dlog(LOG_ERR, errno, __FUNCTION__,
			"could not get interface information for %s",
			ifname);
		close(sock);
		exit(EXIT_FAILURE);
	}
	
	close(sock);
}

void load_modules(char *ifnames)
{
	struct stat st;
	char buf[1024], *l = NULL;
	int found = 0;
	FILE *m;
	
	memset(buf, '\0', 64);
	
	if (stat("./kdymo.ko", &st) == 0)
		sprintf(buf, "/sbin/insmod kdymo.ko ifnames=%s &>/dev/null", ifnames);
	else if (stat("./kdymo.o", &st) == 0)
		sprintf(buf, "/sbin/insmod kdymo.o ifnames=%s &>/dev/null", ifnames);
	else
		sprintf(buf, "/sbin/modprobe kdymo ifnames=%s &>/dev/null", ifnames);
	
	system(buf);
	
	usleep(100000);
	
	/* Check result */
	m = fopen("/proc/modules", "r");
	while (fgets(buf, sizeof(buf), m))
	{
		l = strtok(buf, " \t");
		if (!strcmp(l, "kdymo"))
			found++;
	}
	fclose(m);
	
	if (found < 1)
	{
		fprintf(stderr, "A kernel module could not be loaded, "
			"check your installation... %d\n", found);
		exit(EXIT_FAILURE);
	}
}

void remove_modules(void)
{
    system("/sbin/rmmod kdymo &>/dev/null");
}

void host_init(char *ifnames)
{
	char *ifname, ifnames_aux[(IFNAMSIZ + 1) * DYMO_MAX_NR_INTERFACES];
	struct ifreq ifreq;
	
	if (!ifnames)
	{
		usage();
		exit(EXIT_SUCCESS);
	}
	
	memset(&this_host, 0, sizeof(struct host_info));
	memset(dev_indices, 0, DYMO_MAX_NR_INTERFACES * sizeof(u_int32_t));
	
	// Initialize this_host
	this_host.nif		= 0;
	this_host.seqnum	= 1;
	this_host.prefix	= 0;
	this_host.is_gw		= 0;
	
	// Get interfaces information
	strncpy(ifnames_aux, ifnames, sizeof(ifnames_aux));
	ifname = strtok(ifnames_aux, ",");
	do
	{
		u_int32_t ifindex;
		
		// Get interface index
		get_if_info(&ifreq, ifname, SIOCGIFINDEX);
		ifindex = ifreq.ifr_ifindex;
		this_host.devs[this_host.nif].ifindex	= ifindex;
		dev_indices[this_host.nif]		= ifindex;
		this_host.nif++;
		
		// Copy interface name
		strncpy(DEV_IFINDEX(ifindex).ifname, ifname,
			IFNAMSIZ * sizeof(char));
		
		// Get IP address
		get_if_info(&ifreq, ifname, SIOCGIFADDR);
		DEV_IFINDEX(ifindex).ipaddr.s_addr =
			((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr.s_addr;
		
		// Get broadcast address
		get_if_info(&ifreq, ifname, SIOCGIFBRDADDR);
		DEV_IFINDEX(ifindex).bcast.s_addr =
			((struct sockaddr_in *) &ifreq.ifr_broadaddr)->sin_addr.s_addr;
		
		// Enable interface
		DEV_IFINDEX(ifindex).enabled = 1;
		
		if (this_host.nif >= DYMO_MAX_NR_INTERFACES)
			break;
	} while ((ifname = strtok(NULL, ",")) != NULL);
	
	// Load kernel modules
	load_modules(ifnames_aux);
	
	// Set appropriate kernel options
	if (set_kernel_options() < 0)
	{
		dlog(LOG_ERR, errno, __FUNCTION__, "could not set kernel options");
		exit(EXIT_FAILURE);
	}
}

void host_fini(void)
{
	remove_modules();
	rtable_destroy();
	netlink_fini();
	dymo_socket_fini();
	icmp_socket_fini();
	hello_fini();
	dlog_fini();
}

/* Main loop */
int main(int argc, char *argv[])
{
	struct timeval *timeout;
	char *ifnames = NULL;
	fd_set readers, rfds;
	int i, n, nfds = 0;
	
	progname = argv[0];
	
	// Get command line options
	while (1)
	{
		int opt;
		
		opt = getopt_long(argc, argv, "i:vdnrm:shV", longopts, 0);
		if (opt == -1)
			break;
		
		switch (opt)
		{
			case 'i':
				ifnames = optarg;
				break;
			
			case 'v':
				debug = 1;
				break;
				
			case 'd':
				daemonize = 1;
				break;
			
			case 'n':
				no_path_acc = 1;
				break;
			
			case 'r':
				reissue_rreq = 1;
				break;
			
			case 's':
				s_bit = 1;
				break;
			
			case 'm':
				hello_ival = atoi(optarg);
				break;
			
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
				break;
			case 'V':
				fprintf(stdout, "\nDYMOUM v%s, %s - University of Murcia (Spain)\n"
					"Francisco J. Ros <fjrm@dif.um.es>\n\n",
					DYMO_UM_VERSION,
					DYMO_DRAFT_VERSION);
				exit(EXIT_SUCCESS);
				break;
			
			case '?':
			case ':':
			default:
				usage();
				exit(EXIT_SUCCESS);
				break;
		}
	}
	
	// Check we are root
	if (geteuid() != 0)
	{
		fprintf(stderr, "You must be root\n");
		exit(EXIT_FAILURE);
	}
	
	// Daemonize
	if (daemonize)
	{
		// Create a child process, parent dies
		if (fork() != 0)
			exit(EXIT_SUCCESS);
		
		// Close stdin, stdout and stderr
		close(0);
		close(1);
		close(2);
		
		// Process is a group leader
		setsid();
	}
	
	// Make a clean exit
	atexit(host_fini);
	
	// Initialize data structures and services
	rtable_init();
	dlog_init();
	host_init(ifnames);
	netlink_init();
	dymo_socket_init();
	icmp_socket_init();
	hello_init();
	
	// Catch signals
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	//signal(SIGSEGV, signal_handler);
	
	FD_ZERO(&readers);
	for (i = 0; i < num_callbacks; i++)
	{
		FD_SET(callbacks[i].fd, &readers);
		if (callbacks[i].fd >= nfds)
			nfds = callbacks[i].fd + 1;
	}
	
	// Main loop
	while (1)
	{
		memcpy((char *) &rfds, (char *) &readers, sizeof(rfds));
		
		timeout = timer_age_queue();
		
		if ((n = select(nfds, &rfds, NULL, NULL, timeout)) < 0)
		{
			if (errno != EINTR)
				dlog(LOG_WARNING, errno, __FUNCTION__, "failed select() in main loop");
			continue;
		}
		
		if (n > 0)
		{
			for (i = 0; i < num_callbacks; i++)
				if (FD_ISSET(callbacks[i].fd, &rfds))
					callbacks[i].func(callbacks[i].fd);
		}
	}
	
	return 0;
}
