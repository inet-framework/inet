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

#ifndef __DYMO_NB_H__
#define __DYMO_NB_H__

#ifndef NS_NO_GLOBALS
#include "inet/routing/extras/dymo/dymoum/dlist.h"
#include "inet/routing/extras/dymo/dymoum/timer_queue.h"
#include <sys/types.h>

namespace inet {

namespace inetmanet {

/* Manage neighborhood connectivity. This information is acquired via HELLO
   messages. */

#define NB_TIMEOUT  2*1000*hello_ival

typedef struct nb
{
    dlist_head_t    list_head;
    struct in_addr  nb_addr;
    u_int32_t   ifindex;
    struct timer    timer;
} nb_t;

} // namespace inetmanet

} // namespace inet

#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

/* Add a new entry to the list */
nb_t *nb_insert(struct in_addr nb_addr, u_int32_t ifindex);

/* Update an existing nb_t entry */
void nb_update(nb_t *nb);

/* Remove an entry from the list */
int nb_remove(nb_t *nb);

/* Find an entry in the list with the given address and ifindex */
nb_t *nb_find(struct in_addr nb_addr, u_int32_t ifindex);

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __DYMO_NB_H__ */

