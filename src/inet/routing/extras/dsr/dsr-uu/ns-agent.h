/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_NS_AGENT
#define _DSR_NS_AGENT

#ifndef NS2
#error "To compile the ns-2 version of DSR-UU, NS2 must be defined!"
#endif              /* NS2 */

namespace inet { namespace inetmanet { class DSRUU; } }

#include <stdarg.h>

#include <object.h>
#include <agent.h>
#include <trace.h>
#include <scheduler.h>
#include <packet.h>
#include <dsr-priqueue.h>
#include <mac.h>
#include <mac-802_11.h>
#include <mobilenode.h>

#define ETH_ALEN 6
#define NSCLASS DSRUU::
#define ConfVal(name) DSRUU::get_confval(name)
#define ConfValToUsecs(cv) DSRUU::confval_to_usecs(cv)

#include "inet/routing/extras/dsr/dsr-uu/tbl.h"
#include "endian.h"
#include "inet/routing/extras/dsr/dsr-uu/timer.h"

#define NO_DECLS
#include "inet/routing/extras/dsr/dsr-uu/debug_dsr.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-opt.h"
#include "inet/routing/extras/dsr/dsr-uu/send-buf.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-rreq.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-pkt.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-rrep.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-rerr.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-ack.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-srt.h"
#include "inet/routing/extras/dsr/dsr-uu/neigh.h"
#include "inet/routing/extras/dsr/dsr-uu/link-cache.h"
#undef NO_DECLS

typedef dsr_opt_hdr hdr_dsr;
#define HDR_DSRUU(p) ((hdr_dsr *)hdr_dsr::access(p))

#define init_timer(timer)
#define timer_pending(timer) ((timer)->getStatus() == TIMER_PENDING)
#define del_timer_sync(timer) del_timer(timer)
#define MALLOC(s, p) malloc(s)
#define FREE(p) free(p)
#define XMIT(pkt) ns_xmit(pkt)
#define DELIVER(pkt) ns_deliver(pkt)
#define __init
#define __exit
#define ntohl(x) x
#define htonl(x) x
#define htons(x) x
#define ntohs(x) x

#define IPDEFTTL 64

namespace inet {

namespace inetmanet {

class DSRUU:public Tap, public Agent
{
  public:
    friend class DSRUUTimer;

    DSRUU();
    ~DSRUU();

    DSRUUTimer ack_timer;

    int command(int argc, const char *const *argv);
    void recv(Packet *, Handler * callback = 0);
    void tap(const Packet * p);
    Packet *ns_packet_create(struct dsr_pkt *dp);
    void ns_xmit(struct dsr_pkt *dp);
    void ns_deliver(struct dsr_pkt *dp);
    void xmit_failed(Packet *p);

    struct hdr_ip *dsr_build_ip(struct dsr_pkt *dp, struct in_addr src,
                                struct in_addr dst, int ip_len,
                                int tot_len, int protocol, int ttl);
    void add_timer(DSRUUTimer * t)
    {
        /* printf("Setting timer %s to %f\n", t->get_name(), t->expires - Scheduler::getInstance().clock()); */
        if (t->expires - Scheduler::getInstance().clock() < 0)
            t->resched(0);
        else
            t->resched(t->expires - Scheduler::getInstance().clock());
    }
    /*      void mod_timer (DSRUUTimer *t, unsinged long expires_)  *//*            { t->expires = expires_ ; t->resched(t->expires); } */
    void del_timer(DSRUUTimer * t)
    {
        //printf("Cancelling timer %s\n", t->get_name());
        t->cancel();
    }
    void set_timer(DSRUUTimer * t, struct timeval *expires)
    {
        //printf("In set_timer\n");
        t->expires = expires->tv_usec;
        t->expires /= 1000000l;
        t->expires += expires->tv_sec;
        /*  printf("Set timer %s to %f\n", t->get_name(), t->expires - Scheduler::getInstance().clock()); */
        add_timer(t);
    }
    static const unsigned int get_confval(enum confval cv)
    {
        return confvals[cv];
    }
    static const unsigned int set_confval(enum confval cv, unsigned int val)
    {
        confvals[cv] = val;
        return val;
    }

#define NO_GLOBALS
#undef NO_DECLS

#undef _DSR_H
#include "dsr.h"

#undef _DSR_OPT_H
#include "dsr-opt.h"

#undef _DSR_IO_H
#include "inet/routing/extras/dsr/dsr-uu/dsr-io.h"

#undef _DSR_RREQ_H
#include "dsr-rreq.h"

#undef _DSR_RREP_H
#include "dsr-rrep.h"

#undef _DSR_RERR_H
#include "dsr-rerr.h"

#undef _DSR_ACK_H
#include "dsr-ack.h"

#undef _DSR_SRT_H
#include "dsr-srt.h"

#undef _SEND_BUF_H
#include "send-buf.h"

#undef _NEIGH_H
#include "neigh.h"

#undef _MAINT_BUF_H
#include "inet/routing/extras/dsr/dsr-uu/maint-buf.h"

#undef _LINK_CACHE_H
#include "link-cache.h"

#undef _DEBUG_H
#include "debug_dsr.h"

#undef NO_GLOBALS

    struct in_addr my_addr()
    {
        return myaddr_;
    }
    int arpset(struct in_addr addr, unsigned int mac_addr);
    inline void ethtoint(char *eth, int *num)
    {
        memcpy((char *)num, eth, ETH_ALEN);
        return;
    }
    inline void inttoeth(int *num, char *eth)
    {
        memcpy(eth, (char *)num, ETH_ALEN);
        return;
    }
  private:
    static int confvals[CONFVAL_MAX];
    struct in_addr myaddr_;
    unsigned long macaddr_;
    MACAddress macAddr;
    Trace *trace_;
    Mac *mac_;
    LL *ll_;
    CMUPriQueue *ifq_;
    MobileNode *node_;

    struct tbl rreq_tbl;
    struct tbl grat_rrep_tbl;
    struct tbl send_buf;
    struct tbl neigh_tbl;
    struct tbl maint_buf;

    unsigned int rreq_seqno;

    DSRUUTimer grat_rrep_tbl_timer;
    DSRUUTimer send_buf_timer;
    DSRUUTimer neigh_tbl_timer;
    DSRUUTimer lc_timer;

    /* The link cache */
    struct lc_graph LC;
};

} // namespace inetmanet

} // namespace inet

#endif              /* _DSR_NS_AGENT_H */

