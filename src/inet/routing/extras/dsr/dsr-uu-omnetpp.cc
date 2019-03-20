/*****************************************************************************
 *
 * Copyright (C) 2002 Uppsala University.
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
 * Authors: Bj�n Wiberg <bjorn.wiberg@home.se>
 *          Erik Nordstr� <erik.nordstrom@it.uu.se>
 * Authors: Alfonso Ariza Quintana.<aarizaq@uma.ea>
 *
 *****************************************************************************/

//#include <iostream>
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"

#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/routing/extras/dsr/DsrProtocolTag_m.h"
#include "inet/common/packet/dissector/PacketDissector.h"

namespace inet {

using namespace ieee80211;

namespace inetmanet {

unsigned int DSRUU::confvals[CONFVAL_MAX];
//simtime_t DSRUU::current_time;
struct dsr_pkt * DSRUU::lifoDsrPkt;
int DSRUU::lifo_token;

Define_Module(DSRUU);


std::ostream& operator<<(std::ostream& os, const DsrDataBase::PathsToDestination& e)
{

    for (unsigned int i = 0; i<e.size(); i++)
    {
        os << "path : " << i << " " << "expire :" << e[i].expires << " ";
        os << " route :  ";
        for (unsigned int  j = 0;  j < e[i].route.size() ;j++)
        {
            os << e[i].route[j] << " - ";
        }
        os << endl;
    }
    return os;
};

std::ostream& operator<<(std::ostream& os, const DSRUU::PacketStoreage& e)
{
    os << "expire :" << e.time << " ";

    os << "destination :" << Ipv4Address(e.packet->dst.s_addr.toIpv4());
    return os;
};



struct iphdr *DSRUU::dsr_build_ip(struct dsr_pkt *dp, struct in_addr src,
                                  struct in_addr dst, int ip_len, int tot_len,
                                  int protocol, int ttl)
{
    struct iphdr *iph;

    dp->nh.iph = iph = (struct iphdr *)dp->ip_data;

    if (dp->payload)
    {
        iph->ttl = (ttl ? ttl : IPDEFTTL);
        iph->saddr = src.s_addr;
        iph->daddr = dst.s_addr;
    }
    else
    {
        iph->version = 4; //IPVERSION;
        iph->ihl = ip_len;
        iph->tos = 0;
        iph->id = 0;
        iph->frag_off = 0;
        iph->ttl = (ttl ? ttl : IPDEFTTL);
        iph->saddr = src.s_addr;
        iph->daddr = dst.s_addr;
    }

    iph->tot_len = htons(tot_len);
    iph->protocol = protocol;
    return iph;
}



void DSRUU::omnet_xmit(struct dsr_pkt *dp)
{

    double jitter = 0;

    if (dp->flags & PKT_REQUEST_ACK)
        maint_buf_add(dp);

    if (dp->nh.iph->ttl<= 0) {
        DEBUG("Dropping packet with TTL = 0.");
        drop(dp->payload, ICMP_TIME_EXCEEDED);
        dp->payload = nullptr;
        dsr_pkt_free(dp);
        return;
    }

    auto p = newDsrPacket(dp, interfaceId);

    if (!p)
    {
        DEBUG("Could not create packet\n");
        if (dp->payload)
            drop(dp->payload, ICMP_DESTINATION_UNREACHABLE);
        dp->payload = nullptr;
        dsr_pkt_free(dp);
        return;
    }

    DEBUG("xmitting pkt src=%d dst=%d nxt_hop=%d\n",
          (uint32_t)dp->src.s_addr.toIpv4().getInt(), (uint32_t)dp->dst.s_addr.toIpv4().getInt(), (uint32_t)dp->nxt_hop.s_addr.toIpv4().getInt());

    /* Set packet fields depending on packet type */
    if (dp->flags & PKT_XMIT_JITTER)
    {
        /* Broadcast packet */
        jitter=0;
        if (ConfVal(BroadcastJitter))
        {
           jitter = uniform(0, ((double) ConfVal(BroadcastJitter))/1000);
           DEBUG("xmit jitter=%f s\n", jitter);
        }
    }

    if (dp->dst.s_addr.toIpv4().getInt() != DSR_BROADCAST)
    {
        /* Get hardware destination address */
        p->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(dp->nxt_hop.s_addr);
        //IPv4Address nextIp((uint32_t)dp->nxt_hop.s_addr);
        //p->setNextAddress(nextIp);
    }
    /*
    if (!ConfVal(UseNetworkLayerAck)) {
        cmh->xmit_failure_ = xmit_failure;
        cmh->xmit_failure_data_ = (void *) this;
    }
    */
    //L3Address prev = myaddr_.s_addr;
    //p->setPrevAddress(prev);
#if 0
    if (jitter)
         sendDelayed(p, jitter, "socketOut");
    else if (dp->dst.s_addr.toIpv4().getInt() != DSR_BROADCAST)
        sendDelayed(p, par("unicastDelay"), "socketOut");
    else
        sendDelayed(p, par("broadcastDelay"), "socketOut");
#else
    if (jitter)
        injectDirectToIp(p, jitter, nullptr);
    else if (dp->dst.s_addr.toIpv4().getInt() != DSR_BROADCAST)
        injectDirectToIp(p, par("unicastDelay"), nullptr);
    else
        injectDirectToIp(p, par("broadcastDelay"), nullptr);
#endif
    dp->payload = nullptr;
    dsr_pkt_free(dp);
}

void DSRUU::omnet_deliver(struct dsr_pkt *dp)
{
    int dsr_opts_len = 0;
    if (!dp->dh.opth.empty())
    {
        dsr_opts_len = dp->dh.opth.begin()->p_len + DSR_OPT_HDR_LEN;
        dsr_opt_remove(dp);
    }

    auto outInterface = interfaceId;
    if (isLocalAddress(dp->dst.s_addr))
        outInterface = -1;

    auto dgram = newDsrPacket(dp, interfaceId, false);

    dp->payload = nullptr;
    dsr_pkt_free(dp);
    injectDirectToIp(dgram, 0, nullptr);
}


void  DSRUUTimer::resched(double delay)
{
    if (msgtimer.isScheduled())
        a_->cSimpleModule::cancelEvent(&msgtimer);
    a_->scheduleAt(simTime()+delay, &msgtimer);
}

void DSRUUTimer::cancel()
{
    if (msgtimer.isScheduled())
        a_->cancelEvent(&msgtimer);
}

void DSRUU::initialize(int stage)
{
    ManetRoutingBase::initialize(stage);

    //current_time =simTime();
    if (stage == INITSTAGE_LOCAL)
    {

        for (int i = 0; i < CONFVAL_MAX; i++)
        {
            /* Override the default values in the ns-default.tcl file */
            confvals[i] = confvals_def[i].val;
//          sprintf(name, "%s_", confvals_def[i].name);
//          bind(name,  &confvals[i]);
        }
        nodeActive = true;

        confvals[FlushLinkCache] = 0;
        confvals[PromiscOperation] = 0;
        confvals[UseNetworkLayerAck] = 0;
        confvals[UseNetworkLayerAck] = 0;
#ifdef DEBUG
        if (par("PrintDebug"))
            confvals[PrintDebug] = 1;
#endif
        if (par("FlushLinkCache"))
            confvals[FlushLinkCache] = 1;
        if (par("PromiscOperation"))
            confvals[PromiscOperation] = 1;
        if (par("UseNetworkLayerAck"))
            confvals[UseNetworkLayerAck] = 1;
        int aux_var;
        aux_var = par("BroadcastJitter");
        if (aux_var!=-1)
            confvals[BroadcastJitter] = aux_var;
        aux_var = par("RouteCacheTimeout");
        if (aux_var!=-1)
            confvals[RouteCacheTimeout] = par("RouteCacheTimeout");
        aux_var = par("SendBufferTimeout");
        if (aux_var!=-1)
            confvals[SendBufferTimeout] = par("SendBufferTimeout");
        aux_var = par("SendBufferSize");
        if (aux_var!=-1)
            confvals[SendBufferSize] = par("SendBufferSize");
        aux_var = par("RequestTableSize");
        if (aux_var!=-1)
            confvals[RequestTableSize] = par("RequestTableSize");
        aux_var = par("RequestTableIds");
        if (aux_var!=-1)
            confvals[RequestTableIds] = par("RequestTableIds");
        aux_var = par("MaxRequestRexmt");
        if (aux_var!=-1)
            confvals[MaxRequestRexmt] = par("MaxRequestRexmt");
        aux_var = par("MaxRequestPeriod");
        if (aux_var!=-1)
            confvals[MaxRequestPeriod] = par("MaxRequestPeriod");
        aux_var = par("RequestPeriod");
        if (aux_var!=-1)
            confvals[RequestPeriod] = par("RequestPeriod");
        aux_var = par("NonpropRequestTimeout");
        if (aux_var!=-1)
            confvals[NonpropRequestTimeout] = par("NonpropRequestTimeout");
        aux_var = par("RexmtBufferSize");
        if (aux_var!=-1)
            confvals[RexmtBufferSize] = par("RexmtBufferSize");
        aux_var = par("MaintHoldoffTime");
        if (aux_var!=-1)
            confvals[MaintHoldoffTime] = par("MaintHoldoffTime");
        aux_var = par("MaxMaintRexmt");
        if (aux_var!=-1)
            confvals[MaxMaintRexmt] = par("MaxMaintRexmt");

        if (par("TryPassiveAcks"))
            confvals[TryPassiveAcks] = 1;
        else
            confvals[TryPassiveAcks] = 0;

        aux_var = par("PassiveAckTimeout");
        if (aux_var!=-1)
            confvals[PassiveAckTimeout] = par("PassiveAckTimeout");
        aux_var = par("GratReplyHoldOff");
        if (aux_var!=-1)
            confvals[GratReplyHoldOff] = par("GratReplyHoldOff");
        aux_var = par("MAX_SALVAGE_COUNT");

        if (aux_var!=-1)
            confvals[MAX_SALVAGE_COUNT] = par("MAX_SALVAGE_COUNT");

        lifo_token = par("LifoSize");
        if (par("PathCache"))
            confvals[PathCache] = 1;
        else
            confvals[PathCache] = 0;

        if (par("RetryPacket"))
            confvals[RetryPacket] = 1;
        else
            confvals[RetryPacket] = 0;

        aux_var = par("RREQMaxVisit");

        if (aux_var!=-1)
            confvals[RREQMaxVisit] = par("RREQMaxVisit");
        if (par("RREPDestinationOnly"))
            confvals[RREPDestinationOnly] = 1;
        else
            confvals[RREPDestinationOnly] = 0;

        if (confvals[RREQMaxVisit]>1)
            confvals[RREQMulVisit] = 1;
        else
            confvals[RREQMulVisit] = 0;
#ifdef DEBUG
        /* Default values specific to simulation */
        set_confval(PrintDebug, 1);
#endif
        grat_rrep_tbl_timer.setOwer(this);
        send_buf_timer.setOwer(this);
        neigh_tbl_timer.setOwer(this);
        lc_timer.setOwer(this);
        ack_timer.setOwer(this);
        etx_timer.setOwer(this);
        is_init = true;
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        registerRoutingModule();

        registerService(Protocol::dsr, nullptr, gate("socketIn"));
        registerProtocol(Protocol::dsr, gate("socketOut"), nullptr);

        registerService(Protocol::manet, nullptr, gate("socketIn"));
        registerProtocol(Protocol::manet, gate("socketOut"), nullptr);

        registerHook();

        // ASSERT(stage >= STAGE:IP_LAYER_READY_FOR_HOOK_REGISTRATION);

        int  num_80211 = 0;
        InterfaceEntry *   i_face;

        for (int i = 0; i < getInterfaceTable()->getNumInterfaces(); i++)
        {
            auto ie = getInterfaceTable()->getInterface(i);
            auto name = ie->getInterfaceName();
            if (strstr(name, "wlan")!=nullptr)
            {
                i_face = ie;
                num_80211++;
                interfaceId = ie->getInterfaceId();
            }
        }
        // One enabled network interface (in total)
        if (num_80211!=1)
            throw cRuntimeError("DSR has found %i 80211 interfaces", num_80211);

        /* Initilize tables */
        neigh_tbl_init();
        rreq_tbl_init();
        grat_rrep_tbl_init();
        maint_buf_init();
        send_buf_init();
        etxNumRetry = -1;
        etxActive = par("ETX_Active");
        if (etxActive)
        {
            etxTime = par("ETXHelloInterval");
            etxNumRetry = par("ETXRetryBeforeFail");
            etxWindowSize = etxTime*(unsigned int)par("ETXWindowNumHello");
            etxJitter = 0.1;
            etx_timer.init(&DSRUU::EtxMsgSend, nullptr);
            set_timer(&etx_timer, 0.0);
            //set_timer(&etx_timer, etxTime);
            etxWindow = 0;
            etxSize = 100; // Minimun length
        }
        auto ie = interface80211ptr;
        myaddr_.s_addr = ie->getNetworkAddress();
        macaddr_ = ie->getMacAddress();

        if (!par("UseNetworkLayerAck").boolValue())
        {
            linkLayerFeeback();
            // host->subscribe(linkBrokenSignal, this);
            //host->subscribe(NF_LINK_BREAK, this);
            // host->subscribe(NF_TX_ACKED, this);
        }

//        if (get_confval(PromiscOperation))
//            host->subscribe(NF_LINK_PROMISCUOUS, this);
        // clear routing entries related to wlan interfaces and autoassign ip adresses
        bool manetPurgeRoutingTables = (bool) par("manetPurgeRoutingTables");
        if (manetPurgeRoutingTables) {
            // clean the route table wlan interface entry
            for (int i = this->getInetRoutingTable()->getNumRoutes()-1; i>=0; i--) {
                auto entry = getInetRoutingTable()->getRoute(i);
                const InterfaceEntry *ie = entry->getInterface();
                if (strstr(ie->getInterfaceName(), "wlan")!=nullptr) {
                    getInetRoutingTable()->deleteRoute(entry);
                }
            }
        }
        auto interface = interface80211ptr;
        CHK(interface->getProtocolData<Ipv4InterfaceData>())->joinMulticastGroup(Ipv4Address::LL_MANET_ROUTERS);
        struct in_addr myAddr = my_addr();
        pathCacheMap.setRoot(myAddr.s_addr);
        is_init = true;
        EV_INFO << "Dsr active" << "\n";
        WATCH_MAP(pathCacheMap.pathsCache);
    }
    return;
}

void DSRUU::finish()
{
    neigh_tbl_cleanup();
    rreq_tbl_cleanup();
    grat_rrep_tbl_cleanup();
    send_buf_cleanup();
    maint_buf_cleanup();
    pathCacheMap.cleanAllDataBase();

}

DSRUU::DSRUU():ManetRoutingBase()
{
    lifoDsrPkt = nullptr;
    lifo_token = 0;
    grat_rrep_tbl_timer_ptr = new DSRUUTimer(this);
    send_buf_timer_ptr = new DSRUUTimer(this);
    neigh_tbl_timer_ptr = new DSRUUTimer(this);
    lc_timer_ptr = new DSRUUTimer(this);
    ack_timer_ptr = new DSRUUTimer(this);
    etx_timer_ptr = new DSRUUTimer(this);
    is_init = false;
    rreqInfoMap.clear();
}

DSRUU::~DSRUU()
{
    neigh_tbl_cleanup();
    rreq_tbl_cleanup();
    grat_rrep_tbl_cleanup();
    send_buf_cleanup();
    maint_buf_cleanup();
    pathCacheMap.cleanAllDataBase();
    rreqInfoMap.clear();
    struct dsr_pkt * pkt;
    pkt = lifoDsrPkt;
// delete ETX
    while (!etxNeighborTable.empty())
    {
        auto i = etxNeighborTable.begin();
        delete (*i).second;
        etxNeighborTable.erase(i);
    }

// Delete Timers
    delete grat_rrep_tbl_timer_ptr;
    delete send_buf_timer_ptr;
    delete neigh_tbl_timer_ptr;
    delete lc_timer_ptr;
    delete ack_timer_ptr;
    delete etx_timer_ptr;
// Clean the Lifo queue
    while (pkt!=nullptr)
    {
        lifoDsrPkt = pkt->next;
        delete pkt;
        pkt = DSRUU::lifoDsrPkt;
        lifo_token++;
    }
}

void DSRUU::handleTimer(cMessage* msg)
{
    if (ack_timer.testAndExcute(msg))
        return;
    else if (grat_rrep_tbl_timer.testAndExcute(msg))
        return;
    else if (send_buf_timer.testAndExcute(msg))
        return;
    else if (neigh_tbl_timer.testAndExcute(msg))
        return;
    else if (lc_timer.testAndExcute(msg))
        return;
    else if (etx_timer.testAndExcute(msg))
        return;
    else
    {
        rreq_timer_test(msg);
        return;
    }
}

INetfilter::IHook::Result DSRUU::processPacket(Packet *pkt, unsigned int) {
    // DSR will take control of the packets

    const auto &ipHeader = pkt->peekAtFront<Ipv4Header>();
    //const auto& networkHeader = getNetworkProtocolHeader(pkt);
    const L3Address& destAddr = ipHeader->getDestinationAddress();

    if (destAddr.isBroadcast() || isLocalAddress(destAddr) || destAddr.isMulticast())
        return ACCEPT;

    struct in_addr nxt_hop;
    nxt_hop.s_addr = destAddr;

    const auto &dsrHeader = findDsrProtocolHeader(pkt);
    if (dsrHeader) {
        if (dsrHeader->getPrevAddress() == this->my_addr().s_addr)
            return ACCEPT;
    }
    else {
        // if the destination is 1 hopt, send the packet.
        // This shouldn't really happen ?
        EV_INFO << "Data packet from "<< destAddr <<"without DSR header!n";
        // one hop?

        auto srt = RouteFind(my_addr(), nxt_hop);
        if (srt && (srt->addrs.empty())) {
            pkt->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(srt->dst.s_addr);
            return ACCEPT;
        }
    }

    // check if the routing table has an entry for this packet

    if (omnet_exist_rte(nxt_hop))
        return ACCEPT;

    // check the interface that the DSR is using: ??? Not sure of thi
    auto tag = pkt->findTag<InterfaceInd>();

    if (tag) {
        auto ie = this->getInterfaceEntryById(tag->getInterfaceId());
        if (ie != interface80211ptr)
            return ACCEPT;
    }

    // must be stolen and processed.
    take(pkt);
    defaultProcess(pkt);
    return STOLEN;
    // TODO: check if the destination is a neighbor node, set the next hop and acept.
    // in other case steal the packet
}

INetfilter::IHook::Result DSRUU::ensureRouteForDatagram(Packet *datagram)
{

    if (isInMacLayer()) {
        // TODO: MAC layer
        throw cRuntimeError("Error DSR-uu HOOK");
        return ACCEPT;
    }

    const auto &networkHeader = datagram->peekAtFront<Ipv4Header>();
    const L3Address& destAddr = networkHeader->getDestinationAddress();

    // check if it has DSR header
    if (networkHeader->getProtocolId() == IP_PROT_DSR) {
        const auto& networkHeader = removeNetworkProtocolHeader<Ipv4Header>(datagram);
        const auto &chunk = datagram->peekAtFront<DSRPkt>();
        if (chunk)
            datagram->addTagIfAbsent<DsrProtocolInd>()->setDsrProtocolHeader(chunk);
        insertNetworkProtocolHeader(datagram, Protocol::ipv4, networkHeader);
        take(datagram);
        mainProcess(datagram);
        return STOLEN;
    }

    if (destAddr.isBroadcast() || isLocalAddress(destAddr) || destAddr.isMulticast())
        return ACCEPT;

    unsigned int ifindex = interface80211ptr->getInterfaceId();  /* Always use ns interface */
    INetfilter::IHook::Result res = processPacket(datagram,ifindex);   // Data path
    scheduleEvent();
    return res;

}

void DSRUU::handleMessageWhenUp(cMessage *msg)
{
    if (!nodeActive) {
        delete msg;
        return;
    }

    if (is_init == false)
        throw cRuntimeError("Dsr has not been initialized ");


    if (msg->isSelfMessage() && checkTimer(msg)) {
        scheduleEvent();
        return;
    }
    else if (msg->isSelfMessage()) {
        handleTimer(msg);
        scheduleEvent();
        return;
    }

    // Ext messge
    throw cRuntimeError("A packet has arrived to DSR, DSR should stolen the datagrams from IP");
    mainProcess(check_and_cast<Packet *>(msg));
/*
    IPv4Datagram * ipDgram = nullptr;
    if (dynamic_cast<IPv4Datagram *>(msg)) {
        ipDgram = dynamic_cast<IPv4Datagram *>(msg);
    }
    else {
        // recapsulate and send
        if (proccesICMP(msg))
            return;
        //EV << "############################################################\n";
        //EV << "!!WARNING: DSR has received not supported packet, delete it \n";
        //EV << "############################################################\n";
        //delete msg;
        //return;
        throw cRuntimeError("DSR has received not supported packet");
    }
    DEBUG("##########\n");
    if (ipDgram->getSrcAddress().isUnspecified())
        ipDgram->setSrcAddress(interface80211ptr->ipv4Data()->getIPAddress());
    // Process a Dsr message
*/
    //defaultProcess(check_and_cast<Packet *>(msg));
    return;

}

void DSRUU::mainProcess(Packet *pkt)
{
    auto dsrHeader = findDsrProtocolHeader(pkt);
    if (dsrHeader) {
        auto etxHeader = dynamicPtrCast<DSRPktExt>(
                constPtrCast<DSRPkt>(dsrHeader));
        if (etxHeader) {
            EtxMsgProc(pkt);
            return;
        }
        else {
            //
            const auto& networkHeader = getNetworkProtocolHeader(pkt);
            const L3Address& destAddr = networkHeader->getDestinationAddress();
            const L3Address& nextAddr = dsrHeader->getNextAddress();
            if (destAddr.isBroadcast() || isLocalAddress(destAddr)
                    || destAddr.isMulticast() || isLocalAddress(nextAddr))
                defaultProcess(pkt);
            else
                throw cRuntimeError(
                        "The packet must be stolen from the IP layer with the Ipv4 header and a valid address");
            return;
        }

    }
    delete pkt;
    return;
}



void DSRUU::defaultProcess(Packet * ipDgram)
{
    struct dsr_pkt *dp;
    dp = dsr_pkt_alloc(ipDgram); // crear estructura dsr
    int protocol = dp->nh.iph->protocol;
    switch (protocol)
    {
    case IP_PROT_DSR:
        if (dp->src.s_addr != myaddr_.s_addr)
        {
            dsr_recv(dp);
        }
        else
        {
            dsr_pkt_free(dp);
//          DEBUG("Locally generated DSR packet\n");
        }
        break;
    default:
        if (dp->src.s_addr == myaddr_.s_addr)
        {
            dp->payload_len += IP_HDR_LEN;
            DEBUG("Local packet without DSR header\n");
            dsr_start_xmit(dp);
        }
        else
        {
            // This shouldn't really happen ?
            DEBUG("Data packet from %s without DSR header!n",
                  print_ip(dp->src));
            dsr_pkt_free(dp);
        }
        break;
    }
}

void DSRUU::processLinkBreak(const Packet *pkt)
{
    if (pkt == nullptr)
        return;

    const auto& header80211 = pkt->peekAtFront<Ieee80211DataOrMgmtHeader>();
    if (header80211 == nullptr)
        return;

    auto sender = header80211->getTransmitterAddress();
    auto receiver = header80211->getReceiverAddress();

    const auto& networkHeader = getNetworkProtocolHeader(const_cast<Packet *>(pkt));
    if (networkHeader == nullptr)
        return;

    packetFailed(pkt);
}


#if 0
void DSRUU::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{

    //current_time = simTime();

    if (obj==nullptr)
        return;

/*    if (signalID == NF_TX_ACKED && !get_confval(UseNetworkLayerAck))
        EV << "DSR TX ACK \n";
    else if (signalID == NF_LINK_BREAK && !get_confval(UseNetworkLayerAck))
        EV << "DSR link breack \n";
    else if (signalID == NF_LINK_PROMISCUOUS && get_confval(PromiscOperation))
        EV << "Dsr promisc \n";
    else
        return;
*/
    Enter_Method("Dsr receiveSignal");
    MacAddress sender;
    MacAddress receiver;

/*    Packet  *dgram = dynamic_cast<Ieee80211DataFrame *>(obj);

    if (frame)
    {
        if (dynamic_cast<IPv4Datagram *>(frame->getEncapsulatedPacket()))
            dgram = check_and_cast<IPv4Datagram *>(frame->getEncapsulatedPacket());
        else
            return;
        sender = frame->getTransmitterAddress();
        receiver = frame->getReceiverAddress();
    }
    else
    {


    }
    if (signalID == linkBrokenSignal) {
        if (dgram)
            packetFailed(dgram);
    }
*/

/*  if (signalID == NF_TX_ACKED)
    {
        if (dgram)
            packetLinkAck(dgram);
    }
    else if (signalID == linkBrokenSignal)
    {
        if (dgram)
            packetFailed(dgram);
    }
    else if (signalID == NF_LINK_PROMISCUOUS)
    {
        if (dynamic_cast<DSRPkt *>(dgram))
        {
            DSRPkt *paux = check_and_cast <DSRPkt *> (frame->getEncapsulatedPacket());
            // DSRPkt *p = check_and_cast <DSRPkt *> (paux->dup());
            // take(p);
            EV << "####################################################\n";
            EV << "Dsr protocol received promiscuous packet from " << paux->getSrcAddress() << "\n";
            EV << "#####################################################\n";
            Ieee802Ctrl *ctrl = new Ieee802Ctrl();
            ctrl->setSrc(sender);
            ctrl->setDest(receiver);
            //  p->setControlInfo(ctrl);
            tap(paux,ctrl);
        }
    }*/
}
#endif

void DSRUU::packetLinkAck(Packet *ipDgram)
{
    struct in_addr nxt_hop;
    auto dsrPkt = findDsrProtocolHeader(ipDgram);
    if (dsrPkt == nullptr)
    {
        const auto& networkHeader = findNetworkProtocolHeader(ipDgram);
        if (networkHeader != nullptr) {
            auto ipHeader = dynamicPtrCast<Ipv4Header>(constPtrCast<NetworkHeaderBase>(networkHeader));
            if (ipHeader != nullptr ) {
                // This shouldn't really happen ?
                EV << "Data packet from "<< ipHeader->getDestAddress() <<"without DSR header!n";
                // one hop?
                struct dsr_srt *srt;
                nxt_hop.s_addr = ipHeader->getDestAddress();
                if (ConfVal(PathCache))
                    srt = ph_srt_find_map(my_addr(), nxt_hop, 0);
                else
                    srt = ph_srt_find_link_route_map(my_addr(),nxt_hop, 0);
                if (srt && srt->addrs.empty())
                    maint_buf_del_all(nxt_hop);
            }
        }
        return;
    }

    nxt_hop.s_addr = dsrPkt->getNextAddress();
    maint_buf_del_all(nxt_hop);
    return;
}

void DSRUU::packetFailed(const Packet *pktAux)
{
    struct dsr_pkt *dp;
    struct in_addr dst, nxt_hop;
    //struct in_addr prev_hop;
    /* Cast the packet so that we can touch it */
    /* Do nothing for my own packets... */

    const auto& header80211 = pktAux->peekAtFront<Ieee80211DataOrMgmtHeader>();
    if (header80211 == nullptr)
        return;

    auto senderAddr = header80211->getTransmitterAddress();

    Packet *ipDgram = const_cast<Packet *> (pktAux);
    const auto& networkHeader = getNetworkProtocolHeader(ipDgram);
    L3Address destination = networkHeader->getDestinationAddress();
    L3Address source = networkHeader->getSourceAddress();
    const auto &dsrPkt = findDsrProtocolHeader(ipDgram);

    if (dsrPkt == nullptr) {

        // This shouldn't really happen ?
        EV_INFO << "Data packet from "<< source <<"without DSR header!n";
        // one hop?
        struct dsr_srt *srt;
        nxt_hop.s_addr = destination;
        if (ConfVal(PathCache))
            srt = ph_srt_find_map(my_addr(), nxt_hop, 0);
        else
            srt = ph_srt_find_link_route_map(my_addr(),nxt_hop, 0);
        if (srt && (srt->addrs.empty() || srt->dst.s_addr == nxt_hop.s_addr))
            ph_srt_delete_link_map(my_addr(), nxt_hop);// one hop
        return;
    }

    PacketDissector::PduTreeBuilder pduTreeBuilder;
    auto packetProtocolTag = ipDgram->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    PacketDissector packetDissector(ProtocolDissectorRegistry::globalRegistry, pduTreeBuilder);
    packetDissector.dissectPacket(const_cast<Packet *> (ipDgram), protocol);

    auto& protocolDataUnit = pduTreeBuilder.getTopLevelPdu();

    bool isIpv4 = false;
    bool isDsr = false;

    for (const auto& chunk : protocolDataUnit->getChunks()) {
        if (auto childLevel = dynamicPtrCast<const PacketDissector::ProtocolDataUnit>(chunk)) {
            for (const auto& chunkAux : childLevel->getChunks()) {
                if (chunkAux->getChunkType() == Chunk::CT_SEQUENCE) {
                    for (const auto& elementChunk : staticPtrCast<const SequenceChunk>(chunkAux)->getChunks()) {
                        if (dynamic_cast<const Ipv4Header *>(elementChunk.get()))
                            isIpv4 = true;
                        if (dynamic_cast<const DSRPkt *>(elementChunk.get()))
                            isDsr = true;
                    }
                }
            }
        }
        else if (chunk->getChunkType() == Chunk::CT_SEQUENCE) {
            for (const auto& elementChunk : staticPtrCast<const SequenceChunk>(chunk)->getChunks()) {
                if (dynamic_cast<const Ipv4Header *>(elementChunk.get()))
                    isIpv4 = true;
                if (dynamic_cast<const DSRPkt *>(elementChunk.get()))
                    isDsr = true;
            }
        }
    }

    if (!isIpv4)
        return; // nothing more to do

    // create a copy of this packet
    auto pkt = new Packet(ipDgram->getName());

    for (const auto& chunk : protocolDataUnit->getChunks()) {
        if (auto childLevel = dynamicPtrCast<const PacketDissector::ProtocolDataUnit>(chunk)) {
            for (const auto& chunkAux : childLevel->getChunks()) {
                if (chunkAux->getChunkType() == Chunk::CT_SEQUENCE) {
                    // remove previous headers to ipv4
                    bool removed = false;
                    for (const auto& elementChunk : staticPtrCast<const SequenceChunk>(chunkAux)->getChunks()) {
                        if (elementChunk == networkHeader) {
                            removed = true;
                            insertNetworkProtocolHeader(pkt, Protocol::ipv4, dynamicPtrCast<NetworkHeaderBase> (networkHeader->dupShared()));
                        }
                        else if (removed) {
                            if (elementChunk == dsrPkt)
                                insertDsrProtocolHeader(pkt, dynamicPtrCast<DSRPkt> (networkHeader->dupShared()));
                            else
                                pkt->insertAtBack(elementChunk->dupShared());
                        }
                    }
                }
            }
        }
        else if (chunk->getChunkType() == Chunk::CT_SEQUENCE) {
            if (staticPtrCast<const SequenceChunk>(chunk)->getChunks().front() != networkHeader){
                delete pkt;
                return;
            }
            for (const auto& elementChunk : staticPtrCast<const SequenceChunk>(chunk)->getChunks()) {
                if (elementChunk == networkHeader) {
                    insertNetworkProtocolHeader(pkt, Protocol::ipv4, dynamicPtrCast<NetworkHeaderBase> (networkHeader->dupShared()));
                }
                else
                    if (elementChunk == dsrPkt)
                        insertDsrProtocolHeader(pkt, dynamicPtrCast<DSRPkt> (networkHeader->dupShared()));
                    else
                        pkt->insertAtBack(elementChunk->dupShared());
            }
        }
    }
    pkt->copyTags(*ipDgram);
    if (isDsr)
        pkt->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::dsr);
    else
        pkt->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);

    for (int index = 0; index < getNumInterfaces(); index++)  {
        if (getInterfaceEntry(index)->getMacAddress() == senderAddr) {
            pkt->addTagIfAbsent<InterfaceInd>()->setInterfaceId(getInterfaceEntry(index)->getInterfaceId());
            break;
        }
    }

    // pkt is a duplicate of the packet that have removed all headers previous to network header

    //prev_hop.s_addr = p->prevAddress().getInt();
    dst.s_addr = destination;
    nxt_hop.s_addr = dsrPkt->getNextAddress();

    DEBUG("Xmit failure for %s nxt_hop=%s\n", print_ip(dst), print_ip(nxt_hop));
    ph_srt_delete_link_map(my_addr(), nxt_hop);
    dp = dsr_pkt_alloc(pkt);
    if (!dp) {
        DEBUG("Could not allocate DSR packet\n");
        drop(pkt, ICMP_DESTINATION_UNREACHABLE);
    }
    else {
        dsr_rerr_send(dp, nxt_hop);
        dp->nxt_hop = nxt_hop;
        if (maint_buf_salvage(dp) < 0) {
            if (dp->payload)
                drop(dp->payload, -1);
            dp->payload = nullptr;
            dsr_pkt_free(dp);
        }
    }

    /* Salvage the other packets still in the interface queue with the same
     * next hop */
    /* mac access
        while ((qp = ifq_->prq_get_nexthop(cmh->next_hop()))) {

                dp = dsr_pkt_alloc(qp);

                if (!dp) {
                    drop(qp, ICMP_DESTINATION_UNREACHABLE);
                    continue;
                }

                dp->nxt_hop = nxt_hop;

                maint_buf_salvage(dp);
            }
    */
    return;
}


void DSRUU::linkFailed(Ipv4Address ipAdd)
{

    struct in_addr nxt_hop;

    /* Cast the packet so that we can touch it */
    nxt_hop.s_addr = L3Address(ipAdd);
    ph_srt_delete_link_map(my_addr(), nxt_hop);
    return;

}


void DSRUU::tap(Packet *p)
{
    struct dsr_pkt *dp;

    const auto &ipHeader = p->peekAtFront<Ipv4Header>();

    //struct in_addr next_hop, prev_hop;
    //next_hop.s_addr = p->nextAddress().getInt();
    //prev_hop.s_addr = p->prevAddress().getInt();
    auto protocol = p->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol == nullptr)
        throw cRuntimeError("Protocl not found");

    IpProtocolId transportProtocol = (IpProtocolId)ProtocolGroup::ipprotocol.getProtocolNumber(protocol);


    if (transportProtocol != ipHeader->getProtocolId())
        throw cRuntimeError("protocol tag and protocol id are diferent");

    /* Cast the packet so that we can touch it */
    dp = dsr_pkt_alloc(p);
    dp->flags |= PKT_PROMISC_RECV;

    /* TODO: See if this node is the next hop. In that case do nothing */
    switch (transportProtocol)
    {
    case IP_PROT_DSR:
        if (dp->src.s_addr != myaddr_.s_addr) /* Do nothing for my own packets... */
        {
            //DEBUG("DSR packet from %s\n", print_ip(dp->src));
            dsr_recv(dp);
        }
        else
        {
//          DEBUG("Locally gernerated DSR packet\n");
            dsr_pkt_free(dp);
        }
        break;
    default:
        // This shouldn't really happen ?
        DEBUG("Data packet from %s without DSR header!n",
              print_ip(dp->src));
        dsr_pkt_free(dp);
        break;
    }
    return;
}



struct dsr_srt *DSRUU:: RouteFind(struct in_addr src, struct in_addr dst)
{
    if (ConfVal(PathCache))
        return ph_srt_find_map(src, dst, ConfValToUsecs(RouteCacheTimeout));
        //return ph_srt_find(src, dst, 0, ConfValToUsecs(RouteCacheTimeout));
    else
    {
        return  ph_srt_find_link_route_map(src,dst, ConfValToUsecs(RouteCacheTimeout));
     //   return lc_srt_find(src, dst);
    }

}

int DSRUU::RouteAdd(struct dsr_srt *srt, unsigned long timeout, unsigned short flags)
{


    if (ConfVal(PathCache))
    {
        //ph_srt_add(srt, timeout, flags);
        ph_srt_add_map(srt, timeout, flags);
        return 0;
    }
    else
    {
        ph_srt_add_link_map(srt,timeout);
        return 0;
        //return lc_srt_add(srt, timeout, flags);
    }
}

void DSRUU::EtxMsgSend(void *data) {
    EtxList neigh[15];
    auto etxHeader = makeShared<DSRPktExt>();
    int numNeighbor = 0;
    for (auto iter = etxNeighborTable.begin(); iter != etxNeighborTable.end();) {
        // remove old data
        ETXEntry *entry = (*iter).second;
        while (!entry->timeVector.empty()
                && simTime() - entry->timeVector.front() > etxWindowSize)
            entry->timeVector.erase(entry->timeVector.begin());
        if (entry->timeVector.size() == 0) {
            linkFailed((*iter).first);
            etxNeighborTable.erase(iter);
            iter = etxNeighborTable.begin();
            continue;
        }

        double delivery;
        delivery = entry->timeVector.size() / (etxWindowSize / etxTime);
        if (delivery > 0.99)
            delivery = 1;
        entry->deliveryDirect = delivery;

        if (numNeighbor < 15) {
            neigh[numNeighbor].address = iter->first;
            neigh[numNeighbor].delivery = delivery; //(uint32_t)(delivery*0xFFFF); // scale
            numNeighbor++;
            if (neigh[numNeighbor - 1].delivery <= 0)
                printf("\n recojones");
        }
        else {
            // delete
            int aux = 0;
            for (int i = 1; i < 15; i++) {
                if (neigh[i].delivery < neigh[aux].delivery)
                    aux = i;
            }
            if (neigh[aux].delivery < delivery) // (uint32_t) (delivery*0xFFFF))
                    {
                neigh[aux].delivery = delivery; //(uint32_t) (delivery*0xFFFF);
                neigh[aux].address = (*iter).first;
                if (neigh[aux].delivery <= 0)
                    printf("\recojones");
            }
        }
        iter++;
    }
    Packet * pkt = new Packet();

    EtxList *list = etxHeader->addExtension(numNeighbor);
    memcpy(list, neigh, sizeof(EtxList) * numNeighbor);
    etxHeader->setChunkLength(B(8*numNeighbor));
    insertDsrProtocolHeader(pkt, etxHeader);

    Ipv4Address destAddress_var(DSR_BROADCAST);
    auto ipHeader = makeShared<Ipv4Header>();
    ipHeader->setDestAddress(destAddress_var);
    Ipv4Address srcAddress_var = myaddr_.s_addr.toIpv4();
    ipHeader->setSrcAddress(srcAddress_var);
    ipHeader->setTimeToLive(1); // TTL

    ipHeader->setProtocolId(IP_PROT_DSR);

    ipHeader->setCrcMode(CRC_DECLARED_CORRECT);
    ipHeader->setCrc(0);
    ipHeader->setTotalLengthField(ipHeader->getChunkLength() + etxHeader->getChunkLength());

    pkt->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::dsr);

    insertNetworkProtocolHeader(pkt, Protocol::ipv4, ipHeader);

    pkt->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interface80211ptr->getInterfaceId());

    injectDirectToIp(pkt, uniform(0, etxJitter), nullptr);
    // sendDelayed(pkt, uniform(0, etxJitter), "ipOut");
    etxWindow += etxTime;
    set_timer(&etx_timer, etxTime + SIMTIME_DBL(simTime()));
}

void DSRUU::EtxMsgProc(Packet *pkt)
{
    int pos = -1;

    auto dsrHeader =  findDsrProtocolHeader(pkt);
    auto etxHeader = dynamicPtrCast<DSRPktExt> (constPtrCast<DSRPkt>(dsrHeader));

    const auto& networkHeader = getNetworkProtocolHeader(pkt);
    auto ipHeader = dynamicPtrCast<Ipv4Header>(constPtrCast<NetworkHeaderBase>(networkHeader));


    EtxList *list = etxHeader->getExtension();
    int size = etxHeader->getSizeExtension();

    Ipv4Address myAddress(myaddr_.s_addr.toIpv4());
    Ipv4Address srcAddress(ipHeader->getSrcAddress());


    for (int i = 0; i<size; i++) {
        if (list[i].address == myAddress) {
            pos = i;
            break;
        }
    }
    auto it = etxNeighborTable.find(srcAddress);
    ETXEntry *entry = nullptr;
    if (it==etxNeighborTable.end()) {
        // add
        entry = new ETXEntry();
        //entry->address=msg->getSrcAddress();
        etxNeighborTable.insert(std::make_pair(srcAddress, entry));

    }
    else {
        entry = (*it).second;
        while (!entry->timeVector.empty() && simTime()-entry->timeVector.front()>etxWindowSize)
            entry->timeVector.erase(entry->timeVector.begin());
    }
    double delivery;
    entry->timeVector.push_back(simTime());
    delivery = entry->timeVector.size()/(etxWindowSize/etxTime);
    if (delivery>0.99)
        delivery = 1;
    entry->deliveryDirect = delivery;
    if (pos!=-1) {
        //unsigned int cost = (unsigned int) list[pos].delivery;
        //entry->deliveryReverse= ((double)cost)/0xFFFF;
        entry->deliveryReverse = list[pos].delivery;
        if (entry->deliveryDirect<=0)
            printf("Cojones");
    }
    delete pkt;
}

double DSRUU::getCost(Ipv4Address add)
{
    auto it = etxNeighborTable.find(add);
    if (it==etxNeighborTable.end())
        return -1;
    ETXEntry *entry = (*it).second;
    if (entry->deliveryReverse==0 || entry->deliveryDirect==0)
        return -1;
    return (1/(entry->deliveryReverse * entry->deliveryDirect));
}

void DSRUU::ExpandCost(struct dsr_pkt *dp)
{
    struct in_addr myAddr;
    if (!etxActive)
    {
        if (!dp->costVector.empty())
            dp->costVector.clear();
        return;
    }
    myAddr = my_addr();
    Ipv4Address myAddress(myAddr.s_addr.toIpv4());
    if ((dp->costVector.empty() && dp->src.s_addr != myAddr.s_addr) ||  !dp->costVector.empty())
    {
        EtxCost val;
        val.address = myAddress;
        double cost = getCost(dp->src.s_addr.toIpv4());

        if (cost<0)
            val.cost = 1e100;
        else
            val.cost = cost;
        dp->costVector.push_back(val);
    }
    /*
    for (int i=0;i<dp->costVectorSize;i++)
    if (dp->costVector[i].cost>10)
     printf("\n");*/
}

double DSRUU::PathCost(struct dsr_pkt *dp)
{
    double totalCost;
    if (!etxActive)
    {
        totalCost = (double)(DSR_RREQ_ADDRS_LEN(dp->rreq_opt.front())/DSR_ADDRESS_SIZE);
        totalCost += 1;
        return totalCost;
    }
    totalCost = 0;
    for (unsigned int i=0; i<dp->costVector.size(); i++)
    {
        totalCost += dp->costVector[i].cost;
    }
    double cost;
    if (!dp->costVector.empty())
        cost = getCost(dp->costVector[dp->costVector.size()-1].address.toIpv4());
    else
    {
        cost = getCost(dp->src.s_addr.toIpv4());
    }

    if (cost<0)
        cost = 1e100;
    totalCost += cost;
    return totalCost;
}


void DSRUU::AddCost(struct dsr_pkt *dp, struct dsr_srt *srt)
{
    struct in_addr add;

    if (!dp->costVector.empty())
        dp->costVector.clear();
    if (!etxActive)
        return;
    if (srt->cost.empty())
        return;

    // int sizeAddress = srt->laddrs/ DSR_ADDRESS_SIZE;

    EtxCost val;
    for (int i=0; i<(int)srt->addrs.size(); i++)
    {

        add = srt->addrs[i];
        val.address = add.s_addr;
        val.cost = srt->cost[i];
        dp->costVector.push_back(val);
    }
    val.address = srt->dst.s_addr;
    val.cost = srt->cost[srt->cost.size()-1];
    dp->costVector.push_back(val);
}

void DSRUU::ActualizeMyCostRrep(dsr_rrep_opt *rrepOpt)
{
    if (!etxActive && !rrepOpt->cost.empty())
    {
        rrepOpt->cost.clear();
        return;
    }
    // search my address
    if (rrepOpt->addrs.size()  != rrepOpt->cost.size())
    {
        // do nothing
        rrepOpt->cost.clear();
        return;
    }
    for (int i = (int)rrepOpt->addrs.size()-2; i>=0; i--)
    {
        if (rrepOpt->addrs[i] != my_addr().s_addr)
            continue;
        rrepOpt->cost[i+1] = getCost(rrepOpt->addrs[i+1].toIpv4());
        return;
    }
    rrepOpt->cost[0] = getCost(rrepOpt->addrs[0].toIpv4());
}

void DSRUU::AddCostRrep(struct dsr_pkt *dp, struct dsr_srt *srt)
{
    struct in_addr add;

    if (!dp->costVector.empty())
        dp->costVector.clear();
    if (!etxActive)
        return;

    // int sizeAddress = srt->laddrs/ DSR_ADDRESS_SIZE;
    // search my address

    EtxCost val;


    for (int i=0; i<(int)srt->addrs.size(); i++)
    {

        add = srt->addrs[i];
        if (add.s_addr == my_addr().s_addr)
            break;
        add = srt->addrs[i];
        val.address = add.s_addr;
        val.cost = srt->cost[i];
        dp->costVector.push_back(val);
    }

    double cost;
    if (!dp->costVector.empty())
        cost = getCost(dp->costVector[dp->costVector.size()-1].address.toIpv4());
    else
    {
        cost = getCost(dp->src.s_addr.toIpv4());
    }

    val.address = my_addr().s_addr;
    val.cost = cost;
    dp->costVector.push_back(val);
}

bool DSRUU::proccesICMP(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();

    if (protocol != &Protocol::icmpv4)
        return false;

    auto icmpHeader = packet->peekAtFront<IcmpHeader>();
    if (icmpHeader == nullptr) {
        delete msg;
        return true;
    }

    auto dsrHeader = findDsrProtocolHeader(packet);

    if (dsrHeader == nullptr)
    {
        delete msg;
        return true;
    }
    // TODO: Handle ICMP packets.
    // check if is a exclusive DSR packetç
    /*
    if (bogusPacket->getEncapProtocol()==0)
    {
        auto pkt
        // delete all and return
        delete msg;
        return true;
    }

    Ipv4Datagram *newdgram = new IPv4Datagram();
    bogusPacket->setTransportProtocol(bogusPacket->getEncapProtocol());
    Ipv4Datagram dst = this->my_addr().s_addr.toIpv4();
    newdgram->setDestAddress(dst);
    ICMPMessage * icmpMsg = new ICMPMessage();
    icmpMsg->setType(pk->getType());
    icmpMsg->setCode(pk->getCode());
    icmpMsg->encapsulate(bogusPacket->dup());
    newdgram->encapsulate(icmpMsg);
    newdgram->setTransportProtocol(IP_PROT_ICMP);
    send(newdgram,"ipOut");
    */
    delete msg;
    return true;
 }


void DSRUU::handleStartOperation(LifecycleOperation *operation)
{
    nodeActive = true;
}

void DSRUU::handleStopOperation(LifecycleOperation *operation)
{
    nodeActive = false;
    pathCacheMap.cleanAllDataBase();
    neigh_tbl_cleanup();
    rreq_tbl_cleanup();
    grat_rrep_tbl_cleanup();
    send_buf_cleanup();
    maint_buf_cleanup();
    while (!etxNeighborTable.empty()) {
        auto i = etxNeighborTable.begin();
        delete (*i).second;
        etxNeighborTable.erase(i);
    }
    grat_rrep_tbl_timer_ptr->cancel();
    send_buf_timer_ptr->cancel();
    neigh_tbl_timer_ptr->cancel();
    lc_timer_ptr->cancel();
    ack_timer_ptr->cancel();
    etx_timer_ptr->cancel();

}

void DSRUU::handleCrashOperation(LifecycleOperation *operation)
{
    nodeActive = false;
    pathCacheMap.cleanAllDataBase();
    neigh_tbl_cleanup();
    rreq_tbl_cleanup();
    grat_rrep_tbl_cleanup();
    send_buf_cleanup();
    maint_buf_cleanup();
    while (!etxNeighborTable.empty())
    {
        auto i = etxNeighborTable.begin();
        delete (*i).second;
        etxNeighborTable.erase(i);
    }
    grat_rrep_tbl_timer_ptr->cancel();
    send_buf_timer_ptr->cancel();
    neigh_tbl_timer_ptr->cancel();
    lc_timer_ptr->cancel();
    ack_timer_ptr->cancel();
    etx_timer_ptr->cancel();
}




// Constructor
Packet * DSRUU::newDsrPacket(struct dsr_pkt *dp, int interface_id, bool withDsrHeader)
{
    if (dp == nullptr)
        return nullptr;

    Packet *pkt = nullptr;
    if(dp->payload) {
        pkt = dp->payload;
        pkt->trim();
        pkt->clearTags();
        dp->payload = nullptr;
    }
    else {
        pkt = new Packet();
    }

    if (withDsrHeader) {
        const auto& dsrPkt = makeShared<DSRPkt>();
        dsrPkt->cleanAll();

        dsrPkt->setChunkLength(B(DSR_OPT_HDR_LEN + dp->dh.opth.begin()->p_len));
        dsrPkt->setDsrOptions(dp->dh.opth);
        // ¿como gestionar el MAC
        // dp->mac.raw = p->access(hdr_mac::offset_);

        if (dp->costVector.size() > 0) {
            dsrPkt->setCostVector(dp->costVector);
            dp->costVector.clear();
        }

        if (!dp->dst.s_addr.toIpv4().isLimitedBroadcastAddress())
            dsrPkt->setNextAddress(dp->nxt_hop.s_addr);

        dsrPkt->setPrevAddress(this->my_addr().s_addr);

        dsrPkt->setEncapProtocol(dp->encapsulate_protocol);

        insertDsrProtocolHeader(pkt, dsrPkt);
        if (pkt->getName() == nullptr || strcmp(pkt->getName(), "") == 0) {
            auto dopt = dp->dh.opth.front().option.front();
            switch (dopt->type) {
            case DSR_OPT_PADN:
                break;
            case DSR_OPT_RREQ:
                pkt->setName("DsrRREQ");

                break;
            case DSR_OPT_RREP:
                pkt->setName("DsrRREP");
                break;
            case DSR_OPT_RERR:
                pkt->setName("DsrRERR");
                break;
            case DSR_OPT_PREV_HOP:
                break;
            case DSR_OPT_ACK:
                pkt->setName("DsrACK");
                break;
            case DSR_OPT_SRT:
                pkt->setName("DsrSRT");
                break;
            }
        }

    }
    else {

        pkt->addTagIfAbsent<PacketProtocolTag>()->setProtocol(Protocol::getProtocol(dp->nh.iph->protocol));
        // only if it is not a DSR packet
        if (dp->inputInterfaceId != -1)
            pkt->addTagIfAbsent<InterfaceInd>()->setInterfaceId(dp->inputInterfaceId);

    }

    const auto& ipHeader = makeShared<Ipv4Header>();

    ipHeader->setDestAddress(dp->dst.s_addr.toIpv4());
    ipHeader->setSourceAddress(dp->src.s_addr.toIpv4());

    if (withDsrHeader)
        ipHeader->setProtocolId(IP_PROT_DSR);
    else if (dp->encapsulate_protocol != -1)
        ipHeader->setProtocolId((IpProtocolId)dp->encapsulate_protocol);
    else
        ipHeader->setProtocolId((IpProtocolId)dp->nh.iph->protocol);

    auto hopLimitReq = pkt->removeTagIfPresent<HopLimitReq>();

    short ttl = dp->nh.iph->ttl;
    if (hopLimitReq)
        delete hopLimitReq;

    if (auto dontFragmentReq = pkt->removeTagIfPresent<FragmentationReq>()) {
        delete dontFragmentReq;
    }
    // set other fields
    if (DscpReq *dscpReq = pkt->removeTagIfPresent<DscpReq>()) {
        delete dscpReq;
    }
    if (EcnReq *ecnReq = pkt->removeTagIfPresent<EcnReq>()) {
        delete ecnReq;
    }
    ipHeader->setTimeToLive(ttl);
    ipHeader->setCrcMode(CRC_DECLARED_CORRECT);
    ipHeader->setCrc(0);
    ipHeader->setHeaderLength(B(dp->nh.iph->ihl)); // Header length
    ipHeader->setChunkLength(ipHeader->getHeaderLength());
    ipHeader->setTotalLengthField(ipHeader->getChunkLength() + pkt->getDataLength());

    ipHeader->setVersion(dp->nh.iph->version); // Ip version
    ipHeader->setTypeOfService(dp->nh.iph->tos); // ToS
    ipHeader->setIdentification(dp->nh.iph->id); // Identification
    ipHeader->setMoreFragments(dp->nh.iph->frag_off & 0x2000);
    ipHeader->setDontFragment(dp->nh.iph->frag_off & 0x4000);
    ipHeader->setTimeToLive(dp->nh.iph->ttl); // TTL

    insertNetworkProtocolHeader(pkt, Protocol::ipv4, ipHeader);

    if (interface_id >= 0) {
        pkt->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interface_id);
    }

    if (!dp->dst.s_addr.toIpv4().isLimitedBroadcastAddress())
        pkt->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(dp->nxt_hop.s_addr);
    else {
        auto addresses = pkt->addTagIfAbsent<L3AddressReq>();
        addresses->setSrcAddress(this->my_addr().s_addr);
        addresses->setDestAddress(dp->dst.s_addr.toIpv4());
    }

    pkt->addTagIfAbsent<HopLimitReq>()->setHopLimit(ttl);

    return pkt;
}

Packet * DSRUU::newDsrPacket(struct dsr_pkt *dp, int interface_id, const Packet *pktAux)
{
    if (dp == nullptr)
        return nullptr;

    if (pktAux == nullptr)
        return nullptr;

    auto pkt = pktAux->dup();

    pkt->trim();
    pkt->clearTags();

    const auto& dsrPkt = makeShared<DSRPkt>();
    dsrPkt->cleanAll();


    dsrPkt->setChunkLength(B(DSR_OPT_HDR_LEN + dp->dh.opth.begin()->p_len));
    dsrPkt->setDsrOptions(dp->dh.opth);

    if (!dp->dst.s_addr.toIpv4().isLimitedBroadcastAddress())
        dsrPkt->setNextAddress(dp->nxt_hop.s_addr);

        // ¿como gestionar el MAC
        // dp->mac.raw = p->access(hdr_mac::offset_);

    if (dp->costVector.size() > 0) {
        dsrPkt->setCostVector(dp->costVector);
        dp->costVector.clear();
    }

    if (dp->dst.s_addr.toIpv4().getInt() != DSR_BROADCAST)
        dsrPkt->setNextAddress(dp->nxt_hop.s_addr);

    dsrPkt->setPrevAddress(this->my_addr().s_addr);

    insertDsrProtocolHeader(pkt, dsrPkt);


    if (pkt->getName() == nullptr || strcmp(pkt->getName(), "") == 0) {
        auto dopt = dp->dh.opth.front().option.front();
        switch (dopt->type) {
        case DSR_OPT_PADN:
            break;
        case DSR_OPT_RREQ:
            pkt->setName("DsrRREQ");

            break;
        case DSR_OPT_RREP:
            pkt->setName("DsrRREP");
            break;
        case DSR_OPT_RERR:
            pkt->setName("DsrRERR");
            break;
        case DSR_OPT_PREV_HOP:
            break;
        case DSR_OPT_ACK:
            pkt->setName("DsrACK");
            break;
        case DSR_OPT_SRT:
            pkt->setName("DsrSRT");
            break;
        }
    }

    const auto& ipHeader = makeShared<Ipv4Header>();

    ipHeader->setDestAddress(dp->dst.s_addr.toIpv4());
    ipHeader->setSourceAddress(dp->src.s_addr.toIpv4());

    ipHeader->setProtocolId((IpProtocolId)dp->nh.iph->protocol);

    auto hopLimitReq = pkt->removeTagIfPresent<HopLimitReq>();

    short ttl = dp->nh.iph->ttl;
    if (hopLimitReq)
        delete hopLimitReq;

    if (auto dontFragmentReq = pkt->removeTagIfPresent<FragmentationReq>()) {
        delete dontFragmentReq;
    }
    // set other fields
    if (DscpReq *dscpReq = pkt->removeTagIfPresent<DscpReq>()) {
        delete dscpReq;
    }
    if (EcnReq *ecnReq = pkt->removeTagIfPresent<EcnReq>()) {
        delete ecnReq;
    }
    ipHeader->setTimeToLive(ttl);
    ipHeader->setCrcMode(CRC_DECLARED_CORRECT);
    ipHeader->setCrc(0);
    ipHeader->setHeaderLength(B(dp->nh.iph->ihl)); // Header length
    ipHeader->setChunkLength(ipHeader->getHeaderLength());
    ipHeader->setTotalLengthField(ipHeader->getChunkLength() + pkt->getDataLength());

    ipHeader->setVersion(dp->nh.iph->version); // Ip version
    ipHeader->setTypeOfService(dp->nh.iph->tos); // ToS
    ipHeader->setIdentification(dp->nh.iph->id); // Identification
    ipHeader->setMoreFragments(dp->nh.iph->frag_off & 0x2000);
    ipHeader->setDontFragment(dp->nh.iph->frag_off & 0x4000);
    ipHeader->setTimeToLive(dp->nh.iph->ttl); // TTL

    pkt->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::dsr);

    insertNetworkProtocolHeader(pkt, Protocol::ipv4, ipHeader);

    if (interface_id >= 0) {
        pkt->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interface_id);
    }
    pkt->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::dsr);
    if (!dp->dst.s_addr.toIpv4().isLimitedBroadcastAddress())
        pkt->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(dp->nxt_hop.s_addr);
    else {
        auto addresses = pkt->addTagIfAbsent<L3AddressReq>();
        addresses->setSrcAddress(this->my_addr().s_addr);
        addresses->setDestAddress(dp->dst.s_addr.toIpv4());
    }
    ipHeader->setProtocolId(IP_PROT_DSR);

    pkt->addTagIfAbsent<HopLimitReq>()->setHopLimit(ttl);

    if (dp->inputInterfaceId != -1)
        pkt->addTagIfAbsent<InterfaceInd>()->setInterfaceId(dp->inputInterfaceId);

    return pkt;
}



} // namespace inetmanet

} // namespace inet

