/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#define OMNETPP
#ifdef __KERNEL__
#include <linux/proc_fs.h>
#include <linux/module.h>
#undef DEBUG
#endif
#ifndef OMNETPP
#ifdef NS2
#include "inet/routing/extras/dsr/dsr-uu/ns-agent.h"
#endif
#else
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"
#endif
/* #include "inet/transportlayer/tcp_lwip/lwip/include/lwip/debug.h" */
#include "inet/routing/extras/dsr/dsr-uu/dsr-rtc.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-srt.h"
#include "inet/routing/extras/dsr/dsr-uu/tbl.h"
#include "inet/routing/extras/dsr/dsr-uu/link-cache.h"

#ifdef __KERNEL__
#define DEBUG(f, args...)

MODULE_AUTHOR("erik.nordstrom@it.uu.se");
MODULE_DESCRIPTION("DSR link cache kernel module");
MODULE_LICENSE("GPL");

static struct lc_graph LC;

#define LC_PROC_NAME "dsr_lc"

#endif              /* __KERNEL__ */

#define LC_NODES_MAX 500
#define LC_LINKS_MAX 100    /* TODO: Max links should be calculated from Max
* nodes */

#ifndef UINT_MAX
#define UINT_MAX 4294967295U   /* Max for 32-bit integer */
#endif

#define LC_COST_INF UINT_MAX
#define LC_HOPS_INF UINT_MAX

#ifdef LC_TIMER
#define LC_GARBAGE_COLLECT_INTERVAL 5 * 1000000 /* 5 Seconds */
#endif              /* LC_TIMER */

namespace inet {

namespace inetmanet {

struct lc_node
{
    dsr_list_t l;
    struct in_addr addr;
    unsigned int links;
    unsigned int cost;  /* Cost estimate from source when running Dijkstra */
    unsigned int hops;  /* Number of hops from source. Used to get the
                 * length of the source route to allocate. Same as
                 * cost if cost is hops. */
    struct lc_node *pred;   /* predecessor */
    unsigned int vector_cost[0];
};

struct lc_link
{
    dsr_list_t l;
    struct lc_node *src, *dst;
    int status;
    unsigned int cost;
    struct timeval expires;
};

struct link_query
{
    struct in_addr src, dst;
};

struct cheapest_node
{
    struct lc_node *n;
};

#ifdef __KERNEL__
static int lc_print(struct lc_graph *LC, char *buf);
#endif

static inline void __lc_link_del(struct lc_graph *lc, struct lc_link *link)
{
    /* Also free the nodes if they lack other links */
    if (--link->src->links == 0)
        __tbl_del(&lc->nodes, &link->src->l);

    if (--link->dst->links == 0)
        __tbl_del(&lc->nodes, &link->dst->l);

    __tbl_del(&lc->links, &link->l);
}

static inline int crit_addr(void *pos, void *addr)
{
    struct in_addr *a = (struct in_addr *)addr;
    struct lc_node *p = (struct lc_node *)pos;

    if (p->addr.s_addr == a->s_addr)
        return 1;
    return 0;
}
static inline int crit_link_query(void *pos, void *query)
{
    struct lc_link *p = (struct lc_link *)pos;
    struct link_query *q = (struct link_query *)query;

    if (p->src->addr.s_addr == q->src.s_addr &&
            p->dst->addr.s_addr == q->dst.s_addr)
        return 1;
    return 0;
}

static inline int crit_expire(void *pos, void *data)
{
    struct lc_link *link = (struct lc_link *)pos;
    struct lc_graph *lc = (struct lc_graph *)data;
    struct timeval now;

    gettime(&now);

    /* printf("ptr=0x%x exp_ptr=0x%x now_ptr=0x%x %s<->%s\n", (unsigned int)link, (unsigned int)&link->expires, (unsigned int)&now, print_ip(link->src->addr), print_ip(link->dst->addr)); */
    /*  fflush(stdout); */

    if (timeval_diff(&link->expires, &now) <= 0)
    {
        __lc_link_del(lc, link);
        return 1;
    }
    return 0;
}

static inline int do_lowest_cost(void *pos, void *data)
{
    struct lc_node *n = (struct lc_node *)pos;
    struct cheapest_node *cn = (struct cheapest_node *)data;

    if (n->cost != LC_COST_INF && (!cn->n || n->cost < cn->n->cost))
    {
        cn->n = n;
    }
    return 0;
}

static inline int do_relax(void *pos, void *node)
{
    struct lc_link *link = (struct lc_link *)pos;
    struct lc_node *u = (struct lc_node *)node;
    struct lc_node *v = link->dst;

    /* If u and v have a link between them, update cost if cheaper */
    if (link->src == u)
    {
        unsigned int w = link->cost;

        if ((u->cost + w) < v->cost)
        {
            v->cost = u->cost + w;
            v->hops = u->hops + 1;
            v->pred = u;
            return 1;
        }
    }
    return 0;
}

static inline int do_init(void *pos, void *addr)
{
    struct in_addr *a = (struct in_addr *)addr;
    struct lc_node *n = (struct lc_node *)pos;

    if (!a || !n)
        return -1;

    if (n->addr.s_addr == a->s_addr)
    {
        n->cost = 0;
        n->hops = 0;
        n->pred = n;
    }
    else
    {
        n->cost = LC_COST_INF;
        n->hops = LC_HOPS_INF;
        n->pred = NULL;
    }
    return 0;
}

#ifdef LC_TIMER

void NSCLASS lc_garbage_collect(unsigned long data)
{
    char buf[204859];

    lc_print(&LC, buf);

    /*  printf("#node %d\n%s\n", this->myaddr_.s_addr, buf); */
    /*  fflush(stdout); */
    /*  printf("#end\n"); */
    /*  fflush(stdout); */

    tbl_do_for_each(&LC.links, &LC, crit_expire);

    if (!tbl_empty(&LC.links))
        lc_garbage_collect_set();
}

void NSCLASS lc_garbage_collect_set(void)
{
    DSRUUTimer *lctimer;
    struct timeval expires;

#ifdef NS2
    lctimer = &lc_timer;
#else
    lctimer = &LC.timer;
#endif

    lctimer->function = &NSCLASS lc_garbage_collect;
    lctimer->data = 0;

    gettime(&expires);
    timeval_add_usecs(&expires, LC_GARBAGE_COLLECT_INTERVAL);

    set_timer(lctimer, &expires);
}

#endif              /* LC_TIMER */

static inline struct lc_node *lc_node_create(struct in_addr addr)
{
    struct lc_node *n;

    n = (struct lc_node *)MALLOC(sizeof(struct lc_node), GFP_ATOMIC);

    if (!n)
        return NULL;

    memset(n, 0, sizeof(struct lc_node));
    n->addr = addr;
    n->links = 0;
    n->cost = LC_COST_INF;
    n->pred = NULL;

    return n;
};

static inline struct lc_link *__lc_link_find(struct tbl *t, struct in_addr src,
        struct in_addr dst)
{
    struct link_query q = { src, dst };
    return (struct lc_link *)__tbl_find(t, &q, crit_link_query);
}

static int __lc_link_tbl_add(struct tbl *t, struct lc_node *src,
                             struct lc_node *dst, usecs_t timeout,
                             int status, int cost)
{
    struct lc_link *link;
    int res;

    if (!src || !dst)
        return -1;

    link = (struct lc_link *)__lc_link_find(t, src->addr, dst->addr);

    if (!link)
    {
        link = (struct lc_link *)MALLOC(sizeof(struct lc_link),
                                        GFP_ATOMIC);

        if (!link)
            return -1;

        memset(link, 0, sizeof(struct lc_link));

        __tbl_add_tail(t, &link->l);

        link->src = src;
        link->dst = dst;
        src->links++;
        dst->links++;

        res = 1;
    }
    else
        res = 0;

    link->status = status;
    link->cost = cost;
    gettime(&link->expires);
    timeval_add_usecs(&link->expires, timeout);

    return res;
}

int NSCLASS lc_link_add(struct in_addr src, struct in_addr dst,
                        usecs_t timeout, int status, int cost)
{
    struct lc_node *sn, *dn;
    int res;

    DSR_WRITE_LOCK(&LC.lock);

    sn = (struct lc_node *)__tbl_find(&LC.nodes, &src, crit_addr);

    if (!sn)
    {
        sn = lc_node_create(src);

        if (!sn)
        {
            DEBUG("Could not allocate nodes\n");
            DSR_WRITE_UNLOCK(&LC.lock);
            return -1;
        }
        __tbl_add_tail(&LC.nodes, &sn->l);

    }

    dn = (struct lc_node *)__tbl_find(&LC.nodes, &dst, crit_addr);

    if (!dn)
    {
        dn = lc_node_create(dst);
        if (!dn)
        {
            DEBUG("Could not allocate nodes\n");
            DSR_WRITE_UNLOCK(&LC.lock);
            return -1;
        }
        __tbl_add_tail(&LC.nodes, &dn->l);
    }

    res = __lc_link_tbl_add(&LC.links, sn, dn, timeout, status, cost);

    if (res)
    {
#ifdef LC_TIMER
#ifdef NS2
        if (!timer_pending(&lc_timer))
#else
        if (!timer_pending(&LC.timer))
#endif
            lc_garbage_collect_set();
#endif

    }
    else if (res < 0)
        DEBUG("Could not add new link\n");

    DSR_WRITE_UNLOCK(&LC.lock);

    return 0;
}

int NSCLASS lc_link_del(struct in_addr src, struct in_addr dst)
{
    struct lc_link *link;
    int res = 1;

    DSR_WRITE_LOCK(&LC.lock);

    link = __lc_link_find(&LC.links, src, dst);

    if (!link)
    {
        res = -1;
        goto out;
    }

    __lc_link_del(&LC, link);

    /* Assume bidirectional links for now */
    link = __lc_link_find(&LC.links, dst, src);

    if (!link)
    {
        res = -1;
        goto out;
    }

    __lc_link_del(&LC, link);
out:
    LC.src = NULL;
    DSR_WRITE_UNLOCK(&LC.lock);

    return res;
}

static inline void
__dijkstra_init_single_source(struct tbl *t, struct in_addr src)
{
    __tbl_do_for_each(t, &src, do_init);
}

static inline struct lc_node *__dijkstra_find_lowest_cost_node(struct tbl *t)
{
    struct cheapest_node cn = { NULL };

    __tbl_do_for_each(t, &cn, do_lowest_cost);

    return cn.n;
}

/*
  relax( Node u, Node v, double w[][] )
      if d[v] > d[u] + w[u,v] then
          d[v] := d[u] + w[u,v]
          pi[v] := u

*/
static void __lc_move(struct tbl *to, struct tbl *from)
{
    while (!TBL_EMPTY(from))
    {
        struct lc_node *n;

        n = (struct lc_node *)tbl_detach_first(from);

        tbl_add_tail(to, &n->l);
    }
}

void NSCLASS __dijkstra(struct in_addr src)
{
    TBL(S, LC_NODES_MAX);
    struct lc_node *src_node, *u;
    int i = 0;

    if (TBL_EMPTY(&LC.nodes))
    {
        DEBUG("No nodes in Link Cache\n");
        return;
    }

    __dijkstra_init_single_source(&LC.nodes, src);

    src_node = (struct lc_node *)__tbl_find(&LC.nodes, &src, crit_addr);

    if (!src_node)
        return;

    while ((u = __dijkstra_find_lowest_cost_node(&LC.nodes)))
    {

        tbl_detach(&LC.nodes, &u->l);

        /* Add to S */
        tbl_add_tail(&S, &u->l);

        tbl_do_for_each(&LC.links, u, do_relax);
        i++;
    }

    /* Restore the nodes in the LC graph */
    /* memcpy(&LC.nodes, &S, sizeof(S)); */
    /*  LC.nodes = S; */
    __lc_move(&LC.nodes, &S);

    /* Set currently calculated source */
    LC.src = src_node;
}

struct dsr_srt *NSCLASS lc_srt_find(struct in_addr src, struct in_addr dst)
{
    struct dsr_srt *srt = NULL;
    struct lc_node *dst_node;

    if (src.s_addr == dst.s_addr)
        return NULL;

    DSR_WRITE_LOCK(&LC.lock);

    /*  if (!LC.src || LC.src->addr.s_addr != src.s_addr) */
    __dijkstra(src);

    dst_node = (struct lc_node *)__tbl_find(&LC.nodes, &dst, crit_addr);

    if (!dst_node)
    {
        DEBUG("%s not found\n", print_ip(dst));
        goto out;
    }

    /*  lc_print(&LC, lc_print_buf); */
    /*  DEBUG("Find SR to node %s\n%s\n", print_ip(dst_node->addr), lc_print_buf); */

    /*  DEBUG("Hops to %s: %u\n", print_ip(dst), dst_node->hops); */

    if (dst_node->cost != LC_COST_INF && dst_node->pred)
    {
        struct lc_node *d, *n;
        /*      struct lc_link *l; */
        int k = (dst_node->hops - 1);
        int i = 0;
#ifndef OMNETPP
        srt = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                       (k * sizeof(struct in_addr)),
                                       GFP_ATOMIC);
#else
        int size_cost = 0;

        if (etxActive)
            size_cost = dst_node->hops*sizeof(unsigned int);
        srt = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) + (k * sizeof(struct in_addr))+size_cost,GFP_ATOMIC);
        char *aux = (char *) srt;
        aux += sizeof(struct dsr_srt);
        srt->cost=(unsigned int*)aux;
        aux +=size_cost;
        srt->addrs=(struct in_addr*)(aux);
        if (srt->cost==NULL)
        {
            srt->cost=NULL;
            srt->cost_size=0;
        }
        else
            srt->cost_size=dst_node->hops;
#endif



        if (!srt)
        {
            DEBUG("Could not allocate source route!!!\n");
            goto out;
        }

        srt->dst = dst;
        srt->src = src;
        srt->laddrs = k * sizeof(struct in_addr);

        /*      l = __lc_link_find(&LC.links, dst_node->pred->addr, dst_node->addr); */

        /*      if (!l) { */
        /*          DEBUG("Link not found for timeout update!\n"); */
        /*      } else { */
        /*      /\*     DEBUG("Updating timeout for link %s->%s\n",  *\/ */
        /* /\*                print_ip(l->src->addr),  *\/ */
        /* /\*                print_ip(l->dst->addr)); *\/ */
        /*          gettime(&l->expires); */
        /*      } */

        d = dst_node;

        /* Fill in the source route by traversing the nodes starting
         * from the destination predecessor */
        for (n = dst_node->pred; (n != n->pred); n = n->pred)
        {

            /*      l = __lc_link_find(&LC.links, n->addr, d->addr); */

            /*          if (!l) { */
            /*              DEBUG("Link not found for timeout update!\n"); */
            /*          } else { */
            /*          /\*     DEBUG("Updating timeout for link %s->%s\n",  *\/ */
            /* /\*                    print_ip(l->src->addr),  *\/ */
            /* /\*                    print_ip(l->dst->addr)); *\/ */
            /*              gettime(&l->expires); */
            /*          } */
            srt->addrs[k - i - 1] = n->addr;
            i++;
            d = n;
        }
#ifdef OMNETPP
        i=0;
        if (srt->cost_size>0)
            for (n = dst_node; (n != n->pred); n = n->pred)
            {
                srt->cost[srt->cost_size-i-1]=n->cost - n->pred->cost;
                i++;
            }
#endif

        if ((i + 1) != (int)dst_node->hops)
        {
            DEBUG("hop count ERROR i+1=%d hops=%d!!!\n", i + 1,
                  dst_node->hops);
            FREE(srt);
            srt = NULL;
        }
    }
out:
    DSR_WRITE_UNLOCK(&LC.lock);

    return srt;
}

int NSCLASS
lc_srt_add(struct dsr_srt *srt, usecs_t timeout, unsigned short flags)
{
    int i, n, links = 0;
    struct in_addr addr1, addr2;

    if (!srt)
        return -1;

    n = srt->laddrs / sizeof(struct in_addr);

    addr1 = srt->src;
#ifdef OMNETPP
    struct in_addr myaddr =my_addr();
#endif

    for (i = 0; i < n; i++)
    {
        addr2 = srt->addrs[i];
#ifdef OMNETPP
        IPv4Address ipAddr1((uint32_t)addr1.s_addr);
        IPv4Address ipAddr2((uint32_t)addr2.s_addr);
        if (etxActive)
        {
            if (addr1.s_addr==myaddr.s_addr)
                lc_link_add(addr1, addr2, timeout, 0,(unsigned int)getCost(ipAddr2));
            else if (addr2.s_addr==myaddr.s_addr)
                lc_link_add(addr1, addr2, timeout, 0,(unsigned int)getCost(ipAddr1));
            else
                lc_link_add(addr1, addr2, timeout, 0, srt->cost[i]);
        }
        else
#else // ETx inactive
        lc_link_add(addr1, addr2, timeout, 0, 1);
#endif
            links++;

        if (srt->flags & SRT_BIDIR)
        {
#ifdef OMNETPP
            if (etxActive)
                if (addr1.s_addr==myaddr.s_addr)
                    lc_link_add(addr2, addr1, timeout, 0, (unsigned int)getCost(ipAddr2));
                else if (addr2.s_addr==myaddr.s_addr)
                    lc_link_add(addr2, addr1, timeout, 0,(unsigned int)getCost(ipAddr1));
                else
                    lc_link_add(addr2, addr1, timeout, 0, srt->cost[i]);
            else
#else
            lc_link_add(addr2, addr1, timeout, 0, 1);
#endif
                links++;
        }
        addr1 = addr2;
    }
    addr2 = srt->dst;
#ifdef OMNETPP
    if (etxActive)
    {
        IPv4Address ipAddr1((uint32_t)addr1.s_addr);
        IPv4Address ipAddr2((uint32_t)addr2.s_addr);
        if (addr1.s_addr==myaddr.s_addr)
            lc_link_add(addr1, addr2, timeout, 0, (unsigned int)getCost(ipAddr2));
        else if (addr2.s_addr==myaddr.s_addr)
            lc_link_add(addr1, addr2, timeout, 0,(unsigned int)getCost(ipAddr1));
        else
            lc_link_add(addr1, addr2, timeout, 0, srt->cost[srt->cost_size-1]);
    }
    else
#else
    lc_link_add(addr1, addr2, timeout, 0,1);
#endif
        links++;

    if (srt->flags & SRT_BIDIR)
    {
#ifdef OMNETPP
        if (etxActive)
        {
            IPv4Address ipAddr1((uint32_t)addr1.s_addr);
            IPv4Address ipAddr2((uint32_t)addr2.s_addr);
            if (addr1.s_addr==myaddr.s_addr)
                lc_link_add(addr2, addr1, timeout, 0, (unsigned int)getCost(ipAddr2));
            else if (addr2.s_addr==myaddr.s_addr)
                lc_link_add(addr2, addr1, timeout, 0,(unsigned int)getCost(ipAddr1));
            else
                lc_link_add(addr2, addr1, timeout, 0, srt->cost[srt->cost_size-1]);
        }
        else
#endif
            lc_link_add(addr2, addr1, timeout, 0, 1);
        links++;
    }
    return links;
}


int lc_srt_del(struct in_addr src, struct in_addr dst)
{
    return 0;
}

void NSCLASS lc_flush(void)
{
    DSR_WRITE_LOCK(&LC.lock);
#ifdef LC_TIMER
#ifdef NS2
    if (timer_pending(&lc_timer))
        del_timer(&lc_timer);
#else
    if (timer_pending(&LC.timer))
        del_timer(&LC.timer);
#endif
#endif
    tbl_flush(&LC.links, NULL);
    tbl_flush(&LC.nodes, NULL);

    LC.src = NULL;

    DSR_WRITE_UNLOCK(&LC.lock);
}

#ifdef __KERNEL__
static char *print_hops(unsigned int hops)
{
    static char c[18];

    if (hops == LC_HOPS_INF)
        sprintf(c, "INF");
    else
        sprintf(c, "%u", hops);
    return c;
}

static char *print_cost(unsigned int cost)
{
    static char c[18];

    if (cost == LC_COST_INF)
        sprintf(c, "INF");
    else
        sprintf(c, "%u", cost);
    return c;
}

static int lc_print(struct lc_graph *LC, char *buf)
{
    dsr_list_t *pos;
    int len = 0;
    struct timeval now;

    if (!LC)
        return 0;

    gettime(&now);

    DSR_READ_LOCK(&LC->lock);

    len += sprintf(buf, "# %-15s %-15s %-4s Timeout\n", "Src Addr",
                   "Dst Addr", "Cost");

    list_for_each(pos, &LC->links.head)
    {
        struct lc_link *link = (struct lc_link *)pos;

        len += sprintf(buf + len, "  %-15s %-15s %-4u %lu\n",
                       print_ip(link->src->addr),
                       print_ip(link->dst->addr),
                       link->cost,
                       timeval_diff(&link->expires, &now) / 1000000);
    }

    len += sprintf(buf + len, "\n# %-15s %-4s %-4s %-5s %12s %12s\n",
                   "Addr", "Hops", "Cost", "Links", "This", "Pred");

    list_for_each(pos, &LC->nodes.head)
    {
        struct lc_node *n = (struct lc_node *)pos;

        len += sprintf(buf + len, "  %-15s %4s %4s %5u %12lX %12lX\n",
                       print_ip(n->addr),
                       print_hops(n->hops),
                       print_cost(n->cost),
                       n->links,
                       (unsigned long)n, (unsigned long)n->pred);
    }

    DSR_READ_UNLOCK(&LC->lock);
    return len;

}


static int lc_proc_info(char *buffer, char **start, off_t offset, int length)
{
    int len;

    len = lc_print(&LC, buffer);

    *start = buffer + offset;
    len -= offset;
    if (len > length)
        len = length;
    else if (len < 0)
        len = 0;
    return len;
}

EXPORT_SYMBOL(lc_srt_add);
EXPORT_SYMBOL(lc_srt_find);
EXPORT_SYMBOL(lc_flush);
EXPORT_SYMBOL(lc_link_del);
EXPORT_SYMBOL(lc_link_add);

module_init(lc_init);
module_exit(lc_cleanup);

#endif              /* __KERNEL__ */

int __init NSCLASS lc_init(void)
{
    /* Initialize Graph */
    INIT_TBL(&LC.links, LC_LINKS_MAX);
    INIT_TBL(&LC.nodes, LC_NODES_MAX);

    LC.src = NULL;

#ifdef __KERNEL__
    LC.lock = RW_LOCK_UNLOCKED;
#ifdef LC_TIMER
    init_timer(&LC.timer);
#endif
    proc_net_create(LC_PROC_NAME, 0, lc_proc_info);
#endif
    return 0;
}

void __exit NSCLASS lc_cleanup(void)
{
    lc_flush();
#ifdef __KERNEL__
    proc_net_remove(LC_PROC_NAME);
#endif
}

} // namespace inetmanet

} // namespace inet

