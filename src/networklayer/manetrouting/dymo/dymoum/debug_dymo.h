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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifndef NS_NO_GLOBALS
#include "defs_dymo.h"
/* win 32 code */
#ifndef _WIN32
#include <syslog.h>
#else
#define LOG_DEBUG 0
#define LOG_NOTICE 0
#define LOG_ERR 0
#define LOG_WARNING 0
#define LOG_INFO 0
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef OMNETPP
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#else
#include "compatibility.h"
#endif
#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

/* Open DYMOUM log */
void dlog_init();

/* Close DYMOUM log */
void dlog_fini();

/* Log a message given its priority, error number (if needed) and the function
   where it occurred */
void dlog(int pri, int errnum, const char *func,const char *format, ...);

/* Return a string representing a given IP address */
#ifdef OMNETPP
const char *ip2str(Uint128 &ipaddr);
#else
char *ip2str(u_int32_t ipaddr);
#endif

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __DEBUG_H__ */
