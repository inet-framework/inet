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

#ifndef __DLIST_H__
#define __DLIST_H__

#include "inet/routing/extras/dymo/dymoum/defs_dymo.h"

namespace inet {

namespace inetmanet {

/* Doubly linked list based on the Linux kernel implementation */

#define DLIST_SUCCESS   0
#define DLIST_FAILURE   -1

typedef struct dlist_head
{
    struct dlist_head *next, *prev;
} dlist_head_t;

#define DLIST_HEAD_INIT(name) { &(name), &(name) }

#define DLIST_HEAD(name) \
    struct dlist_head name = DLIST_HEAD_INIT(name)

#define INIT_DLIST_ELEM(e) do { \
    (e)->prev = nullptr; (e)->next = nullptr; \
} while (0)

#define INIT_DLIST_HEAD(h) do { \
    (h)->prev = (h); (h)->next = (h); \
} while (0)

/* Internal use only */
static inline void __dlist_add(struct dlist_head *n,
                               struct dlist_head *prev,
                               struct dlist_head *next)
{
    next->prev = n;
    n->next = next;
    n->prev = prev;
    prev->next = n;
}

/* Useful for implementing stacks */
static inline int dlist_add(struct dlist_head *n, struct dlist_head *head)
{
    if (n && head)
    {
        __dlist_add(n, head, head->next);
        return DLIST_SUCCESS;
    }
    return DLIST_FAILURE;
}

/* Useful for implementing queues */
static inline int dlist_add_tail(struct dlist_head *n, struct dlist_head *head)
{
    if (n && head)
    {
        __dlist_add(n, head->prev, head);
        return DLIST_SUCCESS;
    }
    return DLIST_FAILURE;
}

/* Isn't the element attached to a list? */
static inline int dlist_unattached(struct dlist_head *head)
{
    return (head->prev == nullptr && head->next == nullptr);
}

/* Internal use only */
static inline void __dlist_del(struct dlist_head * prev, struct dlist_head * next)
{
#ifdef OMNETPP
    if (next == (dlist_head *)nullptr)
        throw cRuntimeError(" __dlist_del next == nullptr");
    if (prev == (dlist_head *)nullptr)
        throw cRuntimeError(" __dlist_del prev == nullptr");
#endif
    next->prev = prev;
    prev->next = next;
}

/* Dettaches the entry from the list */
static inline int dlist_del(struct dlist_head *entry)
{
    if (entry)
    {
        __dlist_del(entry->prev, entry->next);
        entry->next = (dlist_head *)nullptr;
        entry->prev = (dlist_head *)nullptr;
        return DLIST_SUCCESS;
    }
    return DLIST_FAILURE;
}

/* Is an empty list? */
static inline int dlist_empty(const struct dlist_head *head)
{
    return (head->next == head);
}

/* Iterate over the list */
#define dlist_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

/* Iterate over the list when you want to delete some element(s) during the
   loop */
#define dlist_for_each_safe(pos, n, head) \
        for (pos = (head)->next, n = pos->next; pos != (head); \
                pos = n, n = pos->next)

} // namespace inetmanet

} // namespace inet

#endif  /* __DLIST_H__ */

