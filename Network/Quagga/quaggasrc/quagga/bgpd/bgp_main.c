/* Main routine of bgpd.
   Copyright (C) 1996, 97, 98, 1999 Kunihiro Ishiguro

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#include <zebra.h>

#include "vector.h"
#include "vty.h"
#include "command.h"
#include "getopt.h"
#include "thread.h"
#include <lib/version.h>
#include "memory.h"
#include "prefix.h"
#include "log.h"
#include "privs.h"
#include "sigevent.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_mplsvpn.h"

/* bgpd options, we use GNU getopt library. */
static struct option longopts[] = 
{
  { "daemon",      no_argument,       NULL, 'd'},
  { "config_file", required_argument, NULL, 'f'},
  { "pid_file",    required_argument, NULL, 'i'},
  { "bgp_port",    required_argument, NULL, 'p'},
  { "vty_addr",    required_argument, NULL, 'A'},
  { "vty_port",    required_argument, NULL, 'P'},
  { "retain",      no_argument,       NULL, 'r'},
  { "no_kernel",   no_argument,       NULL, 'n'},
  { "user",        required_argument, NULL, 'u'},
  { "group",       required_argument, NULL, 'g'},
  { "version",     no_argument,       NULL, 'v'},
  { "help",        no_argument,       NULL, 'h'},
  { 0 }
};

/* signal definitions */
static void sighup (void);
static void sigint (void);
static void sigusr1 (void);

struct quagga_signal_t bgp_signals[] = 
{
  { 
    SIGHUP, 
    &sighup,
    0
  },
  {
    SIGUSR1,
    &sigusr1,
    0
  },
  {
    SIGINT,
    &sigint,
    0
  },
  {
    SIGTERM,
    &sigint,
    0
  },
};

/* Configuration file and directory. */
static char config_default[] = SYSCONFDIR BGP_DEFAULT_CONFIG;

/* Route retain mode flag. */
int retain_mode_bgpd ;

/* Master of threads. */
struct thread_master *master_bgpd;

/* Manually specified configuration file name.  */
char *config_file_bgpd ;

/* Process ID saved for use by init system */
const char *pid_file_bgpd ;

/* VTY port number and address.  */
int vty_port_bgpd ;
char *vty_addr_bgpd ;

/* privileges */
static zebra_capabilities_t _caps_p [] =  
{
    ZCAP_BIND, 
    ZCAP_RAW,
};

struct zebra_privs_t bgpd_privs =
{
  _caps_p,
  NULL,
  sizeof(_caps_p)/sizeof(_caps_p[0]),
  0,
#if defined(QUAGGA_USER) && defined(QUAGGA_GROUP)
  QUAGGA_USER,
  QUAGGA_GROUP,
#else
  NULL,
  NULL,
#endif
#ifdef VTY_GROUP
  VTY_GROUP,
#else
  NULL,
#endif
  NULL,
  NULL
};

/* Help information display. */
static void
usage (char *progname, int status)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", progname);
  else
    {    
      printf ("Usage : %s [OPTION...]\n\n\
Daemon which manages kernel routing table management and \
redistribution between different routing protocols.\n\n\
-d, --daemon       Runs in daemon mode\n\
-f, --config_file  Set configuration file name\n\
-i, --pid_file     Set process identifier file name\n\
-p, --bgp_port     Set bgp protocol's port number\n\
-A, --vty_addr     Set vty's bind address\n\
-P, --vty_port     Set vty's port number\n\
-r, --retain       When program terminates, retain added route by bgpd.\n\
-n, --no_kernel    Do not install route to kernel.\n\
-u, --user         User to run as\n\
-g, --group        Group to run as\n\
-v, --version      Print program version\n\
-h, --help         Display this help and exit\n\
\n\
Report bugs to %s\n", progname, ZEBRA_BUG_ADDRESS);
    }

  exit (status);
}

/* SIGHUP handler. */
static void 
sighup (void)
{
  zlog (NULL, LOG_INFO, "SIGHUP received");

  /* Terminate all thread. */
  bgp_terminate ();
  bgp_reset ();
  zlog_info ("bgpd restarting!");

  /* Reload config file. */
  vty_read_config (config_file, config_default);

  /* Create VTY's socket */
  vty_serv_sock (vty_addr, vty_port, BGP_VTYSH_PATH);

  /* Try to return to normal operation. */
}

/* SIGINT handler. */
static void
sigint (void)
{
  zlog_notice ("Terminating on signal");

  if (! retain_mode)
    bgp_terminate ();

  exit (0);
}

/* SIGUSR1 handler. */
static void
sigusr1 (void)
{
  zlog_rotate (NULL);
}

/* Main routine of bgpd. Treatment of argument and start bgp finite
   state machine is handled at here. */
int
bgpd_main_entry (int argc, char **argv)
{
  char *p;
  int opt;
  int daemon_mode = 0;
  char *progname;
  struct thread thread;

  /* Set umask before anything for security */
  umask (0027);

  /* Preserve name of myself. */
  progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);

  zlog_default = openzlog (progname, ZLOG_BGP,
			   LOG_CONS|LOG_NDELAY|LOG_PID, LOG_DAEMON);

  /* BGP master init. */
  bgp_master_init ();

  /* Command line argument treatment. */
  while (1) 
    {
      opt = getopt_long (argc, argv, "df:i:hp:A:P:rnu:g:v", longopts, 0);
    
      if (opt == EOF)
	break;

      switch (opt) 
	{
	case 0:
	  break;
	case 'd':
	  daemon_mode = 1;
	  break;
	case 'f':
	  config_file = optarg;
	  break;
        case 'i':
          pid_file = optarg;
          break;
	case 'p':
	  bm->port = atoi (optarg);
	  break;
	case 'A':
	  vty_addr = optarg;
	  break;
	case 'P':
          /* Deal with atoi() returning 0 on failure, and bgpd not
             listening on bgp port... */
          if (strcmp(optarg, "0") == 0) 
            {
              vty_port = 0;
              break;
            } 
          vty_port = atoi (optarg);
          vty_port = (vty_port ? vty_port : BGP_VTY_PORT);
	  break;
	case 'r':
	  retain_mode = 1;
	  break;
	case 'n':
	  bgp_option_set (BGP_OPT_NO_FIB);
	  break;
	case 'u':
	  bgpd_privs.user = optarg;
	  break;
	case 'g':
	  bgpd_privs.group = optarg;
	  break;
	case 'v':
	  print_version (progname);
	  exit (0);
	  break;
	case 'h':
	  usage (progname, 0);
	  break;
	default:
	  usage (progname, 1);
	  break;
	}
    }

  /* Make thread master. */
  master = bm->master__item;

  /* Initializations. */
  srand (time (NULL));
  signal_init (master, Q_SIGC(bgp_signals), bgp_signals);
  zprivs_init (&bgpd_privs);
  cmd_init (1);
  vty_init (master);
  memory_init ();

  /* BGP related initialization.  */
  bgp_init ();

  /* Sort CLI commands. */
  sort_node ();

  /* Parse config file. */
  vty_read_config (config_file, config_default);

  /* Turn into daemon if daemon_mode is set. */
  if (daemon_mode)
    daemon (0, 0);

  /* Process ID file creation. */
  pid_output (pid_file);

  /* Make bgp vty socket. */
  vty_serv_sock (vty_addr, vty_port, BGP_VTYSH_PATH);

  /* Print banner. */
  zlog_notice ("BGPd %s starting: vty@%d, bgp@%d", QUAGGA_VERSION,
	       vty_port, bm->port);

  /* Start finite state machine, here we go! */
  while (thread_fetch (master, &thread))
    thread_call (&thread);

  /* Not reached. */
  exit (0);
}
