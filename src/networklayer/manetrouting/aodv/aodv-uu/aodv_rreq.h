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
 * Authors: Erik Nordström, <erik.nordstrom@it.uu.se>
 *
 *
 *****************************************************************************/
#ifndef _AODV_RREQ_H
#define _AODV_RREQ_H

#ifndef NS_NO_GLOBALS

#ifndef OMNETPP
#ifndef  _WIN32
#include <endian.h>
#else
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN
#endif
#endif
#endif

#include "defs_aodv.h"
#include "seek_list.h"
#include "routing_table.h"

/* RREQ Flags: */
#define RREQ_JOIN          0x1
#define RREQ_REPAIR        0x2
#define RREQ_GRATUITOUS    0x4
#define RREQ_DEST_ONLY     0x8

#ifndef OMNETPP
typedef struct
{
    u_int8_t type;
#if defined(__LITTLE_ENDIAN)
    u_int8_t res1:4;
    u_int8_t d:1;
    u_int8_t g:1;
    u_int8_t r:1;
    u_int8_t j:1;
#elif defined(__BIG_ENDIAN)
    u_int8_t j:1;       /* Join flag (multicast) */
    u_int8_t r:1;       /* Repair flag */
    u_int8_t g:1;       /* Gratuitous RREP flag */
    u_int8_t d:1;       /* Destination only respond */
    u_int8_t res1:4;
#else
#error "Adjust your <bits/endian.h> defines"
#endif
    u_int8_t res2;
    u_int8_t hcnt;
    u_int32_t rreq_id;
    u_int32_t dest_addr;
    u_int32_t dest_seqno;
    u_int32_t orig_addr;
    u_int32_t orig_seqno;
} RREQ;

#define RREQ_SIZE sizeof(RREQ)
#endif
/* A data structure to buffer information about received RREQ's */
struct rreq_record
{
    list_t l;
    struct in_addr orig_addr;   /* Source of the RREQ */
    u_int32_t rreq_id;      /* RREQ's broadcast ID */
    struct timer rec_timer;
};

struct blacklist
{
    list_t l;
    struct in_addr dest_addr;
    struct timer bl_timer;
};
#endif              /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS
RREQ *rreq_create(u_int8_t flags, struct in_addr dest_addr,
                  u_int32_t dest_seqno, struct in_addr orig_addr);
void rreq_send(struct in_addr dest_addr, u_int32_t dest_seqno, int ttl,
               u_int8_t flags);
void rreq_forward(RREQ * rreq, int size, int ttl);
void rreq_process(RREQ * rreq, int rreqlen, struct in_addr ip_src,
                  struct in_addr ip_dst, int ip_ttl, unsigned int ifindex);
void rreq_route_discovery(struct in_addr dest_addr, u_int8_t flags,
                          struct ip_data *ipd);
void rreq_record_timeout(void *arg);
struct blacklist *rreq_blacklist_insert(struct in_addr dest_addr);
void rreq_blacklist_timeout(void *arg);
void rreq_local_repair(rt_table_t * rt, struct in_addr src_addr,
                       struct ip_data *ipd);

#ifdef NS_PORT
struct rreq_record *rreq_record_insert(struct in_addr orig_addr,
                                       u_int32_t rreq_id);
struct rreq_record *rreq_record_find(struct in_addr orig_addr,
                                     u_int32_t rreq_id);
struct blacklist *rreq_blacklist_find(struct in_addr dest_addr);
#endif              /* NS_PORT */

#endif              /* NS_NO_DECLARATIONS */

#endif              /* AODV_RREQ_H */
