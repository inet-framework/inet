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

#ifndef __ICMP_SOCKET_H__
#define __ICMP_SOCKET_H__

#ifndef NS_NO_GLOBALS

#include "inet/routing/extras/dymo/dymoum/defs_dymo.h"
#ifdef OMNETPP
#include "inet/routing/extras/base/compatibility.h"
#else
#include <netinet/in.h>
#include <sys/types.h>

#endif
#define ICMP_ECHOREPLY_SIZE 8
#define ICMP_SEND_BUF_SIZE  4096
#define ICMP_RECV_BUF_SIZE  4096

#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

void icmp_socket_init(void);
void icmp_socket_fini(void);
u_short in_cksum(u_short *icmp, int len);
void icmp_reply_send(struct in_addr dest_addr, struct dev_info *dev);

#ifdef NS_PORT
void icmp_process(struct in_addr ip_src);
#endif  /* NS_PORT */

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __ICMP_SOCKET_H__ */
