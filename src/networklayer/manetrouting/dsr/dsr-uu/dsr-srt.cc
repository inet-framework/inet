/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#define OMNETPP
#ifdef __KERNEL__
#include <linux/slab.h>
#include <net/ip.h>
#endif

#ifndef OMNETPP
#ifdef NS2
#include "ns-agent.h"
#endif
#else
#include "dsr-uu-omnetpp.h"
#endif

#include "dsr.h"
#include "dsr-srt.h"
#include "dsr-opt.h"
#include "dsr-ack.h"
#include "link-cache.h"
#include "neigh.h"
#include "dsr-rrep.h"
#include "debug_dsr.h"

struct in_addr dsr_srt_next_hop(struct dsr_srt *srt, int sleft)
{
    int n = srt->laddrs / sizeof(struct in_addr);
    struct in_addr nxt_hop;

    if (sleft <= 0)
        nxt_hop = srt->dst;
    else
        nxt_hop = srt->addrs[n - sleft];

    return nxt_hop;
}

struct in_addr dsr_srt_prev_hop(struct dsr_srt *srt, int sleft)
{
    struct in_addr prev_hop;
    int n = srt->laddrs / sizeof(u_int32_t);

    if (n - 1 == sleft)
        prev_hop = srt->src;
    else
        prev_hop = srt->addrs[n - 2 - (sleft)];

    return prev_hop;
}

static int dsr_srt_find_addr(struct dsr_srt *srt, struct in_addr addr,
                             int sleft)
{
    int n = srt->laddrs / sizeof(struct in_addr);

    if (n == 0 || sleft > n || sleft < 1)
        return 0;

    for (; sleft > 0; sleft--)
        if (srt->addrs[n - sleft].s_addr == addr.s_addr)
            return 1;

    if (addr.s_addr == srt->dst.s_addr)
        return 1;

    return 0;
}

#ifndef OMNETPP
struct dsr_srt *dsr_srt_new(struct in_addr src, struct in_addr dst,
                            unsigned int length, char *addrs)
{
    struct dsr_srt *sr;

    sr = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) + length,
                                  GFP_ATOMIC);

    if (!sr)
        return NULL;

    sr->src.s_addr = src.s_addr;
    sr->dst.s_addr = dst.s_addr;
    sr->laddrs = length;
    /*  sr->index = index; */

    if (length != 0 && addrs)
        memcpy(sr->addrs, addrs, length);

    return sr;
}

#else
struct dsr_srt *dsr_srt_new(struct in_addr src, struct in_addr dst,
                            unsigned int length, char *addrs,EtxCost *cost,int sizeEtx)
{
    struct dsr_srt *sr;
    int size_cost=0;
    int sizeAdd;
    bool testInverse=false;

    sizeAdd = length / sizeof(struct in_addr);

    if (sizeEtx>0)
        size_cost=sizeEtx*sizeof(unsigned int);
    sr = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) + length+size_cost,
                                  GFP_ATOMIC);

    char *aux = (char *) sr;
    aux += sizeof(struct dsr_srt);

    sr->cost=(unsigned int*)aux;
    aux +=size_cost;
    sr->addrs=(struct in_addr*) aux;

    if (sizeEtx<=0)
    {
        sr->cost=NULL;
        sr->cost_size=0;
    }
    else
        sr->cost_size=sizeEtx;

    if (!sr)
        return NULL;

    sr->src.s_addr = src.s_addr;
    sr->dst.s_addr = dst.s_addr;
    sr->laddrs = length;
    /*  sr->index = index; */
    uint32_t addrs1;
    uint32_t addrs2;
    if (length != 0 && addrs)
        memcpy(sr->addrs, addrs, length);
    if (sizeEtx>0) // Integrity test
    {
        if (sizeAdd == (sizeEtx-1))
        {
            sr->cost_size=sizeEtx;
            int size = sizeAdd<sizeEtx?sizeAdd:sizeEtx;
            for (int i=0; i<size; i++)
            {
                addrs1 = cost[i].address.getInt();
                addrs2 = sr->addrs[i].s_addr;
                if (addrs2 != addrs1)
                {
                    testInverse = true;
                    break;
                }
            }
            if (!testInverse)
            {
                for (int i=0; i<sizeEtx; i++)
                    sr->cost[i]= (unsigned int) cost[i].cost;
            }
            else
            {
                for (int i=0; i<size; i++)
                {

                    if (cost[sizeEtx-1].address.getInt()==src.s_addr || cost[sizeEtx-1].address.getInt()==dst.s_addr )
                        addrs1 = cost[sizeEtx-2-i].address.getInt();
                    else
                        addrs1 = cost[sizeEtx-1-i].address.getInt();
                    addrs2 = sr->addrs[i].s_addr;
                    if (addrs2 != addrs1)
                        opp_error("Dsr error, Etx address and dsr are different");
                }
                for (int i=0; i<sizeEtx; i++)
                    sr->cost[i]= (unsigned int) cost[sizeEtx-1-i].cost;
            }
        }
        else
        {
            sr->cost_size=sizeAdd+1;
            int j=-1;
            for (int i=0; i<sizeEtx; i++)
            {

                if (cost[i].address.getInt()==src.s_addr)
                {
                    j=i;
                    break;
                }
            }
            if (j==-1)
                opp_error("Dsr error, Etx address and dsr are different");
            else
            {
                testInverse=false;
                int l=0;
                for (int i=j-1; i>=0; i--)
                {

                    addrs1 = cost[i].address.getInt();
                    addrs2 = sr->addrs[l].s_addr;
                    l++;
                    if (addrs2 != addrs1)
                    {
                        testInverse = true;
                        break;
                    }
                }
                if (!testInverse)
                {
                    l=0;
                    for (int i=j-1; i>=0; i--)
                    {
                        sr->cost[l]= (unsigned int) cost[i].cost;
                        l++;
                    }
                }
                else
                {
                    for (int i=0; i<sizeAdd; i++)
                    {
                        addrs1 = cost[j+1+i].address.getInt();
                        addrs2 = sr->addrs[i].s_addr;
                        if (addrs2 != addrs1)
                            opp_error("Dsr error, Etx address and dsr are different");
                    }
                    for (int i=0; i<sizeAdd+1; i++)
                        sr->cost[i]= (unsigned int) cost[j+1+i].cost;
                }
            }
        }
    }
    else
        sr->cost_size=0;
#ifdef BIDIR
    sr->flags |= SRT_BIDIR;
#endif
    return sr;
}


void dsr_srt_split_both(struct dsr_srt *srt, struct in_addr addr, struct in_addr src,struct dsr_srt **srt_to_dest_ptr,struct dsr_srt **srt_to_src_ptr)
{
    int i, n,l;
    bool split;

    struct dsr_srt *srt_to_dest = NULL;
    struct dsr_srt *srt_to_src = NULL;
    *srt_to_dest_ptr = srt_to_dest;
    *srt_to_src_ptr = srt_to_src;
    if (!srt)
        return;
    n = srt->laddrs / sizeof(struct in_addr);

    if (n == 0)
        return;

    split =false;
    if (addr.s_addr ==srt->src.s_addr)
    {
        split = true;
        i=0;
        l=-1;
        if (addr.s_addr != src.s_addr)
            l=-2;
    }
    else if (addr.s_addr ==srt->dst.s_addr)
    {
        for (i = 0; i < n; i++) // avoid loop
            if (src.s_addr == srt->addrs[i].s_addr)
                return;
        split = true;
        i=n;
        l=n+1;
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (addr.s_addr == srt->addrs[i].s_addr)
            {
                split = true;
                l=i;
                if (addr.s_addr != src.s_addr)
                    l--;
                break;
            }
        }
    }
    /* Nothing to split */
    if (!split)
        return;

    int size_cost = 0;
    if (n-l-1>0)
    {
        if (srt->cost_size>0)
            size_cost = (n-l)*sizeof(unsigned int);
        srt_to_dest = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                               ((n-l-1) * sizeof(struct in_addr))+size_cost,GFP_ATOMIC);
        char *aux = (char *) srt_to_dest;
        aux += sizeof(struct dsr_srt);
        srt_to_dest->cost=(unsigned int*)aux;
        aux +=size_cost;
        srt_to_dest->addrs=(struct in_addr*)(aux);
        if (srt->cost==NULL)
        {
            srt_to_dest->cost=NULL;
            srt_to_dest->cost_size=0;
        }
        srt_to_dest->flags = srt->flags;
    }

    if (srt_to_dest)
    {
        srt_to_dest->src.s_addr = src.s_addr;
        srt_to_dest->dst.s_addr = srt->dst.s_addr;
        srt_to_dest->laddrs = sizeof(struct in_addr) * (n-l-1);

        if (addr.s_addr ==srt->src.s_addr && addr.s_addr != src.s_addr)
        {
            memcpy(srt_to_dest->addrs+1, srt->addrs, sizeof(struct in_addr) * n);
            srt_to_dest->addrs[0].s_addr=srt->src.s_addr;
            if (srt->cost_size>0)
            {
                memcpy(srt_to_dest->cost+1, srt->cost, sizeof(unsigned int) * (n+1));
                srt_to_dest->cost_size= srt->cost_size+1;
                srt_to_dest->cost[0]=1;
            }
            else
                srt_to_dest->cost_size=0;
        }
        else
        {
            memcpy(srt_to_dest->addrs, srt->addrs+l+1, sizeof(struct in_addr) * (n-l-1));
            if (srt->cost_size>0)
            {
                memcpy(srt_to_dest->cost, srt->cost+l+1, sizeof(unsigned int) * (n-l));
                srt_to_dest->cost_size=(n-l);
            }
            else
                srt_to_dest->cost_size=0;
        }

        for (int k = 0; k < (n-l-1); k++)
        {
            if (src.s_addr == srt_to_dest->addrs[k].s_addr) // avoid loop
            {
                int j=0;
                for (int m = k+1; m < (n-l-1); m++)
                {
                    srt_to_dest->addrs[j]=srt_to_dest->addrs[m];
                    if (srt->cost_size>0)
                        srt_to_dest->cost[j] = srt_to_dest->cost[m];
                    j++;
                }
                if (srt->cost_size>0)
                {
                    srt_to_dest->cost_size=n-l-(k+1);
                    srt_to_dest->cost[n-l-(k+1)] = srt_to_dest->cost[n-l];
                }
                srt_to_dest->laddrs = sizeof(struct in_addr) * (n-l-1-(k+1));
            }
        }
    }

    if (addr.s_addr ==srt->src.s_addr)
    {
        i=0;
        l=-1;
    }
    else if (src.s_addr ==srt->dst.s_addr)
        i=l=n;
    else
    {
        l=i;
        if (addr.s_addr != src.s_addr)
            l++;
    }

    size_cost = 0;
    if (l>0)
    {
        if (srt->cost_size>0)
            size_cost = (l+1)*sizeof(unsigned int);
        srt_to_src = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                              (l * sizeof(struct in_addr))+size_cost,GFP_ATOMIC);
        char *aux = (char *) srt_to_src;
        aux += sizeof(struct dsr_srt);
        srt_to_src->cost=(unsigned int*)aux;
        aux +=size_cost;
        srt_to_src->addrs=(struct in_addr*)(aux);
        if (srt->cost==NULL)
        {
            srt_to_src->cost=NULL;
            srt_to_src->cost_size=0;
        }
        srt_to_src->flags = srt->flags;
    }
    if (srt_to_src)
    {
        srt_to_src->src.s_addr = src.s_addr;
        srt_to_src->dst.s_addr = srt->src.s_addr;
        srt_to_src->laddrs = sizeof(struct in_addr)*l;
        if (srt->cost_size>0)
            srt_to_src->cost_size=l+1;
        else
            srt_to_src->cost_size=0;

        for (int k=0; k<l; k++)
        {
            srt_to_src->addrs[k].s_addr = srt->addrs[l-k-1].s_addr;
            if (srt->cost_size>0)
                srt_to_src->cost[k] = srt->cost[l-k];
        }
        if (srt->cost_size>0)
            srt_to_src->cost[l] = srt->cost[0];
// Integrity
        for (int k = 0; k < l; k++)
        {
            if (src.s_addr == srt_to_src->addrs[k].s_addr) // avoid loop
            {
                int j=0;
                for (int m = k+1; m < l; m++)
                {
                    srt_to_src->addrs[j]=srt_to_src->addrs[m];
                    if (srt->cost_size>0)
                        srt_to_src->cost[j] = srt_to_src->cost[m];
                    j++;
                }
                if (srt->cost_size>0)
                {
                    srt_to_src->cost_size=l-k;
                    srt_to_src->cost[l-(k+1)] = srt_to_src->cost[l];
                }
                srt_to_src->laddrs = sizeof(struct in_addr) * (l-(k+1));
            }
        }
    }
    *srt_to_dest_ptr = srt_to_dest;
    *srt_to_src_ptr = srt_to_src;
}


#endif

struct dsr_srt *dsr_srt_new_rev(struct dsr_srt *srt)
{
    struct dsr_srt *srt_rev;
    int i, n;

    if (!srt)
        return NULL;
#ifdef OMNETPP
    int size_cost=0;
    if (srt->cost_size>0)
        size_cost=srt->cost_size*sizeof(unsigned int);
    srt_rev = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) + srt->laddrs+size_cost,
                                       GFP_ATOMIC);
    if (!srt_rev)
        return NULL;

    char *aux = (char *) srt_rev;
    aux += sizeof(struct dsr_srt);

    srt_rev->cost=(unsigned int*)aux;
    aux +=size_cost;
    srt_rev->addrs=(struct in_addr*)aux;

    srt_rev->cost_size=srt->cost_size;
    if (srt_rev->cost_size<=0)
        srt_rev->cost=NULL;

    srt_rev->flags = srt->flags;

#else
    srt_rev = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                       srt->laddrs, GFP_ATOMIC);
    if (!srt_rev)
        return NULL;
#endif

    srt_rev->src.s_addr = srt->dst.s_addr;
    srt_rev->dst.s_addr = srt->src.s_addr;
    srt_rev->laddrs = srt->laddrs;


    n = srt->laddrs / sizeof(struct in_addr);

    for (i = 0; i < n; i++)
        srt_rev->addrs[i].s_addr = srt->addrs[n - 1 - i].s_addr;
#ifdef OMNETPP
    if (srt->cost_size>0)
        for (i = 0; (unsigned int)i < srt->cost_size; i++)
            srt_rev->cost[i]= srt->cost[srt->cost_size - 1 - i];
#endif

    return srt_rev;
}

struct dsr_srt *dsr_srt_new_split(struct dsr_srt *srt, struct in_addr addr)
{
    struct dsr_srt *srt_split;
    int i, n;

    if (!srt)
        return NULL;

    n = srt->laddrs / sizeof(struct in_addr);

    if (n == 0)
        return NULL;

    for (i = 0; i < n; i++)
    {
        if (addr.s_addr == srt->addrs[i].s_addr)
            goto split;
    }
    /* Nothing to split */
    return NULL;
split:
#ifndef OMNETPP
    srt_split = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                         (i * sizeof(struct in_addr)),
                                         GFP_ATOMIC);
    if (!srt_split)
        return NULL;
#else
    int size_cost = 0;
    if (srt->cost_size>0)
        size_cost = (i+1)*sizeof(unsigned int);
    srt_split = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                         (i * sizeof(struct in_addr))+size_cost,
                                         GFP_ATOMIC);

    if (!srt_split)
        return NULL;

    char *aux = (char *) srt_split;
    aux += sizeof(struct dsr_srt);

    srt_split->cost=(unsigned int*)aux;
    aux +=size_cost;
    srt_split->addrs=(struct in_addr*)(aux);

    if (srt->cost==NULL)
    {
        srt_split->cost=NULL;
        srt_split->cost_size=0;
    }
    srt_split->flags = srt->flags;
#endif

    srt_split->src.s_addr = srt->src.s_addr;
    srt_split->dst.s_addr = srt->addrs[i].s_addr;
    srt_split->laddrs = sizeof(struct in_addr) * i;

    memcpy(srt_split->addrs, srt->addrs, sizeof(struct in_addr) * i);
#ifdef OMNETPP
    if (srt->cost_size>0)
    {
        memcpy(srt_split->cost, srt->cost, sizeof(unsigned int) * i+1);
        srt_split->cost_size=i+1;
    }
    else
        srt_split->cost_size=0;
#endif
    return srt_split;
}

struct dsr_srt *dsr_srt_new_split_rev(struct dsr_srt *srt, struct in_addr addr)
{
    struct dsr_srt *srt_split, *srt_split_rev;

    srt_split = dsr_srt_new_split(srt, addr);

    if (!srt_split)
        return NULL;

    srt_split_rev = dsr_srt_new_rev(srt_split);

    FREE(srt_split);

    return srt_split_rev;
}

struct dsr_srt *dsr_srt_shortcut(struct dsr_srt *srt, struct in_addr a1,
                                 struct in_addr a2)
{
    struct dsr_srt *srt_cut;
    int i, j, n, n_cut, a1_num, a2_num;

    if (!srt)
        return NULL;

    a1_num = a2_num = -1;

    n = srt->laddrs / sizeof(struct in_addr);

    if (srt->src.s_addr == a1.s_addr)
        a1_num = 0;

    /* Find out how between which node indexes to shortcut */
    for (i = 0; i < n; i++)
    {
        if (srt->addrs[i].s_addr == a1.s_addr)
            a1_num = i + 1;
        if (srt->addrs[i].s_addr == a2.s_addr)
            a2_num = i + 1;
    }

    if (srt->dst.s_addr == a2.s_addr)
        a2_num = i + 1;

    n_cut = n - (a2_num - a1_num - 1);


#ifndef OMNETPP
    srt_cut = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                       (n_cut*sizeof(struct in_addr)),
                                       GFP_ATOMIC);
    if (!srt_cut)
        return NULL;
#else
    int size_cost = 0;
    if (srt->cost_size>0)
        size_cost = (n_cut+1)*sizeof(unsigned int);
    srt_cut = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                       (n_cut * sizeof(struct in_addr))+size_cost,
                                       GFP_ATOMIC);
    if (!srt_cut)
        return NULL;

    char *aux = (char *) srt_cut;
    aux += sizeof(struct dsr_srt);

    srt_cut->cost=(unsigned int*)aux;
    aux +=size_cost;
    srt_cut->addrs=(struct in_addr*) aux;

    if (srt->cost_size>0)
    {
        srt_cut->cost_size=n_cut+1;
    }
    else
    {
        srt_cut->cost=NULL;
        srt_cut->cost_size=0;
    }
    srt_cut->flags = srt->flags;
#endif

    srt_cut->src = srt->src;
    srt_cut->dst = srt->dst;
    srt_cut->laddrs = n_cut * sizeof(struct in_addr);

    if (srt_cut->laddrs == 0)
        return srt_cut;

    j = 0;

    for (i = 0; i < n; i++)
    {
        if (i + 1 > a1_num && i + 1 < a2_num)
            continue;
        srt_cut->addrs[j++] = srt->addrs[i];
    }
#ifdef OMNETPP
    if (srt->cost_size>0)
    {
        j=0;
        srt_cut->cost_size=n_cut+1;
        for (i = 0; (unsigned int)i < srt->cost_size; i++)
        {
            if (i + 1 > a1_num && i + 1 < a2_num)
                continue;
            srt_cut->cost[j++] = srt->cost[i];
        }
    }
    else
        srt_cut->cost_size=0;
#endif
    return srt_cut;
}

struct dsr_srt *dsr_srt_concatenate(struct dsr_srt *srt1, struct dsr_srt *srt2)
{
    struct dsr_srt *srt_cat;
    int n, n1, n2;

    if (!srt1 || !srt2)
        return NULL;

    n1 = srt1->laddrs / sizeof(struct in_addr);
    n2 = srt2->laddrs / sizeof(struct in_addr);

    /* We assume that the end node of the first srt is the same as the start
     * of the second. We therefore only count that node once. */
    n = n1 + n2 + 1;


#ifndef OMNETPP
    srt_cat = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                       (n * sizeof(struct in_addr)),
                                       GFP_ATOMIC);
    if (!srt_cat)
        return NULL;
#else
    int size_cost = 0;
    if (srt1->cost_size>0 || srt2->cost_size>0)
        size_cost = (srt1->cost_size+srt2->cost_size)*sizeof(unsigned int);
    srt_cat = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                       (n * sizeof(struct in_addr))+size_cost,
                                       GFP_ATOMIC);
    if (!srt_cat)
        return NULL;

    char *aux = (char *) srt_cat;
    aux += sizeof(struct dsr_srt);

    srt_cat->cost=(unsigned int*)aux;
    aux +=size_cost;
    srt_cat->addrs=(struct in_addr*)aux;
    if (size_cost==0)
    {
        srt_cat->cost=NULL;
        srt_cat->cost_size=0;
    }
    else
        srt_cat->cost_size=srt1->cost_size+srt2->cost_size;
    srt_cat->flags = srt1->flags & srt2->flags;
#endif

    srt_cat->src = srt1->src;
    srt_cat->dst = srt2->dst;
    srt_cat->laddrs = n * sizeof(struct in_addr);

    memcpy(srt_cat->addrs, srt1->addrs, n1 * sizeof(struct in_addr));
    memcpy(srt_cat->addrs + n1, &srt2->src, sizeof(struct in_addr));
    memcpy(srt_cat->addrs + n1 + 1, srt2->addrs, n2 * sizeof(struct in_addr));
#ifdef OMNETPP
    if (size_cost>0)
    {
        memcpy(srt_cat->cost, srt1->cost, srt1->cost_size * sizeof(unsigned int));
        memcpy(srt_cat->cost + srt1->cost_size, srt2->cost, srt2->cost_size * sizeof(unsigned int));
    }
#endif
    return srt_cat;
}


int dsr_srt_check_duplicate(struct dsr_srt *srt)
{
    struct in_addr *buf;
    int n, i, res = 0;

    n = srt->laddrs / sizeof(struct in_addr);

    buf = (struct in_addr *)MALLOC(sizeof(struct in_addr) * (n + 1),
                                   GFP_ATOMIC);

    if (!buf)
        return -1;

    buf[0] = srt->src;

    for (i = 0; i < n; i++)
    {
        int j;

        for (j = 0; j < i + 1; j++)
            if (buf[j].s_addr == srt->addrs[i].s_addr)
            {
                res = 1;
                goto out;
            }
        buf[i+1] = srt->addrs[i];
    }

    for (i = 0; i < n + 1; i++)
        if (buf[i].s_addr == srt->dst.s_addr)
        {
            res = 1;
            goto out;
        }
out:
    FREE(buf);

    return res;
}
struct dsr_srt_opt *dsr_srt_opt_add(char *buf, int len, int flags,
                                    int salvage, struct dsr_srt *srt)
{
    struct dsr_srt_opt *srt_opt;

    if (len < (int)DSR_SRT_OPT_LEN(srt))
        return NULL;

    srt_opt = (struct dsr_srt_opt *)buf;

    srt_opt->type = DSR_OPT_SRT;
    srt_opt->length = srt->laddrs + 2;
    srt_opt->f = (flags & SRT_FIRST_HOP_EXT) ? 1 : 0;
    srt_opt->l = (flags & SRT_LAST_HOP_EXT) ? 1 : 0;
    srt_opt->res = 0;
    srt_opt->salv = salvage;
    srt_opt->sleft = (srt->laddrs / sizeof(struct in_addr));

    memcpy(srt_opt->addrs, srt->addrs, srt->laddrs);

    return srt_opt;
}

int NSCLASS dsr_srt_add(struct dsr_pkt *dp)
{
    char *buf;
    int n, len, ttl, tot_len, ip_len;
    int prot = 0;

    if (!dp || !dp->srt)
        return -1;

    n = dp->srt->laddrs / sizeof(struct in_addr);

    dp->nxt_hop = dsr_srt_next_hop(dp->srt, n);

    /* Calculate extra space needed */

    len = DSR_OPT_HDR_LEN + DSR_SRT_OPT_LEN(dp->srt);

    DEBUG("SR: %s\n", print_srt(dp->srt));

    buf = dsr_pkt_alloc_opts(dp, len);

    if (!buf)
    {
        /*      DEBUG("Could allocate memory\n"); */
        return -1;
    }
#ifdef NS2
    if (dp->p)
    {
        hdr_cmn *cmh = HDR_CMN(dp->p);
        prot = cmh->ptype();
    }
    else
        prot = PT_NTYPE;

    ip_len = IP_HDR_LEN;
    tot_len = dp->payload_len + ip_len + len;
    ttl = dp->nh.iph->ttl();
#else
    prot = dp->nh.iph->protocol;
    ip_len = (dp->nh.iph->ihl << 2);
    tot_len = ntohs(dp->nh.iph->tot_len) + len;
    ttl = dp->nh.iph->ttl;
#endif
#ifdef OMNETPP
    ip_len = dp->nh.iph->ihl;
#endif
    dp->nh.iph = dsr_build_ip(dp, dp->src, dp->dst, ip_len, tot_len,
                              IPPROTO_DSR, ttl);

    if (!dp->nh.iph)
        return -1;

    dp->dh.opth = dsr_opt_hdr_add(buf, len, prot);

    if (!dp->dh.opth)
    {
        /*      DEBUG("Could not create DSR opts header!\n"); */
        return -1;
    }

    buf += DSR_OPT_HDR_LEN;
    len -= DSR_OPT_HDR_LEN;

    dp->srt_opt = dsr_srt_opt_add(buf, len, 0, dp->salvage, dp->srt);

    if (!dp->srt_opt)
    {
        /*      DEBUG("Could not create Source Route option header!\n"); */
        return -1;
    }

    buf += DSR_SRT_OPT_LEN(dp->srt);
    len -= DSR_SRT_OPT_LEN(dp->srt);

    return 0;
}

int NSCLASS dsr_srt_opt_recv(struct dsr_pkt *dp, struct dsr_srt_opt *srt_opt)
{
    struct in_addr next_hop_intended;
    struct in_addr myaddr = my_addr();
    int n;
    int i;
    unsigned int j;

    if (!dp || !srt_opt)
        return DSR_PKT_ERROR;

    dp->srt_opt = srt_opt;

    /* We should add this source route info to the cache... */
#ifdef OMNETPP
    dp->srt = dsr_srt_new(dp->src, dp->dst, srt_opt->length,
                          (char *)srt_opt->addrs,dp->costVector,dp->costVectorSize);
#else
    dp->srt = dsr_srt_new(dp->src, dp->dst, srt_opt->length,
                          (char *)srt_opt->addrs);
#endif
    if (!dp->srt)
    {
        DEBUG("Create source route failed\n");
        return DSR_PKT_ERROR;
    }
    n = dp->srt->laddrs / sizeof(struct in_addr);

    DEBUG("SR: %s sleft=%d\n", print_srt(dp->srt), srt_opt->sleft);

    /* Copy salvage field */
    dp->salvage = dp->srt_opt->salv;

    next_hop_intended = dsr_srt_next_hop(dp->srt, srt_opt->sleft);
    dp->prv_hop = dsr_srt_prev_hop(dp->srt, srt_opt->sleft - 1);
    dp->nxt_hop = dsr_srt_next_hop(dp->srt, srt_opt->sleft - 1);

    DEBUG("next_hop=%s prev_hop=%s next_hop_intended=%s\n",
          print_ip(dp->nxt_hop),
          print_ip(dp->prv_hop), print_ip(next_hop_intended));

    neigh_tbl_add(dp->prv_hop, dp->mac.ethh);


#ifdef OMNETPP
    if (ConfVal(PathCache))
    {
        if (etxActive)
        {
            ph_srt_add_node (dp->prv_hop,ConfValToUsecs(RouteCacheTimeout), 0,(unsigned int)getCost(IPv4Address((uint32_t)dp->prv_hop.s_addr)));
        }
        else
            ph_srt_add_node (dp->prv_hop,ConfValToUsecs(RouteCacheTimeout), 0,0);

        struct dsr_srt * from_me_to_dest=NULL;
        struct dsr_srt * from_me_to_src=NULL;
        struct in_addr split_add;

        if (next_hop_intended.s_addr != myaddr.s_addr)
            split_add.s_addr=dp->prv_hop.s_addr;
        else
        {
            split_add.s_addr=myaddr.s_addr;
            //dsr_srt_split_both(dp->srt,split_add,myaddr,&from_me_to_dest,&from_me_to_src);// Split vector to dest and to source
        }
        // if you want that the procotocol can't learn in promiscuous mode comment ths line and uncomment previous
        dsr_srt_split_both(dp->srt,split_add,myaddr,&from_me_to_dest,&from_me_to_src);// Split vector to dest and to source
        if (from_me_to_dest)
        {
            dsr_rtc_add(from_me_to_dest, ConfValToUsecs(RouteCacheTimeout), 0);
            FREE(from_me_to_dest);
        }
        if (from_me_to_src)
        {
            dsr_rtc_add(from_me_to_src, ConfValToUsecs(RouteCacheTimeout), 0);
            FREE(from_me_to_src);
        }

    }
    else
    {
        if (etxActive)
        {
            lc_link_add(my_addr(), dp->prv_hop,
                        ConfValToUsecs(RouteCacheTimeout), 0, (unsigned int)getCost(IPv4Address((uint32_t)dp->prv_hop.s_addr)));
        }
        else
            lc_link_add(my_addr(), dp->prv_hop,
                        ConfValToUsecs(RouteCacheTimeout), 0, 1);

        dsr_rtc_add(dp->srt, ConfValToUsecs(RouteCacheTimeout), 0);
    }

#else
    lc_link_add(my_addr(), dp->prv_hop,
                ConfValToUsecs(RouteCacheTimeout), 0, 1);

    dsr_rtc_add(dp->srt, ConfValToUsecs(RouteCacheTimeout), 0);
#endif




    /* Automatic route shortening - Check if this node is the
     * intended next hop. If not, is it part of the remaining
     * source route? */
    if (next_hop_intended.s_addr != myaddr.s_addr &&
            dsr_srt_find_addr(dp->srt, myaddr, srt_opt->sleft) &&
            !grat_rrep_tbl_find(dp->src, dp->prv_hop))
    {
        struct dsr_srt *srt, *srt_cut;

        /* Send Grat RREP */
        DEBUG("Send Gratuitous RREP to %s\n", print_ip(dp->src));

        srt_cut = dsr_srt_shortcut(dp->srt, dp->prv_hop, myaddr);

        if (!srt_cut)
            return DSR_PKT_DROP;
#ifdef OMNETPP
        if (etxActive)
        {
            j=0;
            for (i=0 ; (unsigned int)i< (srt_cut->laddrs / sizeof(struct in_addr)); i++)
            {
                j=i+1;
                if (myaddr.s_addr==srt_cut->addrs[i].s_addr)
                    break;
            }
            if (srt_cut->cost_size>j)
                srt_cut->cost[j] = (unsigned int)getCost(IPv4Address((uint32_t)dp->prv_hop.s_addr));
        }
#endif

        DEBUG("shortcut: %s\n", print_srt(srt_cut));

        /* srt = dsr_rtc_find(myaddr, dp->src); */
        if (srt_cut->laddrs / sizeof(struct in_addr) == 0)
            srt = dsr_srt_new_rev(srt_cut);
        else
            srt = dsr_srt_new_split_rev(srt_cut, myaddr);

        if (!srt)
        {
            DEBUG("No route to %s\n", print_ip(dp->src));
            FREE(srt_cut);
            return DSR_PKT_DROP;
        }
        DEBUG("my srt: %s\n", print_srt(srt));

        grat_rrep_tbl_add(dp->src, dp->prv_hop);

        dsr_rrep_send(srt, srt_cut);

        FREE(srt_cut);
        FREE(srt);
    }

    if (dp->flags & PKT_PROMISC_RECV)
        return DSR_PKT_DROP;

    if (srt_opt->sleft == 0)
        return DSR_PKT_SRT_REMOVE;

    if (srt_opt->sleft > n)
    {
        // Send ICMP parameter error
        return DSR_PKT_SEND_ICMP;
    }

    srt_opt->sleft--;

#ifdef OMNETPP
    if (etxActive)
    {
        j=0;

        for (i=0 ; i< dp->costVectorSize; i++)
        {
            if (myaddr.s_addr==dp->costVector[i].address.getInt())
            {
                if (i==0)
                    dp->costVector[i].cost = getCost(IPv4Address((uint32_t)dp->src.s_addr));
                else
                    dp->costVector[i].cost = getCost(dp->costVector[i-1].address);
                break;
            }
        }
    }
#endif

    /* TODO: check for multicast address in next hop or dst */
    /* TODO: check MTU and compare to pkt size */

    return DSR_PKT_FORWARD;
}
