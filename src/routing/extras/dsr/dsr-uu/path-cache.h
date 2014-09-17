/* Copyright (C) Uppsala University
 * Copyright (C) Universidad de Málaga
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 * Author: Alfonso Ariza,
 */

#ifndef _DSR_PATH_CACHE_H
#define _DSR_PATH_CACHE_H

#include "tbl.h"
#include "timer.h"

#ifndef NO_GLOBALS

#define MAX_TABLE_HASH 64

namespace inet {

namespace inetmanet {

struct path_table
{
    struct tbl hash[MAX_TABLE_HASH];
#ifdef __KERNEL__
    struct timer_list timer;
    rwlock_t lock;
#endif
};

} // namespace inetmanet

} // namespace inet

#define VALID 0
#define GET_HASH(s,dest) &((s)->hash[dest%MAX_TABLE_HASH])

#endif              /* NO_GLOBALS */

#ifndef NO_DECLS

struct dsr_srt  * ph_srt_find(struct in_addr srt,struct in_addr dst,int criteria,unsigned int timeout);
void ph_srt_add(struct dsr_srt * srt, usecs_t timeout, unsigned short flags);
void ph_srt_add_node(struct in_addr node, usecs_t timeout, unsigned short flags,unsigned int cost);
void ph_srt_delete_node(struct in_addr src);
void ph_srt_delete_link(struct in_addr srt,struct in_addr dst);
int  path_cache_init(void);
void path_cache_cleanup(void);

#endif              /* NO_DECLS */

#endif              /* _LINK_CACHE */
