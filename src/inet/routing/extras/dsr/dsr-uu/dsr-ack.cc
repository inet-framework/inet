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

struct dsr_ack_opt *dsr_ack_opt_add(struct dsr_opt_hdr *buf, int len, struct in_addr src,
                                    struct in_addr dst, unsigned short id)
{


    if (len < (int)DSR_ACK_HDR_LEN)
        return nullptr;

    struct dsr_ack_opt *ack = new dsr_ack_opt();
    ack->type = DSR_OPT_ACK;
    ack->length = DSR_ACK_OPT_LEN;
    ack->id = htons(id);
    ack->dst = dst.s_addr;
    ack->src = src.s_addr;
    buf->option.push_back(ack);

    return ack;
}

int NSCLASS dsr_ack_send(struct in_addr dst, unsigned short id)
{
    struct dsr_pkt *dp;
    struct dsr_ack_opt *ack_opt;
    int len;
    struct dsr_opt_hdr *buf;

    /* srt = dsr_rtc_find(my_addr(), dst); */

    /*  if (!srt) { */
    /*      DEBUG("No source route to %s\n", print_ip(dst.s_addr)); */
    /*      return -1; */
    /*  } */

    len = DSR_OPT_HDR_LEN + /* DSR_SRT_OPT_LEN(srt) +  */ DSR_ACK_HDR_LEN;

    dp = dsr_pkt_alloc(nullptr);

    dp->dst = dst;
    /* dp->srt = srt; */
    dp->nxt_hop = dst;  //dsr_srt_next_hop(dp->srt, 0);
    dp->src = my_addr();

    buf = dsr_pkt_alloc_opts(dp);

    if (!buf)
        goto out_err;

    dp->nh.iph = dsr_build_ip(dp, dp->src, dp->dst, IP_HDR_LEN,
                              IP_HDR_LEN + len, IPPROTO_DSR, IPDEFTTL);

    if (!dp->nh.iph)
    {
        DEBUG("Could not create IP header\n");
        goto out_err;
    }

    dsr_opt_hdr_add(buf, len, DSR_NO_NEXT_HDR_TYPE);

    if (dp->dh.opth.empty())
    {
        DEBUG("Could not create DSR opt header\n");
        goto out_err;
    }

    len -= DSR_OPT_HDR_LEN;

    /* dp->srt_opt = dsr_srt_opt_add(buf, len, dp->srt); */

    /*  if (!dp->srt_opt) { */
    /*      DEBUG("Could not create Source Route option header\n"); */
    /*      goto out_err; */
    /*  } */

    /*  buf += DSR_SRT_OPT_LEN(dp->srt); */
    /*  len -= DSR_SRT_OPT_LEN(dp->srt); */

    ack_opt = dsr_ack_opt_add(buf, len, dp->src, dp->dst, id);

    if (!ack_opt)
    {
        DEBUG("Could not create DSR ACK opt header\n");
        goto out_err;
    }

    DEBUG("Sending ACK to %s id=%u\n", print_ip(dst), id);

    dp->flags |= PKT_XMIT_JITTER;

    XMIT(dp);

    return 1;

out_err:
    dsr_pkt_free(dp);
    return -1;
}

static struct dsr_ack_req_opt *dsr_ack_req_opt_create(struct dsr_opt_hdr *buf, int len,
        unsigned short id)
{

    if (len < (int)DSR_ACK_REQ_HDR_LEN)
        return nullptr;

    struct dsr_ack_req_opt *ack_req = new  dsr_ack_req_opt();
    /* Build option */
    ack_req->type = DSR_OPT_ACK_REQ;
    ack_req->length = DSR_ACK_REQ_OPT_LEN;
    ack_req->id = htons(id);
    buf->option.push_back(ack_req);

    return ack_req;
}



int NSCLASS dsr_ack_req_send(struct dsr_pkt *dp)
{
    if (!dp->payload)
        return -1;

    /* If TTL = 0, drop packet */
    if (dp->nh.iph->ttl <= 0) {
        DEBUG("Dropping packet with TTL = 0.");
        return -1;
    }

    /* srt = dsr_rtc_find(my_addr(), dst); */

    /*  if (!srt) { */
    /*      DEBUG("No source route to %s\n", print_ip(dst.s_addr)); */
    /*      return -1; */
    /*  } */
    dp->numRetries++;


    auto dgram = newDsrPacket(dp, interfaceId, dp->payload);


    if (dp->nh.iph->protocol != IP_PROT_DSR)
        throw cRuntimeError("error in Packet, is not DSR ");


    DEBUG("xmitting pkt src=%s dst=%s nxt_hop=%s\n",
          dp->src.s_addr.str().c_str(), dp->dst.s_addr.str().c_str(), dp->nxt_hop.s_addr.str().c_str());

    /* Set packet fields depending on packet type */
    double jitter = 0;
    if (dp->flags & PKT_XMIT_JITTER)
    {
        /* Broadcast packet */
        if (ConfVal(BroadcastJitter)!=0)
        {
            jitter = uniform(0, ((double) ConfVal(BroadcastJitter))/1000);
            DEBUG("xmit jitter=%f s\n", jitter);
        }
    }

    /*
    if (!ConfVal(UseNetworkLayerAck)) {
        cmh->xmit_failure_ = xmit_failure;
        cmh->xmit_failure_data_ = (void *) this;
    }
    */

//    if (jitter)
//        sendDelayed(dgram, jitter, "socketOut");
//    else if (dp->dst.s_addr.toIpv4().getInt() != DSR_BROADCAST)
//        sendDelayed(dgram, par("unicastDelay"), "socketOut");
//    else
//        sendDelayed(dgram, par ("broadcastDelay"), "socketOut");
    if (jitter)
        injectDirectToIp(dgram, jitter, nullptr);
    else if (dp->dst.s_addr.toIpv4().getInt() != DSR_BROADCAST)
        injectDirectToIp(dgram, par("unicastDelay"), nullptr);
    else
        injectDirectToIp(dgram, par ("broadcastDelay"), nullptr);
    return 1;
}


struct dsr_ack_req_opt *NSCLASS
dsr_ack_req_opt_add(struct dsr_pkt *dp, unsigned short id)
{
    struct dsr_opt *dopt = nullptr;
    int prot = 0, tot_len = 0, ttl = IPDEFTTL;
    struct dsr_opt_hdr *buf = nullptr;

    if (!dp)
        return nullptr;

    /* If we are forwarding a packet and there is already an ACK REQ option,
     * we just overwrite the old one. */
    for (unsigned int i = 0; i < dp->dh.opth.size(); i++)
    {
        for (unsigned int j = 0; j < dp->dh.opth[i].option.size(); j++)
        {
            dopt = dp->dh.opth[i].option[j];
            if (dopt->type==DSR_OPT_ACK_REQ)
            {
                buf = &dp->dh.opth[i];
                auto ackOpt = dynamic_cast<dsr_ack_req_opt *>(dopt);
                if (ackOpt == nullptr)
                    throw cRuntimeError("The option is not of the ACK type");
                // change the id and return, this is rewrite the option
                ackOpt->id = id;
                return  ackOpt;
            }
        }
    }
#ifdef NS2
    if (dp->p)
    {
        hdr_cmn *cmh = HDR_CMN(dp->p);
        prot = cmh->ptype();
    }
    else
        prot = DSR_NO_NEXT_HDR_TYPE;

    ttl = dp->nh.iph->ttl();
#else
    if (dp->nh.raw)
    {
        tot_len = ntohs(dp->nh.iph->tot_len);
        prot = dp->nh.iph->protocol;
        ttl = dp->nh.iph->ttl;
    }
#endif
    if (dp->dh.opth.empty())
    {

        buf = dsr_pkt_alloc_opts(dp);
        DEBUG("Allocating options for ACK REQ\n");
        if (dp->dh.opth.empty())
            return nullptr;

        dsr_build_ip(dp, dp->src, dp->dst, IP_HDR_LEN,
                     tot_len + DSR_OPT_HDR_LEN + DSR_ACK_REQ_HDR_LEN,
                     IPPROTO_DSR, ttl);

        dsr_opt_hdr_add(buf, DSR_OPT_HDR_LEN + DSR_ACK_REQ_HDR_LEN,prot);

        if (dp->dh.opth.empty())
            return nullptr;
    }
    else
    {

        buf = &dp->dh.opth.back();

        DEBUG("Expanding options for ACK REQ p_len=%d\n",
              ntohs(dp->dh.opth.begin()->p_len));
        if (!buf)
            return nullptr;

        dsr_build_ip(dp, dp->src, dp->dst, IP_HDR_LEN,
                     tot_len + DSR_ACK_REQ_HDR_LEN, IPPROTO_DSR, ttl);

        dsr_opt_hdr_add(buf, DSR_OPT_HDR_LEN +
                            ntohs(dp->dh.opth.begin()->p_len) +
                            DSR_ACK_REQ_HDR_LEN, dp->dh.opth.begin()->nh);
    }
    DEBUG("Added ACK REQ option id=%u\n", id, ntohs(dp->dh.opth.begin()->p_len));
    return dsr_ack_req_opt_create(buf, DSR_ACK_REQ_HDR_LEN, id);
}

int NSCLASS dsr_ack_req_send(struct in_addr neigh_addr, unsigned short id)
{
    struct dsr_pkt *dp;
    struct dsr_ack_req_opt *ack_req;
    int len = DSR_OPT_HDR_LEN + DSR_ACK_REQ_HDR_LEN;
    struct dsr_opt_hdr *buf;

    dp = dsr_pkt_alloc(nullptr);

    dp->dst = neigh_addr;
    dp->nxt_hop = neigh_addr;
    dp->src = my_addr();

    buf = dsr_pkt_alloc_opts(dp);

    if (dp->dh.opth.empty())
    {
        DEBUG("Could not create DSR opt header\n");
        goto out_err;
    }

    if (!buf)
        goto out_err;

    dp->nh.iph = dsr_build_ip(dp, dp->src, dp->dst, IP_HDR_LEN,
                              IP_HDR_LEN + len, IPPROTO_DSR, 1);

    if (!dp->nh.iph)
    {
        DEBUG("Could not create IP header\n");
        goto out_err;
    }

    dsr_opt_hdr_add(buf, len, DSR_NO_NEXT_HDR_TYPE);

    len -= DSR_OPT_HDR_LEN;
    dp->dh.opth.begin()->p_len = len;

    ack_req = dsr_ack_req_opt_create(buf, len, id);

    if (!ack_req)
    {
        DEBUG("Could not create ACK REQ opt\n");
        goto out_err;
    }

    DEBUG("Sending ACK REQ for %s id=%u\n", print_ip(neigh_addr), id);

    XMIT(dp);

    return 1;

out_err:
    dsr_pkt_free(dp);
    return -1;
}

int NSCLASS dsr_ack_req_opt_recv(struct dsr_pkt *dp, struct dsr_ack_req_opt *ack_req_opt)
{
    unsigned short id;

    if (!ack_req_opt || !dp || dp->flags & PKT_PROMISC_RECV)
        return DSR_PKT_ERROR;

    dp->ack_req_opt = ack_req_opt;

    id = ntohs(ack_req_opt->id);

    if (!dp->srt_opt)
        dp->prv_hop = dp->src;

    DEBUG("src=%s prv=%s id=%u\n",
          print_ip(dp->src), print_ip(dp->prv_hop), id);

    dsr_ack_send(dp->prv_hop, id);

    return DSR_PKT_NONE;
}

int NSCLASS dsr_ack_opt_recv(struct dsr_ack_opt *ack)
{
    unsigned short id;
    struct in_addr dst, src, myaddr;
    int n;

    if (!ack)
        return DSR_PKT_ERROR;

    myaddr = my_addr();

    dst.s_addr = ack->dst;
    src.s_addr = ack->src;
    id = ntohs(ack->id);

    DEBUG("ACK dst=%s src=%s id=%u\n", print_ip(dst), print_ip(src), id);

    if (dst.s_addr != myaddr.s_addr)
        return DSR_PKT_ERROR;

    /* Purge packets buffered for this next hop */
    n = maint_buf_del_all_id(src, id);

    DEBUG("Removed %d packets from maint buf\n", n);

    return DSR_PKT_NONE;
}

} // namespace inetmanet

} // namespace inet

