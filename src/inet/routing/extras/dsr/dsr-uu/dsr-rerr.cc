/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"
#include "inet/routing/extras/dsr/dsr-uu/debug_dsr.h"

namespace inet {

namespace inetmanet {

static struct dsr_rerr_opt *dsr_rerr_opt_add(struct dsr_opt_hdr *buf, int len,
        int err_type,
        struct in_addr err_src,
        struct in_addr err_dst,
        struct in_addr unreach_addr,
        int salv)
{
    struct dsr_rerr_opt *rerr_opt;

    if (!buf || len < (int)DSR_RERR_HDR_LEN)
        return nullptr;

    rerr_opt = new dsr_rerr_opt;

    rerr_opt->type = DSR_OPT_RERR;
    rerr_opt->length = DSR_RERR_OPT_LEN;
    rerr_opt->err_type = err_type;
    rerr_opt->err_src = err_src.s_addr;
    rerr_opt->err_dst = err_dst.s_addr;
    rerr_opt->res = 0;
    rerr_opt->salv = salv;

    switch (err_type)
    {
    case NODE_UNREACHABLE:
        if (len < (int)(DSR_RERR_HDR_LEN + 4))
            return nullptr;
        rerr_opt->length += 4;
        //rerr_opt->info.resize(sizeof(unreach_addr));
        //memcpy(&rerr_opt->info[0], &unreach_addr, sizeof(unreach_addr));
        rerr_opt->info =  unreach_addr.s_addr;
        break;
    case FLOW_STATE_NOT_SUPPORTED:
        break;
    case OPTION_NOT_SUPPORTED:
        break;
    }
    buf->option.push_back(rerr_opt);
    return rerr_opt;
}

int NSCLASS dsr_rerr_send(struct dsr_pkt *dp_trigg, struct in_addr unr_addr)
{
    struct dsr_pkt *dp;
    struct dsr_rerr_opt *rerr_opt;
    struct in_addr dst, err_src, err_dst, myaddr, unr_dst;
    struct dsr_opt_hdr *buf;
    int n, len, i;

    myaddr = my_addr();

    if (!dp_trigg || dp_trigg->src.s_addr == myaddr.s_addr)
        return -1;

    if (!dp_trigg->srt_opt)
    {
        DEBUG("Could not find source route option\n");
        return -1;
    }

    if (dp_trigg->srt_opt->salv == 0)
        dst = dp_trigg->src;
    else
        dst.s_addr = dp_trigg->srt_opt->addrs[1];

    dp = dsr_pkt_alloc(nullptr);

    if (!dp)
    {
        DEBUG("Could not allocate DSR packet\n");
        return -1;
    }

    dp->srt = dsr_rtc_find(myaddr, dst);

    if (!dp->srt)
    {
        DEBUG("No source route to %s\n", print_ip(dst));
        dsr_pkt_free(dp);
        return -1;
    }

    len = DSR_OPT_HDR_LEN + DSR_SRT_OPT_LEN(dp->srt) +
          (DSR_RERR_HDR_LEN + 4) +
          DSR_ACK_HDR_LEN * dp_trigg->ack_opt.size();

    /* Also count in RERR opts in trigger packet */
    for (i = 0; i < (int)dp_trigg->rerr_opt.size(); i++)
    {
        if (dp_trigg->rerr_opt[i]->salv > ConfVal(MAX_SALVAGE_COUNT))
            break;

        len += (dp_trigg->rerr_opt[i]->length + 2);
    }

    DEBUG("opt_len=%d SR: %s\n", len, print_srt(dp->srt));
    n = dp->srt->laddrs / sizeof(struct in_addr);
    dp->src = myaddr;
    dp->dst = dst;
    dp->nxt_hop = dsr_srt_next_hop(dp->srt, n);

    dp->nh.iph = dsr_build_ip(dp, dp->src, dp->dst, IP_HDR_LEN,
                              IP_HDR_LEN + len, IPPROTO_DSR, IPDEFTTL);

    if (!dp->nh.iph)
    {
        DEBUG("Could not create IP header\n");
        goto out_err;
    }

    buf = dsr_pkt_alloc_opts(dp);

    if (!buf)
        goto out_err;

    dsr_opt_hdr_add(buf, len, DSR_NO_NEXT_HDR_TYPE);

    if (dp->dh.opth.empty())
    {
        DEBUG("Could not create DSR options header\n");
        goto out_err;
    }

    len -= DSR_OPT_HDR_LEN;
    dp->dh.opth.begin()->p_len = len;

    dp->srt_opt = dsr_srt_opt_add(buf, len, 0, 0, dp->srt);

    if (!dp->srt_opt)
    {
        DEBUG("Could not create Source Route option header\n");
        goto out_err;
    }

    len -= DSR_SRT_OPT_LEN(dp->srt);
    rerr_opt = dsr_rerr_opt_add(buf, len, NODE_UNREACHABLE, dp->src,
                                dp->dst, unr_addr,
                                dp_trigg->srt_opt->salv);

    if (!rerr_opt)
        goto out_err;

    len -= (rerr_opt->length + 2);

    /* Add old RERR options */
    for (i = 0; i < (int)dp_trigg->rerr_opt.size(); i++)
    {

        if (dp_trigg->rerr_opt[i]->salv > ConfVal(MAX_SALVAGE_COUNT))
            break;
        buf->option.push_back(dp_trigg->rerr_opt[i]->dup());
        len -= (dp_trigg->rerr_opt[i]->length + 2);
    }

    /* TODO: Must preserve order of RERR and ACK options from triggering
     * packet */

    /* Add old ACK options */
    for (i = 0; i < (int)dp_trigg->ack_opt.size(); i++)
    {
        buf->option.push_back(dp_trigg->ack_opt[i]->dup());
        len -= (dp_trigg->ack_opt[i]->length + 2);
    }

    err_src.s_addr = rerr_opt->err_src;
    err_dst.s_addr = rerr_opt->err_dst;
    unr_dst.s_addr = rerr_opt->info;

    /*DEBUG("Send RERR err_src %s err_dst %s unr_dst %s\n",
            print_ip(err_src),
            print_ip(err_dst),
            print_ip(*((struct in_addr *)&rerr_opt->info[0])));*/


    DEBUG("Send RERR err_src %s err_dst %s unr_dst %s\n",
              print_ip(err_src),
              print_ip(err_dst),
              print_ip(unr_dst));

    XMIT(dp);

    return 0;

out_err:

    dsr_pkt_free(dp);

    return -1;

}

int NSCLASS dsr_rerr_opt_recv(struct dsr_pkt *dp, struct dsr_rerr_opt *rerr_opt)
{
    struct in_addr err_src, err_dst, unr_addr;

    if (!rerr_opt)
        return -1;

    dp->rerr_opt.push_back(rerr_opt);

    switch (rerr_opt->err_type)
    {
    case NODE_UNREACHABLE:
        err_src.s_addr = rerr_opt->err_src;
        err_dst.s_addr = rerr_opt->err_dst;

        //memcpy(&unr_addr, &(rerr_opt->info[0]), sizeof(unr_addr));
        unr_addr.s_addr = rerr_opt->info;

        DEBUG("NODE_UNREACHABLE err_src=%s err_dst=%s unr=%s\n",
              print_ip(err_src), print_ip(err_dst), print_ip(unr_addr));

        /* For now we drop all unacked packets... should probably
         * salvage */
        maint_buf_del_all(err_dst);

        /* Remove broken link from cache */

#ifdef OMNETPP
        ph_srt_delete_link_map(err_src,unr_addr);
#else
        lc_link_del(err_src, unr_addr);
#endif

        /* TODO: Check options following the RERR option */
        /*      dsr_rtc_del(my_addr(), err_dst); */
        break;
    case FLOW_STATE_NOT_SUPPORTED:
        DEBUG("FLOW_STATE_NOT_SUPPORTED\n");
        break;
    case OPTION_NOT_SUPPORTED:
        DEBUG("OPTION_NOT_SUPPORTED\n");
        break;
    }

    return 0;
}

} // namespace inetmanet

} // namespace inet
