/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */

#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"
#include "inet/routing/extras/dsr/dsr-uu/debug_dsr.h"

#define SIZE_ADDRESS DSR_ADDRESS_SIZE
namespace inet {

namespace inetmanet {

struct in_addr NSCLASS dsr_srt_next_hop(struct dsr_srt *srt, int sleft)
{
    int n = srt->laddrs / SIZE_ADDRESS;
    struct in_addr nxt_hop;

    if (sleft <= 0)
        nxt_hop = srt->dst;
    else
        nxt_hop = srt->addrs[n - sleft];

    return nxt_hop;
}

struct in_addr NSCLASS dsr_srt_prev_hop(struct dsr_srt *srt, int sleft)
{
    struct in_addr prev_hop;
    int n = srt->laddrs / SIZE_ADDRESS;

    if (n - 1 == sleft)
        prev_hop = srt->src;
    else
        prev_hop = srt->addrs[n - 2 - (sleft)];

    return prev_hop;
}

static int dsr_srt_find_addr(struct dsr_srt *srt, struct in_addr addr,
                             int sleft)
{
    int n = srt->laddrs / SIZE_ADDRESS;

    if (n == 0 || sleft > n || sleft < 1)
        return 0;

    for (; sleft > 0; sleft--)
        if (srt->addrs[n - sleft].s_addr == addr.s_addr)
            return 1;

    if (addr.s_addr == srt->dst.s_addr)
        return 1;

    return 0;
}

struct dsr_srt * NSCLASS dsr_srt_new(struct in_addr src, struct in_addr dst,
                            unsigned int length, const VectorAddress &addrs,const std::vector<EtxCost>&cost)
{
    struct dsr_srt *sr;
    int sizeAdd;
    bool testInverse=false;

    sizeAdd =  addrs.size();

    sr = new dsr_srt;

    if (!sr)
        return nullptr;

    sr->src.s_addr = src.s_addr;
    sr->dst.s_addr = dst.s_addr;
    sr->laddrs = length;

    /*  sr->index = index; */
    L3Address addrs1;
    L3Address addrs2;
    sr->addrs.resize(addrs.size());
    for (unsigned int i=0; i<addrs.size(); i++)
    {
        sr->addrs[i].s_addr = addrs[i];
    }

    if (!cost.empty()) // Integrity test
    {
        if (sizeAdd == (int)(cost.size()-1))
        {
            int size = sizeAdd;
            for (int i=0; i<size; i++)
            {
                addrs1 = cost[i].address;
                addrs2 = sr->addrs[i].s_addr;
                if (addrs2 != addrs1)
                {
                    testInverse = true;
                    break;
                }
            }
            if (!testInverse)
            {
                for (unsigned int i=0; i<cost.size(); i++)
                   sr->cost.push_back((unsigned int) cost[i].cost);
            }
            else
            {
                for (unsigned int i=0; i<cost.size(); i++)
                   sr->cost.push_back((unsigned int)cost[cost.size()-1-i].cost);

                for (int i=0; i<size; i++)
                {

                    if (cost[cost.size()-1].address == src.s_addr || cost[cost.size()-1].address == dst.s_addr )
                        addrs1 = cost[cost.size()-2-i].address;
                    else
                        addrs1 = cost[cost.size()-1-i].address;
                    addrs2 = sr->addrs[i].s_addr;
                    if (addrs2 != addrs1)
                        throw cRuntimeError("Dsr error, Etx address and dsr are different");
                }
            }
        }
        else
        {
            sr->cost.resize(sizeAdd+1);
            int j=-1;
            for (unsigned int i=0; i<cost.size(); i++)
            {

                if (cost[i].address == src.s_addr)
                {
                    j=i;
                    break;
                }
            }
            if (j==-1)
                throw cRuntimeError("Dsr error, Etx address and dsr are different");
            else
            {
                testInverse=false;
                int l=0;

                for (int i=j-1; i>=0; i--)
                {

                    addrs1 = cost[i].address;
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
                        addrs1 = cost[j+1+i].address;
                        addrs2 = sr->addrs[i].s_addr;
                        if (addrs2 != addrs1)
                            throw cRuntimeError("Dsr error, Etx address and dsr are different");
                    }
                    for (int i=0; i<sizeAdd+1; i++)
                        sr->cost[i]= (unsigned int) cost[j+1+i].cost;
                }
            }
        }
    }
#ifdef BIDIR
    sr->flags |= SRT_BIDIR;
#endif
    return sr;
}


void NSCLASS dsr_srt_split_both(struct dsr_srt *srt, struct in_addr addr, struct in_addr src,struct dsr_srt **srt_to_dest_ptr,struct dsr_srt **srt_to_src_ptr)
{
    int i, n,l;
    bool split;

    struct dsr_srt *srt_to_dest = nullptr;
    struct dsr_srt *srt_to_src = nullptr;
    *srt_to_dest_ptr = srt_to_dest;
    *srt_to_src_ptr = srt_to_src;
    if (!srt)
        return;
    n = srt->laddrs / SIZE_ADDRESS;

    if (n != (int)srt->addrs.size())
        throw cRuntimeError("size mismatch ");

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

    if (n-l-1>0)
    {
       // srt_to_dest = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
       //                                        ((n-l-1) * sizeof(struct in_addr))+size_cost,GFP_ATOMIC);
        srt_to_dest = new dsr_srt;
        if (srt->cost.empty())
        {
            srt_to_dest->cost.clear();
        }
        srt_to_dest->flags = srt->flags;
    }

    if (srt_to_dest)
    {
        srt_to_dest->src.s_addr = src.s_addr;
        srt_to_dest->dst.s_addr = srt->dst.s_addr;
        srt_to_dest->laddrs = SIZE_ADDRESS * (n-l-1);

        if (addr.s_addr ==srt->src.s_addr && addr.s_addr != src.s_addr)
        {
            srt_to_dest->addrs.push_back(srt->src);
            for (unsigned int i = 0; i< srt->addrs.size(); i++)
                srt_to_dest->addrs.push_back(srt->addrs[i]);

            if (!srt->cost.empty())
            {
                srt_to_dest->cost.push_back(1);
                for (unsigned int i = 0; i<srt->cost.size(); i++)
                    srt_to_dest->cost.push_back(srt->cost[i]);
            }
        }
        else
        {
            for (unsigned int i = l+1; i < srt->addrs.size(); i++)
                srt_to_dest->addrs.push_back(srt->addrs[i]);

            if (!srt->cost.empty())
            {
                for (unsigned int i = l+1; i < srt->cost.size(); i++)
                    srt_to_dest->cost.push_back(srt->cost[i]);
            }
        }

        for (int k = 0; k < (n-l-1); k++)
        {
            if (src.s_addr == srt_to_dest->addrs[k].s_addr) // avoid loop
            {
                int j=0;
                for (int m = k+1; m < (n-l-1); m++)
                {
                    srt_to_dest->addrs[j] = srt_to_dest->addrs[m];
                    if (!srt->cost.empty())
                        srt_to_dest->cost[j] = srt_to_dest->cost[m];
                    j++;
                }
                if (!srt->cost.empty())
                {
                    srt_to_dest->cost.resize(n-l-(k+1));
                    srt_to_dest->cost[n-l-(k+1)] = srt_to_dest->cost[n-l];
                }
                srt_to_dest->addrs.resize(n-l-1-(k+1));
                srt_to_dest->laddrs = SIZE_ADDRESS * (n-l-1-(k+1));
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

    if (l>0)
    {
        //srt_to_src = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
        //                                      (l * sizeof(struct in_addr))+size_cost,GFP_ATOMIC);
        srt_to_src = new dsr_srt;

        if (srt->cost.empty())
            srt_to_src->cost.clear();
        srt_to_src->flags = srt->flags;
    }
    if (srt_to_src)
    {
        srt_to_src->src.s_addr = src.s_addr;
        srt_to_src->dst.s_addr = srt->src.s_addr;
        srt_to_src->laddrs = SIZE_ADDRESS * l;

        for (int k=0; k<l; k++)
        {
            srt_to_src->addrs.push_back(srt->addrs[l-k-1]);
            if (!srt->cost.empty())
                srt_to_src->cost.push_back(srt->cost[l-k]);
        }

        if (!srt->cost.empty())
            srt_to_src->cost.push_back(srt->cost[0]);

// Integrity
        for (int k = 0; k < l; k++)
        {
            if (src.s_addr == srt_to_src->addrs[k].s_addr) // avoid loop
            {
                int j=0;
                for (int m = k+1; m < l; m++)
                {
                    srt_to_src->addrs[j]=srt_to_src->addrs[m];
                    if (!srt->cost.empty())
                        srt_to_src->cost[j] = srt_to_src->cost[m];
                    j++;
                }
                if (!srt->cost.empty())
                {
                    srt_to_src->cost[l-(k+1)] = srt_to_src->cost[l];
                }
                srt_to_src->laddrs = SIZE_ADDRESS * (l-(k+1));
            }
        }
    }
    *srt_to_dest_ptr = srt_to_dest;
    *srt_to_src_ptr = srt_to_src;
}


struct dsr_srt * NSCLASS dsr_srt_new_rev(struct dsr_srt *srt)
{
    struct dsr_srt *srt_rev;
    int i, n;

    if (!srt)
        return nullptr;

    srt_rev = new dsr_srt;
    if (!srt_rev)
        return nullptr;
    srt_rev->flags = srt->flags;

    srt_rev->src.s_addr = srt->dst.s_addr;
    srt_rev->dst.s_addr = srt->src.s_addr;
    srt_rev->laddrs = srt->laddrs;


    n = srt->laddrs / SIZE_ADDRESS;

    if (n != (int)srt->addrs.size())
        throw cRuntimeError("size mismatch ");

    for (i = 0; i < n; i++)
        srt_rev->addrs.push_back(srt->addrs[n - 1 - i]);
#ifdef OMNETPP
    if (srt->cost.size()>0)
    {
        for (i = 0; (unsigned int)i < srt->cost.size(); i++)
            srt_rev->cost.push_back(srt->cost[srt->cost.size() - 1 - i]);
    }
#endif

    return srt_rev;
}

struct dsr_srt * NSCLASS dsr_srt_new_split(struct dsr_srt *srt, struct in_addr addr)
{
    struct dsr_srt *srt_split;
    int i, n;

    if (!srt)
        return nullptr;

    n = srt->laddrs / SIZE_ADDRESS;

    if (n != (int)srt->addrs.size())
        throw cRuntimeError("size mismatch ");

    if (n == 0)
        return nullptr;

    for (i = 0; i < n; i++)
    {
        if (addr.s_addr == srt->addrs[i].s_addr)
            goto split;
    }
    /* Nothing to split */
    return nullptr;
split:
    srt_split = new dsr_srt;

    if (!srt_split)
        return nullptr;

    srt_split->flags = srt->flags;

    srt_split->src.s_addr = srt->src.s_addr;
    srt_split->dst.s_addr = srt->addrs[i].s_addr;
    srt_split->laddrs = SIZE_ADDRESS * i;

    for (int pos = 0; pos < i ;pos++)
        srt_split->addrs.push_back(srt->addrs[pos]);

    if (!srt->cost.empty())
    {
        for (int pos = 0; pos <= i ;pos++)
            srt_split->cost.push_back(srt->cost[pos]);
    }
    return srt_split;
}

struct dsr_srt * NSCLASS dsr_srt_new_split_rev(struct dsr_srt *srt, struct in_addr addr)
{
    struct dsr_srt *srt_split, *srt_split_rev;

    srt_split = dsr_srt_new_split(srt, addr);

    if (!srt_split)
        return nullptr;

    srt_split_rev = dsr_srt_new_rev(srt_split);

    delete srt_split;

    return srt_split_rev;
}

struct dsr_srt * NSCLASS dsr_srt_shortcut(struct dsr_srt *srt, struct in_addr a1,
                                 struct in_addr a2)
{
    struct dsr_srt *srt_cut;
    int i, n, n_cut, a1_num, a2_num;

    if (!srt)
        return nullptr;

    a1_num = a2_num = -1;

    n = srt->laddrs / SIZE_ADDRESS;

    if (n != (int)srt->addrs.size())
        throw cRuntimeError("size mismatch ");

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


    srt_cut = new dsr_srt;

    if (!srt_cut)
        return nullptr;


    srt_cut->flags = srt->flags;

    srt_cut->src = srt->src;
    srt_cut->dst = srt->dst;
    srt_cut->laddrs = n_cut * SIZE_ADDRESS;

    if (srt_cut->laddrs == 0)
        return srt_cut;

    for (i = 0; i < n; i++)
    {
        if (i + 1 > a1_num && i + 1 < a2_num)
            continue;
        srt_cut->addrs.push_back(srt->addrs[i]);
    }
    if ((int)srt_cut->addrs.size() != n_cut)
        throw cRuntimeError("DSR src cut error ");
#ifdef OMNETPP
    if (!srt->cost.empty())
    {
        for (int i = 0; i < (int)srt->cost.size(); i++)
        {
            if (i + 1 > a1_num && i + 1 < a2_num)
                continue;
            srt_cut->cost.push_back(srt->cost[i]);
        }
    }
#endif
    return srt_cut;
}

struct dsr_srt * NSCLASS dsr_srt_concatenate(struct dsr_srt *srt1, struct dsr_srt *srt2)
{
    struct dsr_srt *srt_cat;
    int n, n1, n2;

    if (!srt1 || !srt2)
        return nullptr;

    n1 = srt1->laddrs / SIZE_ADDRESS;
    if (n1 != (int)srt1->addrs.size())
        throw cRuntimeError("size mismatch ");
    n2 = srt2->laddrs / SIZE_ADDRESS;
    if (n2 != (int)srt2->addrs.size())
        throw cRuntimeError("size mismatch ");

    /* We assume that the end node of the first srt is the same as the start
     * of the second. We therefore only count that node once. */
    n = n1 + n2 + 1;

    int size_cost = 0;

    srt_cat = new dsr_srt;

    if (!srt_cat)
        return nullptr;

    srt_cat->flags = srt1->flags & srt2->flags;

    srt_cat->src = srt1->src;
    srt_cat->dst = srt2->dst;
    srt_cat->laddrs = n * SIZE_ADDRESS;

    for (unsigned int i = 0; i<srt1->addrs.size();i++)
        srt_cat->addrs.push_back(srt1->addrs[i]);
    srt_cat->addrs.push_back(srt2->src);

    for (unsigned int i = 0; i<srt2->addrs.size();i++)
        srt_cat->addrs.push_back(srt2->addrs[i]);

    if (n != (int)srt_cat->addrs.size())
        throw cRuntimeError("size mismatch ");
#ifdef OMNETPP
    if (size_cost>0)
    {
        for (unsigned int i = 0; i<srt1->addrs.size();i++)
            srt_cat->cost.push_back(srt1->cost[i]);
        for (unsigned int i = 0; i<srt2->addrs.size();i++)
            srt_cat->cost.push_back(srt2->cost[i]);
    }
#endif
    return srt_cat;
}


int NSCLASS dsr_srt_check_duplicate(struct dsr_srt *srt)
{
    struct in_addr *buf;
    int n, i, res = 0;

    n = srt->laddrs / SIZE_ADDRESS;

    buf = new in_addr[n + 1];

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
    delete [] buf;

    return res;
}

struct dsr_srt_opt * NSCLASS dsr_srt_opt_add(struct dsr_opt_hdr *opt_hdr, int len, int flags,
                                    int salvage, struct dsr_srt *srt)
{
    struct dsr_srt_opt *srt_opt;

    if (len < (int)DSR_SRT_OPT_LEN(srt))
        return nullptr;

    srt_opt = new dsr_srt_opt ();

    srt_opt->type = DSR_OPT_SRT;
    srt_opt->length = srt->laddrs + 2;
    srt_opt->f = (flags & SRT_FIRST_HOP_EXT) ? 1 : 0;
    srt_opt->l = (flags & SRT_LAST_HOP_EXT) ? 1 : 0;
    srt_opt->res = 0;
    srt_opt->salv = salvage;
    srt_opt->sleft = (srt->laddrs / SIZE_ADDRESS);

    std::vector<unsigned int> cost;
    for (unsigned int i = 0; i<srt->addrs.size();i++)
        srt_opt->addrs.push_back(srt->addrs[i].s_addr);

    opt_hdr->option.push_back(srt_opt);
    return srt_opt;
}


struct dsr_srt_opt * NSCLASS dsr_srt_opt_add_char(char *buf, int len, int flags,
                                    int salvage, struct dsr_srt *srt)
{
        struct dsr_srt_opt *srt_opt;

        if (len < (int)DSR_SRT_OPT_LEN(srt))
            return nullptr;

        srt_opt = (struct dsr_srt_opt *)buf;

        srt_opt->type = DSR_OPT_SRT;
        srt_opt->length = srt->laddrs + 2;
        srt_opt->f = (flags & SRT_FIRST_HOP_EXT) ? 1 : 0;
        srt_opt->l = (flags & SRT_LAST_HOP_EXT) ? 1 : 0;
        srt_opt->res = 0;
        srt_opt->salv = salvage;
        srt_opt->sleft = (srt->laddrs / SIZE_ADDRESS);
        srt_opt->addrs.clear();
        for (unsigned int i = 0; i<srt->addrs.size();i++)
            srt_opt->addrs.push_back(srt->addrs[i].s_addr);
        return srt_opt;
}

int NSCLASS dsr_srt_add(struct dsr_pkt *dp)
{
    struct dsr_opt_hdr *buf;
    int n, len, ttl, tot_len, ip_len;
    int prot = 0;

    if (!dp || !dp->srt)
        return -1;

    n = dp->srt->laddrs / SIZE_ADDRESS;

    dp->nxt_hop = dsr_srt_next_hop(dp->srt, n);

    /* Calculate extra space needed */

    len = DSR_OPT_HDR_LEN + DSR_SRT_OPT_LEN(dp->srt);

    DEBUG("SR: %s\n", print_srt(dp->srt));

    buf = dsr_pkt_alloc_opts(dp);

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

    dsr_opt_hdr_add(buf, len, prot);

    if (dp->dh.opth.empty())
    {
        /*      DEBUG("Could not create DSR opts header!\n"); */
        return -1;
    }

    len -= DSR_OPT_HDR_LEN;
    dp->dh.opth.begin()->p_len = len;

    dp->srt_opt = dsr_srt_opt_add(buf, len, 0, dp->salvage, dp->srt);

    if (!dp->srt_opt)
    {
        /*      DEBUG("Could not create Source Route option header!\n"); */
        return -1;
    }
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
                          srt_opt->addrs,dp->costVector);
#else
    dp->srt = dsr_srt_new(dp->src, dp->dst, srt_opt->length,
                          (char *)srt_opt->addrs);
#endif
    if (!dp->srt)
    {
        DEBUG("Create source route failed\n");
        return DSR_PKT_ERROR;
    }
    n = dp->srt->laddrs / SIZE_ADDRESS;

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
            ph_srt_add_node_map(dp->prv_hop,ConfValToUsecs(RouteCacheTimeout), 0,(unsigned int)getCost(dp->prv_hop.s_addr.toIpv4()));
        }
        else
            ph_srt_add_node_map(dp->prv_hop,ConfValToUsecs(RouteCacheTimeout), 0,0);

        struct dsr_srt * from_me_to_dest = nullptr;
        struct dsr_srt * from_me_to_src = nullptr;
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
            /* Send buffered packets and cancel discovery*/
            /*
            for (unsigned int i = 0; i < from_me_to_dest->addrs.size(); i++ )
            {
                send_buf_set_verdict(SEND_BUF_SEND, from_me_to_dest->addrs[i]);
                rreq_tbl_route_discovery_cancel(from_me_to_dest->addrs[i]);
            }
            send_buf_set_verdict(SEND_BUF_SEND, from_me_to_dest->dst);
            rreq_tbl_route_discovery_cancel(from_me_to_dest->dst);
            */
            delete from_me_to_dest;
        }
        if (from_me_to_src)
        {
            dsr_rtc_add(from_me_to_src, ConfValToUsecs(RouteCacheTimeout), 0);
            /*
            for (unsigned int i = 0; i < from_me_to_src->addrs.size(); i++ )
            {
                send_buf_set_verdict(SEND_BUF_SEND, from_me_to_src->addrs[i]);
                rreq_tbl_route_discovery_cancel(from_me_to_src->addrs[i]);
            }
            send_buf_set_verdict(SEND_BUF_SEND, from_me_to_src->dst);
            rreq_tbl_route_discovery_cancel(from_me_to_src->dst);
            */
            delete from_me_to_src;
        }

    }
    else
    {
        if (etxActive)
        {
            ph_add_link_map(my_addr(), dp->prv_hop,
                        ConfValToUsecs(RouteCacheTimeout), 0, (unsigned int)getCost(dp->prv_hop.s_addr.toIpv4()));
        }
        else
            ph_add_link_map(my_addr(), dp->prv_hop,
                        ConfValToUsecs(RouteCacheTimeout), 0, 1);

        for (unsigned int i = 0; i < dp->srt->addrs.size(); i++ )
        {
            send_buf_set_verdict(SEND_BUF_SEND, dp->srt->addrs[i]);
            rreq_tbl_route_discovery_cancel(dp->srt->addrs[i]);
        }

        send_buf_set_verdict(SEND_BUF_SEND, dp->srt->src);
        rreq_tbl_route_discovery_cancel(dp->srt->src);

        send_buf_set_verdict(SEND_BUF_SEND, dp->srt->dst);
        rreq_tbl_route_discovery_cancel(dp->srt->dst);


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
            for (i=0 ; (unsigned int)i< (srt_cut->laddrs / SIZE_ADDRESS); i++)
            {
                j=i+1;
                if (myaddr.s_addr==srt_cut->addrs[i].s_addr)
                    break;
            }
            if (srt_cut->cost.size()>j)
                srt_cut->cost[j] = (unsigned int)getCost(dp->prv_hop.s_addr.toIpv4());
        }
#endif

        DEBUG("shortcut: %s\n", print_srt(srt_cut));

        /* srt = dsr_rtc_find(myaddr, dp->src); */
        if (srt_cut->laddrs / SIZE_ADDRESS == 0)
            srt = dsr_srt_new_rev(srt_cut);
        else
            srt = dsr_srt_new_split_rev(srt_cut, myaddr);

        if (!srt)
        {
            DEBUG("No route to %s\n", print_ip(dp->src));
            delete srt_cut;
            return DSR_PKT_DROP;
        }
        DEBUG("my srt: %s\n", print_srt(srt));

        grat_rrep_tbl_add(dp->src, dp->prv_hop);

        dsr_rrep_send(srt, srt_cut);

        delete srt_cut;
        delete srt;
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

        for (unsigned int i=0 ; i< dp->costVector.size(); i++)
        {
            if (myaddr.s_addr == dp->costVector[i].address)
            {
                if (i==0)
                    dp->costVector[i].cost = getCost(dp->src.s_addr.toIpv4());
                else
                    dp->costVector[i].cost = getCost(dp->costVector[i-1].address.toIpv4());
                break;
            }
        }
    }
#endif

    /* TODO: check for multicast address in next hop or dst */
    /* TODO: check MTU and compare to pkt size */

    return DSR_PKT_FORWARD;
}
} // namespace inetmanet

} // namespace inet

