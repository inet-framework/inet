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

#ifndef __DYMO_SOCKET_H__
#define __DYMO_SOCKET_H__

#ifndef NS_NO_GLOBALS

#include "defs_dymo.h"
#include "dymo_generic.h"
#include "dymo_re.h"
#ifndef OMNETPP
#include <netinet/in.h>
#else
#include "compatibility.h"
#endif
/* We are allowing a maximum DYMO message size of a RE message
   which advertises MAX_RE_BLOCKS RENodeAddresses */
#define DYMO_MSG_MAX_SIZE   RE_SIZE
#define RECV_BUF_SIZE       DYMO_MSG_MAX_SIZE
#define SEND_BUF_SIZE       DYMO_MSG_MAX_SIZE

#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

/* Used to limit rate of sending DYMO messages */
#ifdef OMNETPP
struct timeval dymo_rate[50 - 1];
#else
struct timeval dymo_rate[DYMO_RATELIMIT - 1];
#endif
int num_dymo_msgs;

/* Creates and configures an UDP socket for each enabled interface */
void dymo_socket_init();

/* Closes all sockets */
void dymo_socket_fini();

/* Prepares sending buffer for a new element */
#ifndef OMNETPP
DYMO_element *dymo_socket_new_element();
#endif

/* Queues an element before it is sent */
DYMO_element *dymo_socket_queue(DYMO_element *e);

/* Sends queued DYMO element */
void dymo_socket_send(struct in_addr dest_addr, struct dev_info *dev,double delay=-1);

#ifdef NS_PORT
/* Process a received DYMO packet */

#ifndef OMNETPP
void recv_dymoum_pkt(Packet *p);
#else
void recv_dymoum_pkt(DYMO_element *e,const struct in_addr &src_addr,int);
#endif

#endif  /* NS_PORT */

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __DYMO_SOCKET_H__ */

