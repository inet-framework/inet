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
#define NS_PORT
#define OMNETPP
#ifdef NS_PORT
#ifndef OMNETPP
#include "ns/dymo_um.h"
#else
#include "../dymo_um_omnet.h"
#endif
#else
#include "inet/routing/extras/dymo/dymoum/debug_dymo.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>


extern int debug, daemonize;
#endif  /* NS_PORT */

namespace inet {

namespace inetmanet {

void NS_CLASS dlog_init()
{
#ifndef NS_PORT
    int option = 0;

    if (debug && !daemonize)
        option = LOG_PERROR;

    openlog("dymod", option, LOG_DAEMON);
#endif  /* NS_PORT */
}

void NS_CLASS dlog_fini()
{
#ifndef NS_PORT
    closelog();
#endif  /* NS_PORT */
}

void NS_CLASS dlog(int pri, int errnum, const char *func, const char *format, ...)
{
    va_list ap;
    char msg[1024];

    return;

    memset(msg, 0, sizeof(msg));

    va_start(ap, format);
    vsprintf(msg, format, ap);
    va_end(ap);

#ifndef NS_PORT
    if (errnum != 0)
        syslog(pri, "%s: %s: %s", func, msg, strerror(errnum));
    else
        syslog(pri, "%s: %s", func, msg);
#else
#ifndef OMNETPP
    debug("node %s: %s: %s\n", ip2str(ra_addr_), func, msg);
#else
    EV_DEBUG << "node "<< nodeName << " function " << func << "   " << msg << "\n";
#endif
#endif
}
#ifdef OMNETPP
const char *NS_CLASS ip2str(L3Address &ipaddr)
{
    return ipaddr.str().c_str();
}

#else
char *NS_CLASS ip2str(u_int32_t ipaddr)
{
#ifndef NS_PORT
    struct in_addr addr;

    addr.s_addr = ipaddr;
    return inet_ntoa(addr);
#else
return L3Address::getInstance().print_nodeaddr(ipaddr);
#endif
}
#endif

} // namespace inetmanet

} // namespace inet

