/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef __INET_DSR_UU_OMNETPP_H
#define __INET_DSR_UU_OMNETPP_H


#ifndef OMNETPP
#define OMNETPP
#endif


#include "inet/routing/extras/base/compatibility_dsr.h"

// Force to considere all links bi dir (link cache disjktra)
#define BIDIR
//#include <stdarg.h>

#ifndef MobilityFramework
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/ProtocolMap.h"
#include "inet/routing/extras/base/ControlManetRouting_m.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/routing/extras/base/ManetNetfilterHook.h"
#include "inet/networklayer/ipv4/ICMP.h"
#else
#include "Blackboard.h"
#include "LinkBreak.h"
#endif

//#include <map>

// generate ev prints
#ifdef _WIN32
#define DEBUG omnet_debug
#else
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG(f, args...) omnet_debug(f, ## args)
#endif


#ifdef MobilityFramework
#ifndef IPv4Address
#define IPv4Address int
#endif
#ifndef IPv4Datagram
#define IPv4Datagram NetwPkt
#endif
#ifndef MACAddress
#define MACAddress int
#endif
#define ICMP_DESTINATION_UNREACHABLE 1
#define ICMP_TIME_EXCEEDED 2
#include "SimpleArp.h"
#include "MacControlInfo.h"
#endif

/*
#ifdef _WIN32
#define DEBUG(f, args)
#else
#ifdef DEBUG
#undef DEBUG
#define DEBUG_PROC
#define DEBUG(f, args...)
//#define DEBUG(f, args...) trace(__FUNCTION__, f, ## args)
#else
#define DEBUG(f, args...)
#endif
#endif
*/

namespace inet {

namespace inetmanet {

#define ETH_ALEN 6
#define NSCLASS DSRUU::

class DSRUU;

#define ConfVal(name) DSRUU::get_confval(name)
#define ConfValToUsecs(cv) confval_to_usecs(cv)

} // namespace inetmanet

} // namespace inet

#include "dsr-uu/tbl.h"
#include "dsr-uu/timer.h"

namespace inet{

namespace inetmanet {

static inline char *print_ip(struct in_addr addr)
{
    static char buf[16 * 4];
    static int index = 0;
    char *str;

    sprintf(&buf[index], "%d.%d.%d.%d",
            0x0ff & (uint32_t)addr.s_addr,
            0x0ff & ((uint32_t)addr.s_addr >> 8),
            0x0ff & ((uint32_t)addr.s_addr >> 16), 0x0ff & ((uint32_t)addr.s_addr >> 24));

    str = &buf[index];
    index += 16;
    index %= 64;

    return str;
}

static inline char *print_eth(char *addr)
{
    static char buf[30];

    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)addr[0], (unsigned char)addr[1],
            (unsigned char)addr[2], (unsigned char)addr[3],
            (unsigned char)addr[4], (unsigned char)addr[5]);

    return buf;
}

static inline char *print_pkt(char *p, int len)
{
    static char buf[3000];
    int i, l = 0;

    for (i = 0; i < len; i++)
        l += sprintf(buf + l, "%02X", (unsigned char)p[i]);

    return buf;
}

} // namespace inetmanet

} // namespace inet

#define NO_DECLS
#include "dsr-uu/dsr.h"
#include "dsr-uu/dsr-opt.h"
#include "dsr-uu/send-buf.h"
#include "dsr-uu/dsr-rreq.h"
#include "dsr-uu/dsr-pkt.h"
#include "dsr-uu/dsr-rrep.h"
#include "dsr-uu/dsr-rerr.h"
#include "dsr-uu/dsr-ack.h"
#include "dsr-uu/dsr-srt.h"
#include "dsr-uu/neigh.h"
#include "dsr-uu/link-cache.h"
#include "inet/routing/extras/dsr/dsr-pkt_omnet.h"
#include "dsr-uu/path-cache.h"
#undef NO_DECLS

namespace inet {

namespace inetmanet {

#define init_timer(timer)
#define timer_pending(timer) ((timer)->pending())

#define del_timer_sync(timer) del_timer(timer)
//#define MALLOC(s, p) malloc(s)
//#define FREE(p) free(p)
#define XMIT(pkt) omnet_xmit(pkt)
#define DELIVER(pkt) omnet_deliver(pkt)
#define __init
#define __exit
#define ntohl(x) x
#define htonl(x) x
#define htons(x) x
#define ntohs(x) x

#define IPDEFTTL 64


#define  grat_rrep_tbl_timer (*grat_rrep_tbl_timer_ptr)
#define  send_buf_timer  (*send_buf_timer_ptr)
#define  neigh_tbl_timer  (*neigh_tbl_timer_ptr)
#define  lc_timer  (*lc_timer_ptr)
#define  ack_timer  (*ack_timer_ptr)
#define  etx_timer  (*etx_timer_ptr)

#ifdef MobilityFramework
class DSRUU:public cSimpleModule, public ImNotifiable
{
#else
class DSRUU:public cSimpleModule, public cListener, public ManetNetfilterHook
{
#endif
  public:
    friend class DSRUUTimer;
    //static simtime_t current_time;
    static struct dsr_pkt *lifoDsrPkt;
    static int lifo_token;

  private:
    struct in_addr myaddr_;
    MACAddress macaddr_;
    int interfaceId;

    // Trace *trace_;
    // Mac *mac_;
    // LL *ll_;
    // CMUPriQueue *ifq_;
    // MobileNode *node_;

    struct tbl rreq_tbl;
    struct tbl grat_rrep_tbl;
    struct tbl send_buf;
    struct tbl neigh_tbl;
    struct tbl maint_buf;

    unsigned int rreq_seqno;

    DSRUUTimer *grat_rrep_tbl_timer_ptr;
    DSRUUTimer *send_buf_timer_ptr;
    DSRUUTimer *neigh_tbl_timer_ptr;
    DSRUUTimer *lc_timer_ptr;
    DSRUUTimer *ack_timer_ptr;
    // ETX Parameters
    bool etxActive;
    DSRUUTimer *etx_timer_ptr;
    int etxSize;
    double etxTime;
    double etxWindow;
    double etxWindowSize;
    double etxJitter;
    struct ETXEntry;
    int etxNumRetry;

    typedef std::map<IPv4Address, ETXEntry*> ETXNeighborTable;
    struct ETXEntry
    {
        double deliveryDirect;
        double deliveryReverse;
        std::vector<simtime_t> timeVector;
        //IPv4Address address;
    };
// In dsr-uu-omnet.cc used for ETX
    ETXNeighborTable etxNeighborTable;
    void EtxMsgSend(unsigned long data);
    void EtxMsgProc(cMessage *msg);
    double getCost(IPv4Address add);
    void AddCost(struct dsr_pkt *,struct dsr_srt *);
    void ExpandCost(struct dsr_pkt *);
    double PathCost(struct dsr_pkt *dp);

    void linkFailed(IPv4Address);
//************++

#ifndef MobilityFramework
    IIPv4RoutingTable *inet_rt;
    IInterfaceTable *inet_ift;
#else
    SimpleArp* arp;
#endif

    /* The link cache */
    struct lc_graph LC;
    struct path_table PCH;

    struct in_addr my_addr()
    {
        return myaddr_;
    }

    void drop (cMessage *msg,int code) { delete msg;}

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

  protected:
    struct in_addr ifaddr;
    struct in_addr bcaddr;
    static unsigned int confvals[CONFVAL_MAX];
    InterfaceEntry *   interface80211ptr;
    void tap(DSRPkt * p,cObject *ctrl);
    void omnet_xmit(struct dsr_pkt *dp);
    void omnet_deliver(struct dsr_pkt *dp);
    void packetFailed(IPv4Datagram *ipDgram);
    void handleTimer(cMessage*);
    void defaultProcess(cMessage*);

    struct dsr_srt *RouteFind(struct in_addr , struct in_addr);
    int RouteAdd(struct dsr_srt *, unsigned long, unsigned short );

    bool proccesICMP(cMessage *msg);

  public:
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    DSRUU();
    ~DSRUU();

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    struct iphdr * dsr_build_ip(struct dsr_pkt *dp, struct in_addr src,
                                struct in_addr dst, int ip_len, int tot_len,
                                int protocol, int ttl);

    void add_timer(DSRUUTimer * t)
    {
        /* printf("Setting timer %s to %f\n", t->get_name(), t->expires - Scheduler::getInstance().clock()); */
        if (t->getExpires() - simTime() < 0)
            t->resched(0);
        else
            t->resched( SIMTIME_DBL(t->getExpires() -simTime()));
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
        double exp;
        exp = expires->tv_usec;
        exp /= 1000000l;
        exp += expires->tv_sec;
        t->setExpires( exp);
        /*  printf("Set timer %s to %f\n", t->get_name(), t->expires - Scheduler::getInstance().clock()); */
        add_timer(t);
    }

    void set_timer(DSRUUTimer * t, double expire)
    {
        //printf("In set_timer\n");
        t->setExpires( expire);
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
#include "dsr-uu/dsr.h"

#undef _DSR_OPT_H
#include "dsr-uu/dsr-opt.h"

#undef _DSR_IO_H
#include "dsr-uu/dsr-io.h"

#undef _DSR_SRT_H
#include "dsr-uu/dsr-srt.h"

#undef _DSR_RREQ_H
#include "dsr-uu/dsr-rreq.h"

#undef _DSR_RREP_H
#include "dsr-uu/dsr-rrep.h"

#undef _DSR_RERR_H
#include "dsr-uu/dsr-rerr.h"

#undef _DSR_ACK_H
#include "dsr-uu/dsr-ack.h"


#undef _SEND_BUF_H
#include "dsr-uu/send-buf.h"

#undef _NEIGH_H
#include "dsr-uu/neigh.h"

#undef _MAINT_BUF_H
#include "dsr-uu/maint-buf.h"

#undef _LINK_CACHE_H
#include "dsr-uu/link-cache.h"

#undef _DSR_PATH_CACHE_H
#include "dsr-uu/path-cache.h"

#undef NO_GLOBALS

};

static inline usecs_t confval_to_usecs(enum confval cv)
{
    usecs_t usecs = 0;
    unsigned int val;

    val = ConfVal(cv);

    switch (confvals_def[cv].type)
    {
    case SECONDS:
        usecs = val * 1000000;
        break;
    case MILLISECONDS:
        usecs = val * 1000;
        break;
    case MICROSECONDS:
        usecs = val;
        break;
    case NANOSECONDS:
        usecs = val / 1000;
        break;
    case BINARY:
    case QUANTA:
    case COMMAND:
    case CONFVAL_TYPE_MAX:
        break;
    }

    return usecs;
}


static inline int omnet_vprintk(const char *fmt, va_list args)
{
    int printed_len, prefix_len=0;
    static char printk_buf[1024];


    /* Emit the output into the temporary buffer */
//#ifdef _WIN32
#if 0
    printed_len = _vsnprintf_s(printk_buf + prefix_len,
                               sizeof(printk_buf) - prefix_len,sizeof(printk_buf) - prefix_len, fmt, args);
#else
    printed_len = vsnprintf(printk_buf + prefix_len,
                            sizeof(printk_buf) - prefix_len, fmt, args);
#endif
    EV_DEBUG << printk_buf;
    return printed_len;
}



static inline void omnet_debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    omnet_vprintk(fmt, args);
    va_end(args);
}

static inline void gettime(struct timeval *tv)
{
    double now, usecs;

    /* Timeval is required, timezone is ignored */
    if (!tv)
        return;
    now = SIMTIME_DBL(simTime());
    tv->tv_sec = (long)now; /* Removes decimal part */
    usecs = (now - tv->tv_sec) * 1000000;
    tv->tv_usec = (long)(usecs+0.5);
    if (tv->tv_usec>1000000)
    {
        tv->tv_usec -=1000000;
        tv->tv_sec++;
    }
}

} // namespace inetmanet

} // namespace inet

#endif              /* _DSR_NS_AGENT_H */
