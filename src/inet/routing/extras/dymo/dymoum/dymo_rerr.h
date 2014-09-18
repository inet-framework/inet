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

#ifndef __DYMO_RERR_H__
#define __DYMO_RERR_H__

#ifndef NS_NO_GLOBALS

#include "inet/routing/extras/dymo/dymoum/defs_dymo.h"
#include "inet/routing/extras/dymo/dymoum/debug_dymo.h"
#include "inet/routing/extras/dymo/dymoum/rtable.h"
#include <assert.h>
#ifndef OMNETPP
#include <sys/types.h>
#endif
#ifndef OMNETPP
/* Blocks contained within a RERR message */
struct rerr_block
{
    u_int32_t   unode_addr;
    u_int32_t   unode_seqnum;
};
#define MAX_RERR_BLOCKS 50

/* RERR message */
typedef struct      // FIXME: adjust byte ordering
{
    u_int32_t   m : 1;
    u_int32_t   h : 2;
    u_int32_t   type : 5;
    u_int32_t   len : 12;
    u_int32_t   ttl : 6;
    u_int32_t   i : 1;
    u_int32_t   res : 5;

    struct rerr_block rerr_blocks[MAX_RERR_BLOCKS];
} RERR;

#define RERR_SIZE   sizeof(RERR)
#define RERR_BLOCK_SIZE sizeof(struct rerr_block)
#define RERR_BASIC_SIZE (RERR_SIZE - (MAX_RERR_BLOCKS * RERR_BLOCK_SIZE))

#endif

#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS
/* Create a RERR given an array of blocks (previously created), the number of
   those blocks and the TTL of the message */
RERR *rerr_create(struct rerr_block *blocks, int nblocks, int ttl);

/* Send a RERR */
void rerr_send(struct in_addr addr, int ttl, rtable_entry_t *entry);

/* Forward a RERR */
void rerr_forward(RERR *rerr);

/* Process a RERR given the address of the sender of the message and the
   interface from which the message was received */
void rerr_process(RERR *rerr,struct in_addr src, u_int32_t ifindex);

/* Return the number of blocks contained inside a RERR */
static NS_INLINE int rerr_numblocks(RERR *rerr)
{
    assert(rerr);

    if ((rerr->len - RERR_BASIC_SIZE) % RERR_BLOCK_SIZE != 0)
        return -1;
    return (rerr->len - RERR_BASIC_SIZE) / RERR_BLOCK_SIZE;
}

#ifdef OMNETPP
void rerr_forward(RERR *rerr,struct in_addr dest_addr);
void rerr_send(struct in_addr addr, int ttl, rtable_entry_t *entry,struct in_addr dest_addr);
#endif

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __DYMO_RERR_H__ */

