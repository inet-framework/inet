/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_H
#define _DSR_H

#ifndef OMNETPP


#ifdef __KERNEL__
#include <asm/byteorder.h>
#include <linux/types.h>
//#include <linux/in.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/time.h>
#ifdef KERNEL26
#include <linux/jiffies.h>
#endif
#else
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <endian.h>
#include <netinet/in.h>
#endif              /* __KERNEL__ */

#endif /*omnetpp*/

#include "dsr-pkt.h"
#include "timer.h"


#ifndef NO_GLOBALS

namespace inet {

namespace inetmanet {

#define DSR_BROADCAST ((unsigned int) 0xffffffff)

#ifndef OMNETPP
#ifdef NS2
#define IPPROTO_DSR PT_DSR
#else
#define IPPROTO_DSR 168        /* Is this correct? */
#endif
#else
#define IPPROTO_DSR IP_PROT_DSR
#endif /* OMNETPP */

#define IP_HDR_LEN 20
#define DSR_OPTS_MAX_SIZE 50    /* This is used to reduce the MTU of the DSR *
* device so that packets are not too big after
* adding the DSR header. A better solution
* should probably be found... */

enum confval
{
#ifdef DEBUG
    PrintDebug,
#endif
    FlushLinkCache,
    PromiscOperation,
    BroadcastJitter,
    RouteCacheTimeout,
    SendBufferTimeout,
    SendBufferSize,
    RequestTableSize,
    RequestTableIds,
    MaxRequestRexmt,
    MaxRequestPeriod,
    RequestPeriod,
    NonpropRequestTimeout,
    RexmtBufferSize,
    MaintHoldoffTime,
    MaxMaintRexmt,
    UseNetworkLayerAck,
    TryPassiveAcks,
    PassiveAckTimeout,
    GratReplyHoldOff,
    MAX_SALVAGE_COUNT,
    RREQMulVisit,
    RREQMaxVisit,
    RREPDestinationOnly,
#ifdef OMNETPP
    PathCache,
    RetryPacket,
#endif
    CONFVAL_MAX,
};

enum confval_type
{
    SECONDS,
    MILLISECONDS,
    MICROSECONDS,
    NANOSECONDS,
    QUANTA,
    BINARY,
    COMMAND,
    CONFVAL_TYPE_MAX,
};

#define MAINT_BUF_MAX_LEN 100
#define RREQ_TBL_MAX_LEN 64 /* Should be enough */
#define SEND_BUF_MAX_LEN 100
#define RREQ_TLB_MAX_ID 16

static struct
{
    const char *name;
    const unsigned int val;
    enum confval_type type;
} confvals_def[CONFVAL_MAX] =
{
#ifdef DEBUG
    { "PrintDebug", 0, BINARY},
#endif
    { "FlushLinkCache", 1, COMMAND}
    ,{ "PromiscOperation", 1, BINARY}
    ,{ "BroadcastJitter", 20, MILLISECONDS}
    ,{ "RouteCacheTimeout", 300, SECONDS}
    ,{ "SendBufferTimeout", 30, SECONDS}
    ,{ "SendBufferSize", SEND_BUF_MAX_LEN, QUANTA}
    ,{ "RequestTableSize", RREQ_TBL_MAX_LEN, QUANTA}
    ,{ "RequestTableIds", RREQ_TLB_MAX_ID, QUANTA}
    ,{ "MaxRequestRexmt", 16, QUANTA}
    ,{ "MaxRequestPeriod", 10, SECONDS}
    ,{ "RequestPeriod", 500, MILLISECONDS}
    ,{ "NonpropRequestTimeout", 30, MILLISECONDS}
    ,{ "RexmtBufferSize", MAINT_BUF_MAX_LEN}
    ,{ "MaintHoldoffTime", 250, MILLISECONDS}
    ,{ "MaxMaintRexmt", 2, QUANTA}
    ,{ "UseNetworkLayerAck", 1, BINARY}
    ,{ "TryPassiveAcks", 1, QUANTA}
    ,{ "PassiveAckTimeout", 100, MILLISECONDS}
    ,{ "GratReplyHoldOff", 1, SECONDS}
    ,{ "MAX_SALVAGE_COUNT", 15, QUANTA}
    ,{"RREQMulVisit",0,BINARY}
    ,{"RREQMaxVisit",1,QUANTA}
    ,{"RREPDestinationOnly",0,BINARY}
#ifdef OMNETPP
    ,{"PathCache", 0, BINARY}
    ,{"RetryPacket", 0, BINARY}
#endif
};

struct dsr_node
{
    struct in_addr ifaddr;
    struct in_addr bcaddr;
    unsigned int confvals[CONFVAL_MAX];
#ifdef __KERNEL__
    char slave_ifname[IFNAMSIZ];
    struct net_device *slave_dev;
    struct in_device *slave_indev;
    struct net_device_stats stats;
    spinlock_t lock;
#endif
};

#ifdef __KERNEL__
#define DSR_SPIN_LOCK(l)    spin_lock(l)
#define DSR_SPIN_UNLOCK(l)  spin_unlock(l)
#define MALLOC(s, p)        kmalloc(s, p)
#define FREE(p)             kfree(p)
#define NSCLASS
#define XMIT(pkt)           dsr_dev_xmit(pkt)
#else
#define DSR_SPIN_LOCK(l)
#define DSR_SPIN_UNLOCK(l)
#endif              /* __KERNEL__ */

#ifdef __KERNEL__

#define ConfVal(cv) (get_confval(cv))
#define ConfValToUsecs(cv) (confval_to_usecs(cv))

extern struct dsr_node *dsr_node;

static inline unsigned int get_confval(enum confval cv)
{
    unsigned int val = 0;

    if (dsr_node)
    {
        DSR_SPIN_LOCK(&dsr_node->lock);
        val = dsr_node->confvals[cv];
        DSR_SPIN_UNLOCK(&dsr_node->lock);
    }
    return val;
}

static inline int set_confval(enum confval cv, unsigned int val)
{
    if (dsr_node)
    {
        DSR_SPIN_LOCK(&dsr_node->lock);
        dsr_node->confvals[cv] = val;
        DSR_SPIN_UNLOCK(&dsr_node->lock);
    }
    else
        return -1;

    return val;
}

static inline void dsr_node_init(struct dsr_node *dn, char *ifname)
{
    int i;
    dn->slave_indev = NULL;
    dn->slave_dev = NULL;
    memcpy(dn->slave_ifname, ifname, IFNAMSIZ);

    spin_lock_init(&dn->lock);

    for (i = 0; i < CONFVAL_MAX; i++)
    {
        dn->confvals[i] = confvals_def[i].val;
    }
}

static inline struct in_addr my_addr(void)
{
    static struct in_addr my_addr;
    if (dsr_node)
    {
        DSR_SPIN_LOCK(&dsr_node->lock);
        my_addr = dsr_node->ifaddr;
        DSR_SPIN_UNLOCK(&dsr_node->lock);
    }
    return my_addr;
}

static inline unsigned long time_add_msec(unsigned long msecs)
{
    struct timespec t;

    t.tv_sec = msecs / 1000;
    t.tv_nsec = (msecs * 1000000) % 1000000000;

    return timespec_to_jiffies(&t);
}

static inline int get_slave_dev_ifindex(void)
{
    int ifindex = -1;

    if (dsr_node)
    {
        DSR_SPIN_LOCK(&dsr_node->lock);
        if (dsr_node->slave_dev)
            ifindex = dsr_node->slave_dev->ifindex;
        DSR_SPIN_UNLOCK(&dsr_node->lock);
    }
    return ifindex;
}

static inline void dsr_node_lock(struct dsr_node *dnode)
{
    spin_lock(&dnode->lock);
}

static inline void dsr_node_unlock(struct dsr_node *dnode)
{
    spin_unlock(&dnode->lock);
}
int dsr_ip_recv(struct sk_buff *skb);
int do_mackill(char *mac);

#endif

} // namespace inetmanet

} // namespace inet

#endif              /* NO_GLOBALS */

#ifndef NO_DECLS
#ifndef OMNETPP
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
#endif
#endif              /* NO_DECLS */



#endif              /* _DSR_H */
