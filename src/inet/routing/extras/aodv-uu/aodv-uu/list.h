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
 * Authors: Erik Nordstr�m, <erik.nordstrom@it.uu.se>
 *
 *****************************************************************************/
#ifndef _LIST_AODV_H
#define _LIST_AODV_H
#define NS_PORT
#define OMNETPP

namespace inet {

namespace inetmanet {

/* Simple linked list inspired from the Linux kernel list implementation */
typedef struct list_t
{
    struct list_t *prev, *next;
} list_t;

#define LIST_NULL -1
#define LIST_SUCCESS 1

#define LIST(name) list_t name = { &(name), &(name) }

#define INIT_LIST_HEAD(h) do { \
    (h)->next = (h); (h)->prev = (h); \
} while (0)

#define INIT_LIST_ELM(le) do { \
    (le)->next = nullptr; (le)->prev = nullptr; \
} while (0)

int list_detach(list_t * le);
int list_add_tail(list_t * head, list_t * le);
int list_add(list_t * head, list_t * le);


#define list_foreach(curr, head) \
        for (curr = (head)->next; curr != (head); curr = curr->next)

#define list_foreach_safe(pos, tmp, head) \
        for (pos = (head)->next, tmp = pos->next; pos != (head); \
                pos = tmp, tmp = pos->next)
#ifndef OMNETPP
#define list_empty(head) ((head) == (head)->next)

#define list_first(head) ((head)->next)

#define list_unattached(le) ((le)->next == nullptr && (le)->prev == nullptr)
#else

int list_empty(list_t*);
list_t* list_first(list_t*);
int list_unattached(list_t*);

#endif

} // namespace inetmanet

} // namespace inet

#endif

