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

#ifndef __BLACKLIST_H__
#define __BLACKLIST_H__

#ifndef NS_NO_GLOBALS

#include "dlist.h"
#include "timer_queue.h"

#ifndef OMNETPP
#include <netinet/in.h>
#include <sys/types.h>
#else
#include "compatibility.h"
#endif

/* Here we maintain a list of those next hops which didn't reply with a unicast
   packet when S-bit was enabled in a RREP. We won't forward RREQs which come
   from these nodes. */

#define BLACKLIST_TIMEOUT   5000

typedef struct blacklist
{
    dlist_head_t    list_head;
    struct in_addr  addr;
    struct timer    timer;
} blacklist_t;

#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

/* Add a new entry to the list */
blacklist_t *blacklist_add(struct in_addr addr);

/* Remove an entry from the list */
int blacklist_remove(blacklist_t *entry);

/* Find an entry in the list with the given address */
blacklist_t *blacklist_find(struct in_addr addr);

void blacklist_erase();

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __BLACKLIST_H__ */
