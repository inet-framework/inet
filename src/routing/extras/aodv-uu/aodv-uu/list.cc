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
#include <stdlib.h>

#include "inet/routing/extras/aodv-uu/aodv-uu/list.h"

namespace inet {

namespace inetmanet {

static inline int listelm_detach(list_t * prev, list_t * next)
{
    next->prev = prev;
    prev->next = next;

    return LIST_SUCCESS;
}

static inline int listelm_add(list_t * le, list_t * prev, list_t * next)
{
    prev->next = le;
    le->prev = prev;
    le->next = next;
    next->prev = le;

    return LIST_SUCCESS;
}


int list_add(list_t * head, list_t * le)
{

    if (!head || !le)
        return LIST_NULL;

    listelm_add(le, head, head->next);

    return LIST_SUCCESS;
}

int list_add_tail(list_t * head, list_t * le)
{

    if (!head || !le)
        return LIST_NULL;

    listelm_add(le, head->prev, head);

    return LIST_SUCCESS;
}

int list_detach(list_t * le)
{
    if (!le)
        return LIST_NULL;

    listelm_detach(le->prev, le->next);

    le->next = le->prev = NULL;

    return LIST_SUCCESS;
};


int list_empty (list_t *head)
{
    if (head == head->next)
        return 1;
    return 0;
}

list_t* list_first(list_t* head)
{
    return head->next;
}

int  list_unattached(list_t *le)
{
    if (le->next == NULL && le->prev == NULL)
        return 1;
    return 0;
}

} // namespace inetmanet

} // namespace inet

