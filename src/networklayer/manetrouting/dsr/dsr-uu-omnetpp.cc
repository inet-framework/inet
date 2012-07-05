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
#include "dsr-uu-omnetpp.h"
#include "IPv4Address.h"
#include "Ieee802Ctrl_m.h"
#include "Ieee80211Frame_m.h"
#include "ICMPMessage_m.h"

unsigned int DSRUU::confvals[CONFVAL_MAX];
//simtime_t DSRUU::current_time;
struct dsr_pkt * DSRUU::lifoDsrPkt;
int DSRUU::lifo_token;

Define_Module(DSRUU);


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
    DSRPkt *p;

    double jitter = 0;

    if (dp->flags & PKT_REQUEST_ACK)
        maint_buf_add(dp);
    if (dp->ip_pkt)
    {
        p = dp->ip_pkt;
        dp->ip_pkt = NULL;
        p->ModOptions(dp, interfaceId);
    }
    else
        p = new DSRPkt(dp, interfaceId);

    if (!p)
    {
        DEBUG("Could not create packet\n");
        if (dp->payload)
            drop(dp->payload, ICMP_DESTINATION_UNREACHABLE);
        dp->payload = NULL;
        dsr_pkt_free(dp);
        return;
    }


    /* If TTL = 0, drop packet */
    if (p->getTimeToLive() <= 0)
    {
        DEBUG("Dropping packet with TTL = 0.");
        drop(p, ICMP_TIME_EXCEEDED);
        dp->payload = NULL;
        dsr_pkt_free(dp);
        return;
    }

    DEBUG("xmitting pkt src=%d dst=%d nxt_hop=%d\n",
          (uint32_t)dp->src.s_addr, (uint32_t)dp->dst.s_addr, (uint32_t)dp->nxt_hop.s_addr);

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

    if (dp->dst.s_addr != DSR_BROADCAST)
    {
        /* Get hardware destination address */
        IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo*>(p->getControlInfo());
        IPv4Address nextIp((uint32_t)dp->nxt_hop.s_addr);
        controlInfo->setNextHopAddr(nextIp);
        p->setNextAddress(nextIp);
    }
    /*
    if (!ConfVal(UseNetworkLayerAck)) {
        cmh->xmit_failure_ = xmit_failure;
        cmh->xmit_failure_data_ = (void *) this;
    }
    */
    IPv4Address prev((uint32_t)myaddr_.s_addr);
    p->setPrevAddress(prev);
    if (jitter)
        sendDelayed(p, jitter, "to_ip");
    else if (dp->dst.s_addr != DSR_BROADCAST)
        sendDelayed(p, par("unicastDelay"), "to_ip");
    else
        sendDelayed(p, par("broadcastDelay"), "to_ip");
    dp->payload = NULL;
    dsr_pkt_free(dp);
}


void DSRUU::omnet_deliver(struct dsr_pkt *dp)
{
    int dsr_opts_len = 0;
    if (dp->dh.raw)
    {
        dsr_opts_len = dp->dh.opth->p_len + DSR_OPT_HDR_LEN;
        dsr_opt_remove(dp);
    }
    IPv4Datagram *dgram;
    dgram = new IPv4Datagram;

    IPv4Address destAddress_var((uint32_t)dp->dst.s_addr);
    dgram->setDestAddress(destAddress_var);
    IPv4Address srcAddress_var((uint32_t)dp->src.s_addr);
    dgram->setSrcAddress(srcAddress_var);
    dgram->setHeaderLength(dp->nh.iph->ihl-dsr_opts_len); // Header length
    dgram->setVersion(dp->nh.iph->version); // Ip version
    dgram->setTypeOfService(dp->nh.iph->tos); // ToS
    dgram->setIdentification(dp->nh.iph->id); // Identification
    dgram->setMoreFragments(dp->nh.iph->tos & 0x2000);
    dgram->setDontFragment(dp->nh.iph->frag_off & 0x4000);
#ifdef NEWFRAGMENT
    dgram->setTotalPayloadLength(dp->totalPayloadLength);
#endif
    dgram->setTimeToLive(dp->nh.iph->ttl); // TTL
    dgram->setTransportProtocol(dp->encapsulate_protocol); // Transport protocol

    if (dp->payload)
        dgram->encapsulate(dp->payload);
    dp->payload = NULL;
    dsr_pkt_free(dp);
    send(dgram, "to_ip");
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
    //current_time =simTime();
    if (!is_init)
    {

        for (int i = 0; i < CONFVAL_MAX; i++)
        {
            /* Override the default values in the ns-default.tcl file */
            confvals[i] = confvals_def[i].val;
//          sprintf(name, "%s_", confvals_def[i].name);
//          bind(name,  &confvals[i]);
        }

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


    if (stage==4)
    {
        /* Search the 80211 interface */
        inet_rt = RoutingTableAccess().get();
        inet_ift = InterfaceTableAccess().get();

        int  num_80211 = 0;
        InterfaceEntry *   ie;
        InterfaceEntry *   i_face;
        const char *name;
        for (int i = 0; i < inet_ift->getNumInterfaces(); i++)
        {
            ie = inet_ift->getInterface(i);
            name = ie->getName();
            if (strstr(name, "wlan")!=NULL)
            {
                i_face = ie;
                num_80211++;
                interfaceId = ie->getInterfaceId();
            }
        }
        // One enabled network interface (in total)
        if (num_80211==1)
            interface80211ptr = i_face;
        else
            opp_error("DSR has found %i 80211 interfaces", num_80211);


        /* Initilize tables */
        lc_init();
        neigh_tbl_init();
        rreq_tbl_init();
        grat_rrep_tbl_init();
        maint_buf_init();
        send_buf_init();
        path_cache_init();
        etxNumRetry = -1;
        etxActive = par("ETX_Active");
        if (etxActive)
        {
            etxTime = par("ETXHelloInterval");
            etxNumRetry = par("ETXRetryBeforeFail");
            etxWindowSize = etxTime*(unsigned int)par("ETXWindowNumHello");
            etxJitter = 0.1;
            etx_timer.init(&DSRUU::EtxMsgSend, 0);
            set_timer(&etx_timer, 0.0);
            //set_timer(&etx_timer, etxTime);
            etxWindow = 0;
            etxSize = 100; // Minimun length
        }
        myaddr_.s_addr = interface80211ptr->ipv4Data()->getIPAddress().getInt();
        macaddr_ = interface80211ptr->getMacAddress();
        nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_LINK_BREAK);
        if (get_confval(PromiscOperation))
            nb->subscribe(this, NF_LINK_PROMISCUOUS);
        // clear routing entries related to wlan interfaces and autoassign ip adresses
        bool manetPurgeRoutingTables = (bool) par("manetPurgeRoutingTables");
        if (manetPurgeRoutingTables)
        {
            IPv4Route *entry;
            // clean the route table wlan interface entry
            for (int i=inet_rt->getNumRoutes()-1; i>=0; i--)
            {
                entry = inet_rt->getRoute(i);
                const InterfaceEntry *ie = entry->getInterface();
                if (strstr(ie->getName(), "wlan")!=NULL)
                {
                    inet_rt->deleteRoute(entry);
                }
            }
        }
        interface80211ptr->ipv4Data()->joinMulticastGroup(IPv4Address::LL_MANET_ROUTERS);
        is_init = true;
        ev << "Dsr active" << "\n";
    }

    return;
}

void DSRUU::finish()
{
    lc_cleanup();
    path_cache_cleanup();
    neigh_tbl_cleanup();
    rreq_tbl_cleanup();
    grat_rrep_tbl_cleanup();
    send_buf_cleanup();
    maint_buf_cleanup();

}

DSRUU::DSRUU():cSimpleModule(), INotifiable()
{
    lifoDsrPkt = NULL;
    lifo_token = 0;
    grat_rrep_tbl_timer_ptr = new DSRUUTimer(this);
    send_buf_timer_ptr = new DSRUUTimer(this);
    neigh_tbl_timer_ptr = new DSRUUTimer(this);
    lc_timer_ptr = new DSRUUTimer(this);
    ack_timer_ptr = new DSRUUTimer(this);
    etx_timer_ptr = new DSRUUTimer(this);
    is_init = false;
}

DSRUU::~DSRUU()
{
    lc_cleanup();
    neigh_tbl_cleanup();
    rreq_tbl_cleanup();
    grat_rrep_tbl_cleanup();
    send_buf_cleanup();
    maint_buf_cleanup();
    path_cache_cleanup();
    struct dsr_pkt * pkt;
    pkt = lifoDsrPkt;
// delete ETX
    while (!etxNeighborTable.empty())
    {
        ETXNeighborTable::iterator i = etxNeighborTable.begin();
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
    while (pkt!=NULL)
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

void DSRUU::defaultProcess(cMessage *ipDgram)
{
    struct dsr_pkt *dp;
    dp = dsr_pkt_alloc(PK(ipDgram)); // crear estructura dsr
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


// Rutina HandleMessage ()
void DSRUU::handleMessage(cMessage* msg)
{
    if (is_init==false)
        opp_error("Dsr has not been initialized ");

    //current_time =simTime();
    if (msg->isSelfMessage())
    {// Timer msg
        handleTimer(msg);
        return;
    }
    /* Control Message decapsulate */
    if (dynamic_cast<ControlManetRouting *>(msg))
    {
        ControlManetRouting * control = check_and_cast <ControlManetRouting *> (msg);
        if (control->getOptionCode() == MANET_ROUTE_NOROUTE)
        {
            cMessage *msg_aux = control->decapsulate();
            EV << "Dsr rec msg " << msg_aux->getName() << "\n";
            delete msg;
            msg = msg_aux;
        }
        else
        {
            delete msg;
            return;
        }
    }
// Ext messge
    if (dynamic_cast<DSRPktExt*>(msg))
    {
        EtxMsgProc(msg);
        return;
    }

    IPv4Datagram * ipDgram = NULL;
    if (dynamic_cast<IPv4Datagram *>(msg))
    {
        ipDgram = dynamic_cast<IPv4Datagram *>(msg);
    }
    else
    {
        // recapsulate and send
        if (proccesICMP(msg))
            return;
        //EV << "############################################################\n";
        //EV << "!!WARNING: DSR has received not supported packet, delete it \n";
        //EV << "############################################################\n";
        //delete msg;
        //return;
        opp_error("DSR has received not supported packet");
    }
    DEBUG("##########\n");
    if (ipDgram->getSrcAddress().isUnspecified())
        ipDgram->setSrcAddress(interface80211ptr->ipv4Data()->getIPAddress());

    // Process a Dsr message
    defaultProcess(ipDgram);
    return;
}



void DSRUU::receiveChangeNotification(int category, const cObject *details)
{
    IPv4Datagram  *dgram = NULL;
    //current_time = simTime();

    if (details==NULL)
        return;

    if (category == NF_LINK_BREAK)
    {
        Enter_Method("Dsr Link Break");
        Ieee80211DataFrame *frame = check_and_cast<Ieee80211DataFrame *>(details);
        if (dynamic_cast<IPv4Datagram *>(frame->getEncapsulatedPacket()))
            dgram = check_and_cast<IPv4Datagram *>(frame->getEncapsulatedPacket());
        else
            return;

        if (!get_confval(UseNetworkLayerAck))
        {
            packetFailed(dgram);
        }
    }
    else if (category == NF_LINK_PROMISCUOUS)
    {
        if (get_confval(PromiscOperation))
        {
            Enter_Method("Dsr promisc");

            if (dynamic_cast<Ieee80211DataFrame *>(const_cast<cObject*>(details)))
            {
                Ieee80211DataFrame *frame = check_and_cast<Ieee80211DataFrame *>(details);
                if (dynamic_cast<DSRPkt *>(frame->getEncapsulatedPacket()))
                {

                    DSRPkt *paux = check_and_cast <DSRPkt *> (frame->getEncapsulatedPacket());

                   // DSRPkt *p = check_and_cast <DSRPkt *> (paux->dup());
                   // take(p);
                    EV << "####################################################\n";
                    EV << "Dsr protocol received promiscuous packet from " << paux->getSrcAddress() << "\n";
                    EV << "#####################################################\n";
                    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
                    ctrl->setSrc(frame->getTransmitterAddress());
                    ctrl->setDest(frame->getReceiverAddress());
                  //  p->setControlInfo(ctrl);
                    tap(paux,ctrl);
                }
            }
        }
    }
}

void DSRUU::packetFailed(IPv4Datagram *ipDgram)
{
    struct dsr_pkt *dp;
    struct in_addr dst, nxt_hop;
    struct in_addr prev_hop;
    /* Cast the packet so that we can touch it */
    /* Do nothing for my own packets... */
    if (ipDgram->getTransportProtocol()!=IP_PROT_DSR)
    {
        // This shouldn't really happen ?
        ev << "Data packet from "<< ipDgram->getSrcAddress() <<"without DSR header!n";
        return;
    }

    DSRPkt *p = NULL;

    if (dynamic_cast<DSRPkt *>(ipDgram))
    {
        p = check_and_cast <DSRPkt *> (ipDgram->dup());
        prev_hop.s_addr = p->prevAddress().getInt();
        dst.s_addr = p->getDestAddress().getInt();
        nxt_hop.s_addr = p->nextAddress().getInt();
        DEBUG("Xmit failure for %s nxt_hop=%s\n", print_ip(dst), print_ip(nxt_hop));

        if (ConfVal(PathCache))
            ph_srt_delete_link(my_addr(), nxt_hop);
        else
            lc_link_del(my_addr(), nxt_hop);

        dp = dsr_pkt_alloc(p);
        if (!dp)
        {
            DEBUG("Could not allocate DSR packet\n");
            drop(p, ICMP_DESTINATION_UNREACHABLE);
        }
        else
        {
            dsr_rerr_send(dp, nxt_hop);
            dp->nxt_hop = nxt_hop;
            maint_buf_salvage(dp);
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

}


void DSRUU::linkFailed(IPv4Address ipAdd)
{

    struct in_addr nxt_hop;

    /* Cast the packet so that we can touch it */
    nxt_hop.s_addr = ipAdd.getInt();
    if (ConfVal(PathCache))
        ph_srt_delete_link(my_addr(), nxt_hop);
    else
        lc_link_del(my_addr(), nxt_hop);

    return;

}


void DSRUU::tap(DSRPkt *p, cObject *ctrl)
{
    struct dsr_pkt *dp;
    //struct in_addr next_hop, prev_hop;
    //next_hop.s_addr = p->nextAddress().getInt();
    //prev_hop.s_addr = p->prevAddress().getInt();
    int transportProtocol = p->getTransportProtocol();
    /* Cast the packet so that we can touch it */
    dp = dsr_pkt_alloc2(p, ctrl);
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
        return ph_srt_find(src, dst, 0, ConfValToUsecs(RouteCacheTimeout));
    else
        return lc_srt_find(src, dst);

}

int DSRUU::RouteAdd(struct dsr_srt *srt, unsigned long timeout, unsigned short flags)
{


    if (ConfVal(PathCache))
    {
        ph_srt_add(srt, timeout, flags);
        return 0;
    }
    else
        return lc_srt_add(srt, timeout, flags);
}

void DSRUU::EtxMsgSend(unsigned long data)
{
    EtxList neigh[15];
    DSRPktExt* msg = new DSRPktExt();
    IPv4Address destAddress_var(DSR_BROADCAST);
    msg->setDestAddress(destAddress_var);
    IPv4Address srcAddress_var((uint32_t)myaddr_.s_addr);
    msg->setSrcAddress(srcAddress_var);
    msg->setTimeToLive(1); // TTL
    msg->setTransportProtocol(IP_PROT_DSR); // Transport protocol
    IPv4ControlInfo *ipControlInfo = new IPv4ControlInfo();
    ipControlInfo->setProtocol(IP_PROT_DSR);
    ipControlInfo->setInterfaceId(interfaceId); // If broadcast packet send to interface
    ipControlInfo->setSrcAddr(srcAddress_var);
    ipControlInfo->setDestAddr(destAddress_var);
    ipControlInfo->setTimeToLive(1);
    msg->setControlInfo(ipControlInfo);

    int numNeighbor = 0;
    for (ETXNeighborTable::iterator iter = etxNeighborTable.begin(); iter!=etxNeighborTable.end();)
    {
        // remove old data
        ETXEntry *entry = (*iter).second;
        while (simTime()-entry->timeVector.front()>etxWindowSize)
            entry->timeVector.erase(entry->timeVector.begin());
        if (entry->timeVector.size()==0)
        {
            linkFailed((*iter).first);
            etxNeighborTable.erase(iter);
            iter = etxNeighborTable.begin();
            continue;
        }

        double delivery;
        delivery = entry->timeVector.size()/(etxWindowSize/etxTime);
        if (delivery>0.99)
            delivery = 1;
        entry->deliveryDirect = delivery;

        if (numNeighbor<15)
        {
            neigh[numNeighbor].address = iter->first;
            neigh[numNeighbor].delivery = delivery; //(uint32_t)(delivery*0xFFFF); // scale
            numNeighbor++;
            if (neigh[numNeighbor-1].delivery <= 0)
                printf("\n recojones");
        }
        else
        {
            // delete
            int aux = 0;
            for (int i=1; i<15; i++)
            {
                if (neigh[i].delivery<neigh[aux].delivery)
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

    EtxList *list = msg->addExtension(numNeighbor);
    memcpy(list, neigh, sizeof(EtxList)*numNeighbor);

    if (msg->getByteLength()<etxSize)
        msg->setByteLength(etxSize);

    sendDelayed(msg,uniform(0,etxJitter), "to_ip");

    etxWindow += etxTime;
    set_timer(&etx_timer, etxTime+ SIMTIME_DBL(simTime()));
}

void DSRUU::EtxMsgProc(cMessage *m)
{
    DSRPktExt *msg;
    int pos = -1;
    msg = dynamic_cast<DSRPktExt*>(m);
    EtxList *list = msg->getExtension();
    int size = msg->getSizeExtension();

    IPv4Address myAddress((uint32_t)myaddr_.s_addr);
    IPv4Address srcAddress(msg->getSrcAddress());


    for (int i = 0; i<size; i++)
    {
        if (list[i].address==myAddress)
        {
            pos = i;
            break;
        }
    }
    ETXNeighborTable::iterator it = etxNeighborTable.find(srcAddress);
    ETXEntry *entry = NULL;
    if (it==etxNeighborTable.end())
    {
        // add
        entry = new ETXEntry();
        //entry->address=msg->getSrcAddress();
        etxNeighborTable.insert(std::make_pair(srcAddress, entry));

    }
    else
    {
        entry = (*it).second;
        while (simTime()-entry->timeVector.front()>etxWindowSize)
            entry->timeVector.erase(entry->timeVector.begin());
    }
    double delivery;
    entry->timeVector.push_back(simTime());
    delivery = entry->timeVector.size()/(etxWindowSize/etxTime);
    if (delivery>0.99)
        delivery = 1;
    entry->deliveryDirect = delivery;
    if (pos!=-1)
    {
        //unsigned int cost = (unsigned int) list[pos].delivery;
        //entry->deliveryReverse= ((double)cost)/0xFFFF;
        entry->deliveryReverse = list[pos].delivery;
        if (entry->deliveryDirect<=0)
            printf("Cojones");
    }
    delete m;
}

double DSRUU::getCost(IPv4Address add)
{
    ETXNeighborTable::iterator it = etxNeighborTable.find(add);
    if (it==etxNeighborTable.end())
        return -1;
    ETXEntry *entry = (*it).second;
    if (entry->deliveryReverse==0 || entry->deliveryDirect==0)
        return -1;
    return (1/(entry->deliveryReverse * entry->deliveryDirect));
}

void DSRUU::ExpandCost(struct dsr_pkt *dp)
{
    EtxCost  * costVector;
    struct in_addr myAddr;

    if (!etxActive)
    {
        if (dp->costVectorSize>0)
            delete [] dp->costVector;

        dp->costVector = NULL;
        dp->costVectorSize = 0;
        return;
    }
    myAddr = my_addr();
    IPv4Address myAddress((uint32_t)myAddr.s_addr);
    if (dp->costVectorSize==0 && dp->src.s_addr!=myAddr.s_addr)
    {
        dp->costVectorSize++;
        dp->costVector = new EtxCost[1];
        dp->costVector[0].address = myAddress;
        double cost = getCost(IPv4Address((uint32_t)dp->src.s_addr));

        if (cost<0)
            dp->costVector[0].cost = 1e100;
        else
            dp->costVector[0].cost = cost;
    }
    else if (dp->costVectorSize>0)
    {
        costVector = new EtxCost[dp->costVectorSize+1];
        memcpy(costVector, dp->costVector, dp->costVectorSize*sizeof(EtxCost));
        costVector[dp->costVectorSize].address = myAddress;
        double cost = getCost(dp->costVector[dp->costVectorSize-1].address);
        if (cost<0)
            costVector[dp->costVectorSize].cost = 1e100;
        else
            costVector[dp->costVectorSize].cost = cost;

        delete [] dp->costVector;
        dp->costVectorSize++;
        dp->costVector = costVector;
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
        totalCost = (double)(DSR_RREQ_ADDRS_LEN(dp->rreq_opt)/sizeof(struct in_addr));
        totalCost += 1;
        return totalCost;
    }
    totalCost = 0;
    for (int i=0; i<dp->costVectorSize; i++)
    {
        totalCost += dp->costVector[i].cost;
    }
    double cost;
    if (dp->costVectorSize>0)
        cost = getCost(dp->costVector[dp->costVectorSize-1].address);
    else
    {
        cost = getCost(IPv4Address(dp->src.s_addr));
    }

    if (cost<0)
        cost = 1e100;
    totalCost += cost;
    return totalCost;
}


void DSRUU::AddCost(struct dsr_pkt *dp, struct dsr_srt *srt)
{
    struct in_addr add;

    if (dp->costVectorSize>0)
        delete [] dp->costVector;
    dp->costVector = NULL;
    dp->costVectorSize = 0;
    if (!etxActive)
        return;

    int sizeAddress = srt->laddrs/ sizeof(struct in_addr);
    dp->costVector = new EtxCost[srt->cost_size];
    dp->costVectorSize = srt->cost_size;
    for (int i=0; i<sizeAddress; i++)
    {
        add = srt->addrs[i];
        dp->costVector[i].address = IPv4Address((uint32_t)add.s_addr);
        dp->costVector[i].cost = srt->cost[i];
    }
    dp->costVector[srt->cost_size-1].address = IPv4Address((uint32_t)srt->dst.s_addr);
    dp->costVector[srt->cost_size-1].cost = srt->cost[srt->cost_size-1];
}


bool DSRUU::proccesICMP(cMessage *msg)
{
    ICMPMessage * pk = dynamic_cast<ICMPMessage *>(msg);
    if (pk==NULL)
        return false;
    // check if
    // recapsulate and send
    if (pk->getControlInfo())
        delete pk->removeControlInfo();
    DSRPkt *bogusPacket = dynamic_cast<DSRPkt *>(pk->getEncapsulatedPacket());
    if (bogusPacket==NULL)
    {
        delete msg;
        return true;
    }
    // check if is a exclusive DSR packet
    if (bogusPacket->getEncapProtocol()==0)
    {
        // delete all and return
        delete msg;
        return true;
    }
    if (pk->getControlInfo())
        delete pk->removeControlInfo();
    IPv4Datagram *newdgram = new IPv4Datagram();
    bogusPacket->setTransportProtocol(bogusPacket->getEncapProtocol());
    IPv4Address dst(this->my_addr().s_addr);
    newdgram->setDestAddress(dst);
    newdgram->encapsulate(bogusPacket);
    newdgram->setTransportProtocol(IP_PROT_ICMP);
    send(newdgram,"to_ip");
    return true;
 }

