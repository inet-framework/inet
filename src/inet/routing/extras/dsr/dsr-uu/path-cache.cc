/*****************************************************************************
 *
 * Copyright (C) 2007 Malaga University.
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
 * Authors: Alfonso Ariza Quintana.<aarizaq@uma.ea>
 *
 *****************************************************************************/
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
#include "inet/routing/extras/dsr/dsr-uu/path-cache.h"

#ifdef __KERNEL__
#define DEBUG(f, args...)

static struct path_table PCH;

#define LC_PROC_NAME "dsr_pc"

#endif              /* __KERNEL__ */

#ifndef UINT_MAX
#define UINT_MAX 4294967295U   /* Max for 32-bit integer */
#endif

#ifndef INT_MAX
#define INT_MAX 2147483640L   /* Max for 32-bit integer */
#endif

namespace inet {

namespace inetmanet {

/* LC_TIMER */

struct node_cache
{
    dsr_list_t l;
    struct tbl paths;
    struct in_addr addr;
};

struct path
{
    dsr_list_t l;
    struct in_addr *route;
    unsigned int *vector_cost;
    unsigned int size_cost;
    unsigned int num_hop;
    int status;
    double cost;
    struct timeval expires;
};

/*
static inline void __ph_delete_route(struct path *rt)
{
    list_del(&rt->l);
}
*/

static inline int crit_cache_query(void *pos, void *query)
{
    struct node_cache *p = (struct node_cache *)pos;
    struct in_addr *q = (struct in_addr *)query;

    if (p->addr.s_addr == q->s_addr)
        return 1;
    return 0;
}


/*
static inline int crit_expire(void *pos, void *data)
{
    struct path *rt = (struct path *)pos;
    struct timeval now;

    gettime(&now);

//    printf("ptr=0x%x exp_ptr=0x%x now_ptr=0x%x %s<->%s\n", (unsigned int)link, (unsigned int)&link->expires, (unsigned int)&now, print_ip(link->src->addr), print_ip(link->dst->addr));
//    fflush(stdout);

    if (timeval_diff(&rt->expires, &now) <= 0)
    {
        __ph_delete_route(rt);
        return 1;
    }
    return 0;
}
*/

static inline struct path *path_create()
{
    struct path *n;

    n = (struct path *)MALLOC(sizeof(struct path), GFP_ATOMIC);

    if (!n)
        return nullptr;
    memset(n, 0, sizeof(struct path));
    return n;
};

static inline struct node_cache *node_cache_create(struct in_addr addr)
{
    struct node_cache *n;
    n = (struct node_cache *)MALLOC(sizeof(struct node_cache), GFP_ATOMIC);

    if (!n)
        return nullptr;
    memset(n, 0, sizeof(struct node_cache));
    INIT_TBL(&n->paths,64);
    n->addr=addr;
    return n;
}

static inline struct node_cache *__node_cache_find(struct path_table *t, struct in_addr dest)
{
    struct tbl *n = GET_HASH(t,(uint32_t)dest.s_addr);
    return (struct node_cache *)__tbl_find(n, &dest, crit_cache_query);
}


static int __ph_route_tbl_add(struct path_table *rt_t,
                              struct in_addr dst,int num_hops,struct in_addr *rt_nodes, usecs_t timeout,
                              int status, double cost,unsigned int * cost_vector, unsigned int vector_size)
{
    struct node_cache *n;
    struct path * rt;
    struct path * rt_aux;
    struct path * rt_aux2;
    int res = 0;
    int exist = 0;
    struct timeval now;
    long diff;
    int size;
    rt_aux2=nullptr;
    dsr_list_t *pos;

    size = sizeof(struct in_addr)*(num_hops-1);
    n = __node_cache_find(rt_t,dst);

    if (n==nullptr)
    {
        n=node_cache_create(dst);
        struct tbl *aux = GET_HASH(rt_t,(uint32_t)dst.s_addr);
        __tbl_add_tail(aux,&n->l);
    }

    gettime(&now);
    if (!tbl_empty(&n->paths))
    {
        list_for_each(pos, &n->paths.head)
        {
            rt = (struct path * ) pos;
            if ((int)rt->num_hop == num_hops)
            {
                if (rt->num_hop == 1)
                {
                    exist=-1;
                    break;
                }
                else if (memcmp(rt->route,rt_nodes,size)==0)
                {
                    exist=-1;
                    break;
                }
            }
        }
    }
    if (!exist)
    {
        rt= path_create();
        if (!rt)
            return -1;
        if (size>0)
        {
            rt->route = (struct in_addr*) MALLOC(size,GFP_ATOMIC);
            memcpy (rt->route,rt_nodes, size);
        }

        if (TBL_FULL(&n->paths)) // Table is full delete oldest
        {
            diff =0;
            list_for_each(pos, &n->paths.head)
            {
                rt_aux = (struct path * ) pos;
                if (diff<timeval_diff(&now,&rt_aux->expires))
                {
                    diff=timeval_diff(&now,&rt_aux->expires);
                    rt_aux2=rt_aux;
                }
            }
            if (rt_aux2)
            {
                __tbl_detach(&n->paths,&rt_aux2->l);
                if (rt_aux2->route)
                    FREE(rt_aux2->route);
                if (rt_aux2->size_cost>0)
                    FREE(rt_aux2->vector_cost);
                FREE (rt_aux2);
            }

        }
        __tbl_add_tail(&n->paths, &rt->l);
    }
    else
        res = 0;

    rt->status = status;

    if (cost!=-1)
        rt->cost = cost;

    rt->num_hop = num_hops;


    if (vector_size>0)
    {
        if (rt->size_cost !=vector_size)
        {
            if (rt->size_cost>0)
                FREE (rt->vector_cost);
            rt->vector_cost = (unsigned int*)MALLOC(vector_size*sizeof(unsigned int),GFP_ATOMIC);
            rt->size_cost=vector_size;
        }
        memcpy (rt->vector_cost,cost_vector, vector_size*sizeof(unsigned int));
    }

    gettime(&rt->expires);
    timeval_add_usecs(&rt->expires, timeout);
    return res;
}


struct dsr_srt *NSCLASS ph_srt_find(struct in_addr src,struct in_addr dst,int criteria,unsigned int timeout)
{
    dsr_list_t *tmp;
    dsr_list_t *pos;
    struct path * rt;
    struct timeval now;
    struct timeval *expires;


    struct in_addr *route;
    unsigned int *vector_cost;
    unsigned int vector_size = 0;

    unsigned int num_hop = UINT_MAX;
    int status; (void)status; // NOP to avoidl UNUSED variable warning
    int i;
    double cost = 1.0/0.0;
    bool find;
    struct dsr_srt *srt = nullptr;
    struct in_addr myAddr = my_addr();

    struct node_cache *dst_node = __node_cache_find(&PCH,dst);
    if (dst_node==nullptr)
        return nullptr;


    if (tbl_empty(&dst_node->paths))
        return nullptr;


    gettime(&now);
    route=nullptr;
    list_for_each_safe(pos,tmp,&dst_node->paths.head)
    {
        rt = (struct path * ) pos;
        if (timeval_diff(&rt->expires, &now) <= 0)
        {
            __tbl_detach(&dst_node->paths,&rt->l);
            if (rt->route)
                FREE(rt->route);
            if (rt->size_cost>0)
                FREE(rt->vector_cost);
            FREE (rt);
            continue;
        }

        if (src.s_addr!=myAddr.s_addr)
        {
            // check
            find = false;
            for (i=0; i<(int)rt->num_hop; i++)
            {
                if (rt->route[i].s_addr==src.s_addr)
                    find = true;
            }

            if (!find)
                continue;
        }

        if (route==nullptr && rt->status==VALID)
        {
            num_hop= rt->num_hop;
            status= rt->status;
            cost= rt->cost;
            route = rt->route;
            vector_cost =rt->vector_cost;
            vector_size=rt->size_cost;
            expires = &rt->expires;
        }
        else if (rt->cost<cost)
        {
            num_hop= rt->num_hop;
            status= rt->status;
            cost= rt->cost;
            route = rt->route;
            vector_cost =rt->vector_cost;
            vector_size=rt->size_cost;
            expires = &rt->expires;
        }
        else if (rt->cost==cost && (timeval_diff(&rt->expires, &now) <0) )
        {
            num_hop= rt->num_hop;
            status= rt->status;
            cost= rt->cost;
            route = rt->route;
            vector_cost =rt->vector_cost;
            vector_size=rt->size_cost;
            expires = &rt->expires;
        }
    }


    if (!route && num_hop!=1)
    {
        DEBUG("%s not found\n", print_ip(dst));
        return nullptr;
    }

    /*  lc_print(&LC, lc_print_buf); */
    /*  DEBUG("Find SR to node %s\n%s\n", print_ip(dst_node->addr), lc_print_buf); */

    /*  DEBUG("Hops to %s: %u\n", print_ip(dst), dst_node->hops); */

    int k = (num_hop - 1);
#ifndef OMNETPP
    srt = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                   (k * sizeof(struct in_addr)),
                                   GFP_ATOMIC);

#else
    int size_cost = 0;
    if (vector_size>0)
        size_cost = vector_size*sizeof(unsigned int);
    srt = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                   (k * sizeof(struct in_addr))+size_cost,
                                   GFP_ATOMIC);
    char *aux = (char *) srt;
    aux += sizeof(struct dsr_srt);
    srt->cost=(unsigned int*)aux;
    aux +=size_cost;
    srt->addrs=(struct in_addr*)(aux);

    if (vector_cost<=nullptr)
    {
        srt->cost=nullptr;
        srt->cost_size=0;
    }
    else
    {
        srt->cost_size=vector_size;
        memcpy(srt->cost,vector_cost,sizeof(unsigned int)*vector_size);
        IPv4Address dstAddr((uint32_t)dst.s_addr);
        IPv4Address srtAddr((uint32_t)srt->addrs[0].s_addr);

        if (myAddr.s_addr==src.s_addr)
        {
            if (num_hop==1)
                srt->cost[0] = (int) getCost(dstAddr);
            else
                srt->cost[0] = (int) getCost(srtAddr);
        }

    }

    if (etxActive)
        if (k!=0 && srt->cost_size==0)
            DEBUG("Could not allocate source route!!!\n");
#endif
    if (!srt)
    {
        DEBUG("Could not allocate source route!!!\n");
        return nullptr;
    }


    srt->dst = dst;
    srt->src = src;
    srt->laddrs = k * sizeof(struct in_addr);
    memcpy(srt->addrs,route,srt->laddrs);
    /// ???????? must be ?????????????????
    if (timeout>0)
    {
        gettime(expires);
        timeval_add_usecs(expires, timeout);
    }
    return srt;
}

void NSCLASS
ph_srt_add(struct dsr_srt *srt, usecs_t timeout, unsigned short flags)
{
    int i, n;
    struct in_addr addr1, addr2;

    int j=0;
    int l;
    struct in_addr myaddr;
    struct dsr_srt *dsr_aux;
    double cost;
    unsigned int init_cost = INT_MAX;
    bool is_first,is_last;
    int size_cost=0;
    unsigned int *cost_vector=nullptr;


    if (!srt)
        return;

    n = srt->laddrs / sizeof(struct in_addr);

    addr1 = srt->src;
    myaddr = my_addr();

    is_first = (srt->src.s_addr==myaddr.s_addr)?true:false;
    is_last = (srt->dst.s_addr==myaddr.s_addr)?true:false;

    if (!is_first && !is_last)
    {
        for (i = 0; i < n; i++)
        {
            j=i+1;
            if (srt->addrs[i].s_addr == myaddr.s_addr)
                break;
        }
    }

    if (!is_last)
    {
#ifdef OMNETPP
        if (etxActive && srt->cost)
        {
            if (j<n)
            {
                IPv4Address srtAddr((uint32_t)srt->addrs[j].s_addr);
                double cost = getCost(srtAddr);
                if (cost<0)
                    init_cost= INT_MAX;
                else
                    init_cost= (unsigned int) cost;
                srt->cost[j]= init_cost;
            }
            else if (n==0)
            {
                IPv4Address srtAddr((uint32_t)srt->dst.s_addr);
                double cost  = getCost(srtAddr);
                if (cost<0)
                    init_cost= INT_MAX;
                else
                    init_cost= (unsigned int) cost;
                srt->cost[0]= init_cost;
            }
        }
#endif
        size_cost = 0;
        cost_vector=nullptr;

        for (i = j; i < n; i++)
        {
            addr2 = srt->addrs[i];
            cost = i+1-j;
#ifdef OMNETPP
            if (etxActive)
            {
                if (srt->cost_size>0)
                {
                    cost = init_cost;
                    if (cost<0)
                        cost = 1e100;
                    for (l=j; l<i; l++)
                        cost += srt->cost[l];
                    size_cost = i-j+1;
                    cost_vector=&(srt->cost[j]);
                }
                if (i+1-j!=0 && size_cost==0)
                    DEBUG("Error !!!\n");
            }
#endif
            __ph_route_tbl_add(&PCH,addr2,i+1-j,&(srt->addrs[j]), timeout, 0,cost, cost_vector,size_cost);
        }

        if ((j<n && n!=0) || (n==0))
        {
            addr2 = srt->dst;
            cost = n+1-j;
            size_cost = 0;
            cost_vector=nullptr;
#ifdef OMNETPP
            if (etxActive)
            {
                if (srt->cost_size>0)
                {
                    cost = init_cost;
                    for (l=j; l<n; l++)
                        cost += srt->cost[l];
                    cost_vector = &(srt->cost[j]);
                    size_cost = n-j+1;
                }
                if (n+1-j!=0 && size_cost==0)
                    DEBUG("Error !!!\n");
            }
#endif
            __ph_route_tbl_add(&PCH,addr2,n+1-j,&(srt->addrs[j]), timeout, 0, cost,cost_vector,size_cost);
        }
    }

    if (is_first)
        return;

    if (srt->flags & SRT_BIDIR&flags)
    {
        j=0;
        cost_vector = nullptr;
        size_cost=0;
        if (!is_first)
        {
            dsr_aux  = dsr_srt_new_rev(srt);
            addr1 = dsr_aux->src;

            if (!is_last)
                for (i = 0; i < n; i++)
                {
                    j=i+1;
                    if (dsr_aux->addrs[i].s_addr == myaddr.s_addr)
                        break;
                }

            if (j<n)
            {

                IPv4Address srtAddr((uint32_t)dsr_aux->addrs[j].s_addr);
                double cost = getCost(srtAddr);
                if (cost<0)
                    init_cost= INT_MAX;
                else
                    init_cost= (unsigned int) cost;

                dsr_aux->cost[j]= init_cost;
            }

            for (i = j; i < n; i++)
            {
                addr2 = dsr_aux->addrs[i];
                cost = i+1-j;
#ifdef OMNETPP
                if (dsr_aux->cost_size>0 && etxActive)
                {
                    cost = init_cost;
                    if (cost<0)
                        cost = 1e100;
                    for (l=j; l<i; l++)
                        cost += dsr_aux->cost[l];
                    size_cost = i-j;
                    cost_vector= &(dsr_aux->cost[j]);
                }
#endif
                __ph_route_tbl_add(&PCH,addr2,i+1-j,&(dsr_aux->addrs[j]), timeout, 0, cost,cost_vector,size_cost);
            }
            cost = n+1-j;
#ifdef OMNETPP
            if (etxActive)
                if (dsr_aux->cost_size>0)
                {
                    cost = init_cost;
                    for (l=j; l<n; l++)
                        cost += dsr_aux->cost[l];
                    size_cost = dsr_aux->cost_size-j+1;
                    cost_vector= &(dsr_aux->cost[j]);
                }
#endif
            addr2 = dsr_aux->dst;
            __ph_route_tbl_add(&PCH,addr2,n+1-j,dsr_aux->addrs, timeout, 0, cost,cost_vector,size_cost);
            FREE (dsr_aux);
        }
    }
}

void NSCLASS
ph_srt_add_node(struct in_addr node, usecs_t timeout, unsigned short flags,unsigned int cost)
{

    struct dsr_srt *srt;
    struct in_addr myAddr;
    myAddr=my_addr();
    if (node.s_addr==myAddr.s_addr)
        return;
#ifndef OMNETPP
    srt = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt),
                                   GFP_ATOMIC);
#else
    unsigned int size_cost=0;
    if (cost!=0)
    {
        size_cost=sizeof(unsigned int);
    }
    srt = (struct dsr_srt *) MALLOC((sizeof(struct dsr_srt) + size_cost), GFP_ATOMIC);

    char *aux = (char *) srt;
    aux += sizeof(struct dsr_srt);

    srt->cost=(unsigned int*)aux;
    aux +=size_cost;
    srt->addrs=(struct in_addr*) aux;
    if (cost!=0)
    {
        srt->cost[0]=cost;
        srt->cost_size=1;
    }
    else
    {
        srt->cost=nullptr;
        srt->cost_size=0;
    }
#endif

    if (!srt)
        return;
    srt->laddrs =0;
    srt->dst=node;
    srt->src=my_addr();

    ph_srt_add(srt,timeout,flags);
    FREE (srt);

}


void NSCLASS ph_srt_delete_node(struct in_addr src)
{
    dsr_list_t *tmp;
    dsr_list_t *tmp2;
    dsr_list_t *pos1;
    dsr_list_t *pos2;
    struct path * rt;
    struct timeval now;
    struct node_cache *n_cache;

    int i,j;

    gettime(&now);

    for (i=0; i<MAX_TABLE_HASH; i++)
    {
        if (tbl_empty(&PCH.hash[i])) continue;
        list_for_each_safe(pos1,tmp,&PCH.hash[i].head)
        {
            n_cache = (struct node_cache *) pos1;
            if (tbl_empty(&n_cache->paths)) continue;
            if (n_cache->addr.s_addr==src.s_addr)
            {
                list_for_each_safe(pos2,tmp2,&n_cache->paths.head)
                {
                    rt = (struct path*) pos2;
                    __tbl_detach(&n_cache->paths,&rt->l);
                    if (rt->route)
                        FREE(rt->route);
                    if (rt->size_cost>0)
                        FREE(rt->vector_cost);
                    FREE (rt);
                }
                continue;
            }
            list_for_each_safe(pos2,tmp2,&n_cache->paths.head)
            {
                rt = (struct path*) pos2;
                if (timeval_diff(&rt->expires, &now) <= 0)
                {
                    __tbl_detach(&n_cache->paths,&rt->l);
                    if (rt->route)
                        FREE(rt->route);
                    if (rt->size_cost>0)
                        FREE(rt->vector_cost);
                    FREE (rt);
                    continue;
                }
                int long_route=rt->num_hop-1;
                for (j=0; j<long_route; j++)
                {
                    if (rt->route[j].s_addr==src.s_addr)
                    {
                        __tbl_detach(&n_cache->paths,&rt->l);
                        if (rt->route)
                            FREE(rt->route);
                        if (rt->size_cost>0)
                            FREE(rt->vector_cost);
                        FREE (rt);
                        break;
                    }
                }
            }
        }
    }
}



void NSCLASS ph_srt_delete_link(struct in_addr src,struct in_addr dst)
{
    dsr_list_t *tmp;
    dsr_list_t *tmp2;
    dsr_list_t *pos1;
    dsr_list_t *pos2;
    struct path * rt;
    struct timeval now;
    struct node_cache *n_cache;
    int i,j;

    gettime(&now);

    for (i=0; i<MAX_TABLE_HASH; i++)
    {
        if (tbl_empty(&PCH.hash[i])) continue;
        list_for_each_safe(pos1,tmp,&PCH.hash[i].head)
        {
            n_cache = (struct node_cache *) pos1;
            if (tbl_empty(&n_cache->paths)) continue;
            list_for_each_safe(pos2,tmp2,&n_cache->paths.head)
            {
                rt = (struct path*) pos2;
                if (timeval_diff(&rt->expires, &now) <= 0)
                {
                    __tbl_detach(&n_cache->paths,&rt->l);
                    if (rt->route)
                        FREE(rt->route);
                    if (rt->size_cost>0)
                        FREE(rt->vector_cost);
                    FREE (rt);
                    continue;
                }
                if (rt->num_hop==1 )
                {
                    if (n_cache->addr.s_addr==dst.s_addr)
                    {
                        if (rt->route)
                            FREE(rt->route);
                        if (rt->size_cost>0)
                            FREE(rt->vector_cost);
                        __tbl_detach(&n_cache->paths,&rt->l);
                        FREE (rt);
                    }
                    continue;
                }
                if (src.s_addr==my_addr().s_addr && rt->route[0].s_addr== dst.s_addr)
                {
                    if (rt->route)
                        FREE(rt->route);
                    if (rt->size_cost>0)
                        FREE(rt->vector_cost);
                    __tbl_detach(&n_cache->paths,&rt->l);
                    FREE (rt);
                    continue;
                }
                int long_route=rt->num_hop-2;
                int nextIt = 1;
                for (j=0; j<long_route; j++)
                {
                    if (rt->route[j].s_addr==src.s_addr && rt->route[j+1].s_addr==dst.s_addr)
                    {
                        __tbl_detach(&n_cache->paths,&rt->l);
                        if (rt->route)
                            FREE(rt->route);
                        if (rt->size_cost>0)
                            FREE(rt->vector_cost);
                        FREE (rt);
                        nextIt=0;
                        break;
                    }
                }

                if (nextIt && (rt->route[long_route].s_addr==src.s_addr && n_cache->addr.s_addr==dst.s_addr))
                {
                    __tbl_detach(&n_cache->paths,&rt->l);
                    if (rt->route)
                        FREE(rt->route);
                    if (rt->size_cost>0)
                        FREE(rt->vector_cost);
                    FREE (rt);
                }
            }
        }
    }
}



int NSCLASS path_cache_init(void)
{
    /* Initialize Graph */
    int i;
    for (i=0; i<MAX_TABLE_HASH; i++)
    {
        INIT_TBL(&PCH.hash[i],100 );
    }
    return 0;
}

void NSCLASS path_cache_cleanup(void)
{
    dsr_list_t *tmp;
    dsr_list_t *tmp2;
    dsr_list_t *pos1;
    dsr_list_t *pos2;
    struct path * rt;
    struct node_cache *n_cache;
    int i;

    for (i=0; i<MAX_TABLE_HASH; i++)
    {
        if (tbl_empty(&PCH.hash[i])) continue;
        list_for_each_safe(pos1,tmp,&PCH.hash[i].head)
        {
            n_cache=(struct node_cache *)pos1;
            if (tbl_empty(&n_cache->paths))
            {
                __tbl_detach(&PCH.hash[i],&n_cache->l);
                FREE(n_cache);
                continue;
            }
            list_for_each_safe(pos2,tmp2,&n_cache->paths.head)
            {
                rt = (struct path *) pos2;
                __tbl_detach(&n_cache->paths,&rt->l);
                if (rt->route)
                    FREE(rt->route);
                if (rt->size_cost>0)
                    FREE(rt->vector_cost);
                FREE (rt);
            }
            __tbl_detach(&PCH.hash[i],&n_cache->l);
            FREE(n_cache);
        }
    }
}

} // namespace inetmanet

} // namespace inet

