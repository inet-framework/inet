/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */

#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"
#include "inet/routing/extras/dsr/dsr-uu/debug_dsr.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/contract/NetworkHeaderBase_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {

namespace inetmanet {

struct dsr_opt_hdr * NSCLASS dsr_pkt_alloc_opts(struct dsr_pkt *dp)
{
    if (!dp)
        return nullptr;

    dp->dh.opth.resize(dp->dh.opth.size()+1);
    return &(dp->dh.opth.back());
}

void NSCLASS NSCLASS dsr_pkt_free_opts(struct dsr_pkt *dp)
{
    dp->dh.opth.clear();
}

struct dsr_pkt *  dsr_pkt::dup()
{
    struct dsr_pkt *dp;
    // int dsr_opts_len = 0;

    // dp = (struct dsr_pkt *)MALLOC(sizeof(struct dsr_pkt), GFP_ATOMIC);
    if (DSRUU::lifoDsrPkt!=nullptr)
    {
        dp=DSRUU::lifoDsrPkt;
        DSRUU::lifoDsrPkt = dp->next;
        DSRUU::lifo_token++;
    }
    else
        dp = new dsr_pkt;

    if (!dp)
        return nullptr;
    dp->clear();
    dp->mac.raw = dp->mac_data;
    dp->nh.iph = (struct iphdr *) dp->ip_data;
    memcpy(dp->mac_data,this->mac_data,sizeof(this->mac_data));
    memcpy(dp->ip_data,this->ip_data,sizeof(this->ip_data));

    dp->src.s_addr = this->src.s_addr;
    dp->dst.s_addr = this->dst.s_addr;

    dp->moreFragments = this->moreFragments;
    dp->fragmentOffset = this->moreFragments;

    dp->totalPayloadLength = this->totalPayloadLength;

    dp->payload_len = this->payload_len;
    if (this->payload)
        dp->payload = this->payload->dup();

    if (this->srt)
    {
        dp->srt = new dsr_srt;
        *dp->srt = *this->srt;
    }

    dp->encapsulate_protocol = this->encapsulate_protocol;
    dp->dh.opth = this->dh.opth;
    dp->costVector = this->costVector;
    DSRUU::dsr_opt_parse(dp);
    dp->flags = this->flags;
    dp->inputInterfaceId = this->inputInterfaceId;
    return dp;
}

dsr_pkt * NSCLASS dsr_pkt_alloc(Packet * p)
{
    auto dp = new dsr_pkt;
    if (dp == nullptr)
        return nullptr;

    dp->clear();

    // int dsr_opts_len = 0;

    // dp = (struct dsr_pkt *)MALLOC(sizeof(struct dsr_pkt), GFP_ATOMIC);


    if (p == nullptr)
        return dp;
    auto tagIfaceIndication = p->findTag<InterfaceInd>();
    if (tagIfaceIndication)
        dp->inputInterfaceId = tagIfaceIndication->getInterfaceId();
    else
        dp->inputInterfaceId = -1;

    const auto &header = peekNetworkProtocolHeader(p, Protocol::ipv4);
    auto ipHeader = dynamicPtrCast<Ipv4Header> (constPtrCast<NetworkHeaderBase>(header));
    if (ipHeader == nullptr)
        throw cRuntimeError("DSRUU Error: This packet is not a Ipv4");

    if (ipHeader->getDestAddress().isUnspecified())
        throw cRuntimeError("DSRUU Error: This packet doesn't have a valid destination address");

    auto ipv4Header = removeNetworkProtocolHeader(p, Protocol::ipv4);


    dp->encapsulate_protocol=-1;
    dp->mac.raw = dp->mac_data;
    auto macAddressInd = p->findTag<MacAddressInd>();
    if (macAddressInd) {
        macAddressInd->getDestAddress().getAddressBytes(dp->mac.ethh->h_dest); /* destination eth addr */
        macAddressInd->getSrcAddress().getAddressBytes(dp->mac.ethh->h_source); /* destination eth addr */
    }
    // IPv4Address dest = dgram->getDestAddress();
    // IPv4Address src = dgram->getSrcAddress();



    dp->src.s_addr = ipHeader->getSrcAddress();
    dp->dst.s_addr = ipHeader->getDestAddress();
    if (dp->src.s_addr.isUnspecified())
        dp->src.s_addr = myaddr_.s_addr;

    dp->nh.iph = (struct iphdr *) dp->ip_data;

    dp->nh.iph->ihl= B(ipHeader->getHeaderLength()).get();// Header length
    dp->nh.iph->version= ipHeader->getVersion();// Ip version
    dp->nh.iph->tos= ipHeader->getTypeOfService();// ToS
    dp->nh.iph->tot_len = B(ipHeader->getTotalLengthField()).get();// Total length
    dp->nh.iph->id = ipHeader->getIdentification();// Identification
    dp->nh.iph->frag_off= 0x1FFF & ipHeader->getFragmentOffset();//
    if (ipHeader->getMoreFragments())
    dp->nh.iph->frag_off |= 0x2000;
    if (ipHeader->getDontFragment())
    dp->nh.iph->frag_off |= 0x4000;

    dp->moreFragments = ipHeader->getMoreFragments();
    dp->fragmentOffset = ipHeader->getFragmentOffset();

    dp->nh.iph->ttl= ipHeader->getTimeToLive();// TTL
    dp->nh.iph->protocol= ipHeader->getProtocolId();// Transport protocol
    // dp->nh.iph->check= p->;                          // Check sum
    dp->nh.iph->saddr = ipHeader->getSrcAddress();
    dp->nh.iph->daddr = ipHeader->getDestAddress();

#ifdef NEWFRAGMENT
    dp->totalPayloadLength = dgram->getTotalPayloadLength();
#endif
    //if (dgram->getFragmentOffset()==0 && !dgram->getMoreFragments())

    dp->payload = nullptr;
    //else
    //  dp->payload = nullptr;

    if (dp->nh.iph->protocol == IP_PROT_DSR && p->peekAtFront<DSRPkt>() == nullptr)
    throw cRuntimeError("DSRUU Error: This packet deosn't have Dsr header");

    const auto &dsrHeader = findDsrProtocolHeader(p);

    if (dsrHeader != nullptr)
    {
        const auto & dsrpkt = removeDsrProtocolHeader(p);
        if (p->getDataLength() > B(0)) {
            dp->payload = p;
        }
        else
            delete p;

        dp->dh.opth = dsrpkt->getDsrOptions();
        dsrpkt->clearDsrOptions();
        // dsr_opts_len = dp->dh.opth.begin()->p_len + DSR_OPT_HDR_LEN;

        if (dp->payload)
            dp->encapsulate_protocol = dsrpkt->getEncapProtocol();

        int n = dsr_opt_parse(dp);
        DEBUG("Packet has %d DSR option(s)\n", n);

        dp->costVector = dsrpkt->getCostVector();
        dsrpkt->resetCostVector();
        p=nullptr;
    }
    else
    {
        if (p->getDataLength() > B(0)) {
            dp->payload = p;
            p = nullptr;
            dp->encapsulate_protocol = ipHeader->getProtocolId();
        }
        else
            delete p;
    }

    dp->payload_len = 0;
    if (dp->payload) {
        dp->payload_len = dp->payload->getByteLength();
        if (dp->payload_len && ConfVal(UseNetworkLayerAck))
            dp->flags |= PKT_REQUEST_ACK;
    }

    return dp;
}

void NSCLASS dsr_pkt_free(dsr_pkt *dp)
{

    if (!dp)
        return;
    dsr_pkt_free_opts(dp);

    if (dp->srt)
        delete dp->srt;

    if (dp->payload)
        delete dp->payload;

    if (!dp->costVector.empty())
           dp->costVector.clear();

    if (!dp->dh.opth.empty())
        dp->dh.opth.clear();

    if (dp->payload != nullptr)
        delete dp->payload

    delete dp;
    dp=nullptr;
    return;
}

} // namespace inetmanet

} // namespace inet

