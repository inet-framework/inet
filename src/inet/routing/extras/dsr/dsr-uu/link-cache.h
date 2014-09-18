/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _LINK_CACHE_H
#define _LINK_CACHE_H

#include "inet/routing/extras/dsr/dsr-uu/tbl.h"
#include "inet/routing/extras/dsr/dsr-uu/timer.h"

//#define LC_TIMER

#ifndef NO_GLOBALS

namespace inet {

namespace inetmanet {

struct lc_graph
{
    struct tbl nodes;
    struct tbl links;
    struct lc_node *src;
#ifdef __KERNEL__
    struct timer_list timer;
    rwlock_t lock;
#endif
};
#ifndef OMNETPP
#define dsr_rtc_find(s,d) lc_srt_find(s,d)
#define dsr_rtc_add(srt,t,f) lc_srt_add(srt,t,f)
#else
#define dsr_rtc_find(s,d) RouteFind(s,d)
#define dsr_rtc_add(srt,t,f) RouteAdd(srt,t,f)
#endif

} // namespace inetmanet

} // namespace inet

#endif              /* NO_GLOBALS */

#ifndef NO_DECLS

int lc_link_del(struct in_addr src, struct in_addr dst);
int lc_link_add(struct in_addr src, struct in_addr dst,
                unsigned long timeout, int status, int cost);
void lc_garbage_collect_set(void);
void lc_garbage_collect(unsigned long data);
struct dsr_srt *lc_srt_find(struct in_addr src, struct in_addr dst);
int lc_srt_add(struct dsr_srt *srt, unsigned long timeout,
               unsigned short flags);
void lc_flush(void);
void __dijkstra(struct in_addr src);
int lc_init(void);
void lc_cleanup(void);

#endif              /* NO_DECLS */


#endif              /* _LINK_CACHE */
