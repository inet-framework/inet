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

#ifndef __PENDING_RREQ_H__
#define __PENDING_RREQ_H__

#ifndef NS_NO_GLOBALS

#include "defs_dymo.h"
#include "dlist.h"
#include "timer_queue.h"
#ifndef OMNETPP
#include <netinet/in.h>
#include <sys/types.h>
#else
#include "compatibility.h"
#endif

/* Here we maintain a list of those RREQs which haven't been answered with a
   RREP yet */

typedef struct pending_rreq
{
    dlist_head_t    list_head;
    struct in_addr  dest_addr;
    u_int32_t   seqnum;
    u_int8_t    tries;
    struct timer    timer;
} pending_rreq_t;

#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

/* Add a new entry to the list */
pending_rreq_t *pending_rreq_add(struct in_addr dest_addr, u_int32_t seqnum);

/* Remove an entry from the list */
int pending_rreq_remove(pending_rreq_t *entry);

/* Find an entry in the list with the given destination address */
pending_rreq_t *pending_rreq_find(struct in_addr dest_addr);

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __PENDING_RREQ_H__ */
