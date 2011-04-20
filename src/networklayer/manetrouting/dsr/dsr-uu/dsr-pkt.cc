/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#define OMNETPP

#ifdef __KERNEL_
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#endif

#ifndef OMNETPP
#ifdef NS2
#include "ns-agent.h"
#endif
#else
#include "dsr-uu-omnetpp.h"

#ifndef MobilityFramework
#include "Ieee802Ctrl_m.h"
#endif

#endif

#include "debug_dsr.h"
#include "dsr-opt.h"
#include "dsr.h"

char *dsr_pkt_alloc_opts(struct dsr_pkt *dp, int len)
{
    if (!dp)
        return NULL;
    dp->dh.raw = (char *)MALLOC(len + DEFAULT_TAILROOM, GFP_ATOMIC);
    if (!dp->dh.raw)
        return NULL;

    dp->dh.tail = dp->dh.raw + len;
    dp->dh.end = dp->dh.tail + DEFAULT_TAILROOM;

    return dp->dh.raw;
}

char *dsr_pkt_alloc_opts_expand(struct dsr_pkt *dp, int len)
{
    char *tmp;
    int old_len;

    if (!dp || !dp->dh.raw)
        return NULL;

    if (dsr_pkt_tailroom(dp) > len)
    {
        tmp = dp->dh.tail;
        dp->dh.tail += len;
        return tmp;
    }

    tmp = dp->dh.raw;
    old_len = dsr_pkt_opts_len(dp);

    if (!dsr_pkt_alloc_opts(dp, old_len + len))
        return NULL;

    memcpy(dp->dh.raw, tmp, old_len);

    FREE(tmp);

    return (dp->dh.raw + old_len);
}

int dsr_pkt_free_opts(struct dsr_pkt *dp)
{
    int len;

    if (!dp->dh.raw)
        return -1;

    len = dsr_pkt_opts_len(dp);

    FREE(dp->dh.raw);

    dp->dh.raw = dp->dh.end = dp->dh.tail = NULL;
    dp->srt_opt = NULL;
    dp->rreq_opt = NULL;
    memset(dp->rrep_opt, 0, sizeof(struct dsr_rrep_opt *) * MAX_RREP_OPTS);
    memset(dp->rerr_opt, 0, sizeof(struct dsr_rerr_opt *) * MAX_RERR_OPTS);
    memset(dp->ack_opt, 0, sizeof(struct dsr_ack_opt *) * MAX_ACK_OPTS);
    dp->num_rrep_opts = dp->num_rerr_opts = dp->num_ack_opts = 0;

    return len;
}

#ifndef OMNETPP
#ifdef NS2
struct dsr_pkt *dsr_pkt_alloc(Packet * p)
{
    struct dsr_pkt *dp;
    struct hdr_cmn *cmh;
    int dsr_opts_len = 0;

    dp = (struct dsr_pkt *)MALLOC(sizeof(struct dsr_pkt), GFP_ATOMIC);

    if (!dp)
        return NULL;

    memset(dp, 0, sizeof(struct dsr_pkt));

    if (p)
    {
        cmh = hdr_cmn::access(p);

        dp->p = p;
        dp->mac.raw = p->access(hdr_mac::offset_);
        dp->nh.iph = HDR_IP(p);

        dp->src.s_addr =
            Address::getInstance().get_nodeaddr(dp->nh.iph->saddr());
        dp->dst.s_addr =
            Address::getInstance().get_nodeaddr(dp->nh.iph->daddr());

        if (cmh->ptype() == PT_DSR)
        {
            struct dsr_opt_hdr *opth;

            opth = hdr_dsr::access(p);

            dsr_opts_len = ntohs(opth->p_len) + DSR_OPT_HDR_LEN;

            if (!dsr_pkt_alloc_opts(dp, dsr_opts_len))
            {
                FREE(dp);
                return NULL;
            }

            memcpy(dp->dh.raw, (char *)opth, dsr_opts_len);

            dsr_opt_parse(dp);

            if ((DATA_PACKET(dp->dh.opth->nh) ||
                    dp->dh.opth->nh == PT_PING) &&
                    ConfVal(UseNetworkLayerAck))
                dp->flags |= PKT_REQUEST_ACK;
        }
        else if ((DATA_PACKET(cmh->ptype()) ||
                  cmh->ptype() == PT_PING) &&
                 ConfVal(UseNetworkLayerAck))
            dp->flags |= PKT_REQUEST_ACK;

        /* A trick to calculate payload length... */
        dp->payload_len = cmh->size() - dsr_opts_len - IP_HDR_LEN;
    }
    return dp;
}

#else

struct dsr_pkt *dsr_pkt_alloc(struct sk_buff *skb)
{
    struct dsr_pkt *dp;
    int dsr_opts_len = 0;

    dp = (struct dsr_pkt *)MALLOC(sizeof(struct dsr_pkt), GFP_ATOMIC);

    if (!dp)
        return NULL;

    memset(dp, 0, sizeof(struct dsr_pkt));

    if (skb)
    {
        /*  skb_unlink(skb); */

        dp->skb = skb;

        dp->mac.raw = skb->mac.raw;
        dp->nh.iph = skb->nh.iph;

        dp->src.s_addr = skb->nh.iph->saddr;
        dp->dst.s_addr = skb->nh.iph->daddr;

        if (dp->nh.iph->protocol == IPPROTO_DSR)
        {
            struct dsr_opt_hdr *opth;
            int n;

            opth = (struct dsr_opt_hdr *)(dp->nh.raw + (dp->nh.iph->ihl << 2));
            dsr_opts_len = ntohs(opth->p_len) + DSR_OPT_HDR_LEN;

            if (!dsr_pkt_alloc_opts(dp, dsr_opts_len))
            {
                FREE(dp);
                return NULL;
            }

            memcpy(dp->dh.raw, (char *)opth, dsr_opts_len);

            n = dsr_opt_parse(dp);

            DEBUG("Packet has %d DSR option(s)\n", n);
        }

        dp->payload = dp->nh.raw +
                      (dp->nh.iph->ihl << 2) + dsr_opts_len;

        dp->payload_len = ntohs(dp->nh.iph->tot_len) -
                          (dp->nh.iph->ihl << 2) - dsr_opts_len;

        if (dp->payload_len)
            dp->flags |= PKT_REQUEST_ACK;
    }
    return dp;
}

#endif

void dsr_pkt_free(struct dsr_pkt *dp)
{

    if (!dp)
        return;
#ifndef NS2
    if (dp->skb)
        dev_kfree_skb_any(dp->skb);
#endif
    dsr_pkt_free_opts(dp);

    if (dp->srt)
        FREE(dp->srt);

    FREE(dp);

    return;
}

#else
#ifdef MobilityFramework

dsr_pkt * dsr_pkt_alloc(cPacket  * p)
{
    struct dsr_pkt *dp;
    int dsr_opts_len = 0;


    // dp = (struct dsr_pkt *)MALLOC(sizeof(struct dsr_pkt), GFP_ATOMIC);
    if (DSRUU::lifoDsrPkt!=NULL)
    {
        dp=DSRUU::lifoDsrPkt;
        DSRUU::lifoDsrPkt = dp->next;
        DSRUU::lifo_token++;
    }
    else
        dp = new dsr_pkt;

    if (!dp)
        return NULL;
    memset(dp, 0, sizeof(dsr_pkt));
    if (p)
    {
        NetwPkt *dgram = dynamic_cast <NetwPkt *> (p);

        dp->encapsulate_protocol=0;
        dp->mac.raw = dp->mac_data;

        dp->src.s_addr = dgram->getSrcAddr();
        dp->dst.s_addr =dgram->getDestAddr();
        dp->nh.iph = (struct iphdr *) dp->ip_data;
        dp->nh.iph->tot_len= dgram->getByteLength(); // Total length
        dp->nh.iph->ttl= dgram->getTtl(); // TTL
        dp->nh.iph->protocol= dgram->getTransportProtocol(); // Transport protocol
        dp->nh.iph->saddr= dgram->getSrcAddr();
        dp->nh.iph->daddr= dgram->getDestAddr();
        dp->payload = p->decapsulate();
        dp->encapsulate_protocol = 0;

        if (dp->nh.iph->protocol == IP_PROT_DSR)
        {
            struct dsr_opt_hdr *opth;
            int n;
            if (dynamic_cast<DSRPkt*> (p))
            {
                DSRPkt * dsrpkt = dynamic_cast<DSRPkt*> (p);
                if (dp->payload)
                    dp->encapsulate_protocol=dsrpkt->getEncapProtocol();
                opth =  dsrpkt->getOptions();
                dsr_opts_len = opth->p_len + DSR_OPT_HDR_LEN;
                if (!dsr_pkt_alloc_opts(dp, dsr_opts_len))
                {
                    FREE(dp);
                    return NULL;
                }
                memcpy(dp->dh.raw, (char *)opth, dsr_opts_len);
                n = dsr_opt_parse(dp);
                DEBUG("Packet has %d DSR option(s)\n", n);
                dp->ip_pkt = dsrpkt;
                dp->costVector = dsrpkt->getCostVector();
                dp->costVectorSize = dsrpkt->getCostVectorSize();
                dsrpkt->resetCostVector();
                p=NULL;
            }
        }
        else
        {
            if (dp->payload)
                dp->encapsulate_protocol = dgram->getTransportProtocol();
        }

        if (dp->payload)
            dp->payload_len = dp->payload->getByteLength();
        if (dp->payload_len && ConfVal(UseNetworkLayerAck))
            dp->flags |= PKT_REQUEST_ACK;

    }
    if (p)
        delete p;
    return dp;
}

#else
dsr_pkt * dsr_pkt_alloc(cPacket  * p)
{
    struct dsr_pkt *dp;
    int dsr_opts_len = 0;


    // dp = (struct dsr_pkt *)MALLOC(sizeof(struct dsr_pkt), GFP_ATOMIC);
    if (DSRUU::lifoDsrPkt!=NULL)
    {
        dp=DSRUU::lifoDsrPkt;
        DSRUU::lifoDsrPkt = dp->next;
        DSRUU::lifo_token++;
    }
    else
        dp = new dsr_pkt;

    if (!dp)
        return NULL;
    memset(dp, 0, sizeof(dsr_pkt));
    if (p)
    {
        IPDatagram *dgram = dynamic_cast <IPDatagram *> (p);
        dp->encapsulate_protocol=0;
        dp->mac.raw = dp->mac_data;
        cPolymorphic * ctrl = dgram->removeControlInfo();
        if (ctrl!=NULL)
        {
            Ieee802Ctrl * ctrlmac = check_and_cast<Ieee802Ctrl *> (ctrl);
            memcpy (dp->mac.ethh->h_dest,ctrlmac->getDest().getAddressBytes(),ETH_ALEN);    /* destination eth addr */
            memcpy (dp->mac.ethh->h_source,ctrlmac->getSrc().getAddressBytes(),ETH_ALEN);   /* destination eth addr */
            delete ctrl;
        }

        // IPAddress dest = dgram->getDestAddress();
        // IPAddress src = dgram->getSrcAddress();

        dp->src.s_addr = dgram->getSrcAddress().getInt();
        dp->dst.s_addr =dgram->getDestAddress().getInt();
        dp->nh.iph = (struct iphdr *) dp->ip_data;
        dp->nh.iph->ihl= dgram->getHeaderLength(); // Header length
        dp->nh.iph->version= dgram->getVersion(); // Ip version
        dp->nh.iph->tos= dgram->getDiffServCodePoint(); // ToS
        dp->nh.iph->tot_len= dgram->getByteLength(); // Total length
        dp->nh.iph->id= dgram->getIdentification(); // Identification
        dp->nh.iph->frag_off= 0x1FFF & dgram->getFragmentOffset(); //
        if (dgram->getMoreFragments())
            dp->nh.iph->frag_off |= 0x2000;
        if (dgram->getDontFragment())
            dp->nh.iph->frag_off |= 0x4000;

        dp->moreFragments=dgram->getMoreFragments();
        dp->fragmentOffset=dgram->getFragmentOffset();

        dp->nh.iph->ttl= dgram->getTimeToLive(); // TTL
        dp->nh.iph->protocol= dgram->getTransportProtocol(); // Transport protocol
        // dp->nh.iph->check= p->;                          // Check sum
        dp->nh.iph->saddr= dgram->getSrcAddress().getInt();
        dp->nh.iph->daddr= dgram->getDestAddress().getInt();
        //if (dgram->getFragmentOffset()==0 && !dgram->getMoreFragments())
        dp->payload = p->decapsulate();
        //else
        //  dp->payload = NULL;
        dp->encapsulate_protocol = 0;

        if (dp->nh.iph->protocol == IP_PROT_DSR)
        {
            struct dsr_opt_hdr *opth;
            int n;
            if (dynamic_cast<DSRPkt*> (p))
            {
                DSRPkt * dsrpkt = dynamic_cast<DSRPkt*> (p);

                opth =  dsrpkt->getOptions();
                dsr_opts_len = opth->p_len + DSR_OPT_HDR_LEN;
                if (!dsr_pkt_alloc_opts(dp, dsr_opts_len))
                {
                    FREE(dp);
                    return NULL;
                }
                if (dp->payload)
                    dp->encapsulate_protocol=dsrpkt->getEncapProtocol();

                memcpy(dp->dh.raw, (char *)opth, dsr_opts_len);
                n = dsr_opt_parse(dp);
                DEBUG("Packet has %d DSR option(s)\n", n);
                dp->ip_pkt = dsrpkt;
                dp->costVector = dsrpkt->getCostVector();
                dp->costVectorSize = dsrpkt->getCostVectorSize();
                dsrpkt->resetCostVector();
                p=NULL;
            }
        }
        else
        {
            if (dp->payload)
                dp->encapsulate_protocol = dgram->getTransportProtocol();
        }

        if (dp->payload)
            dp->payload_len = dp->payload->getByteLength();
        if (dp->payload_len && ConfVal(UseNetworkLayerAck))
            dp->flags |= PKT_REQUEST_ACK;

    }
    if (p)
    {
        delete p;
        p = NULL;
    }
    return dp;
}
#endif

void dsr_pkt_free(dsr_pkt *dp)
{

    if (!dp)
        return;
    dsr_pkt_free_opts(dp);

    if (dp->srt)
        FREE(dp->srt);

    if (dp->payload)
        delete dp->payload;

    if (dp->ip_pkt)
        delete dp->ip_pkt;

    if (dp->costVectorSize>0)
        delete [] dp->costVector;

    if (DSRUU::lifo_token>0)
    {
        DSRUU::lifo_token--;
        dp->next=DSRUU::lifoDsrPkt;
        DSRUU::lifoDsrPkt=dp;
    }
    else
        delete dp;


    dp=NULL;
    return;

}

#endif
