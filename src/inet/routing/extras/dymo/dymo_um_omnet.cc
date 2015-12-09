/*****************************************************************************
 *
 * Copyright (C) 2007 Malaga university
 *
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
 * Authors: Alfonso Ariza Quintana
 *
 *****************************************************************************/


// #define RERRPACKETFAILED

#include <string.h>
#include <assert.h>
#include "inet/routing/extras/dymo/dymo_um_omnet.h"

#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/ICMPMessage_m.h"
#include "inet/common/NotifierConsts.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"


#include "inet/common/ProtocolMap.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/extras/base/ControlManetRouting_m.h"
#ifdef WITH_80211MESH
#include "LocatorNotificationInfo_m.h"
#endif

namespace inet {

using namespace ieee80211;

namespace inetmanet {

typedef std::vector<IPv4Address> IPAddressVector;

Define_Module(DYMOUM);

/* Constructor for the DYMOUM routing agent */

bool DYMOUM::log_file_fd_init = false;
int DYMOUM::log_file_fd = -1;
#ifdef DYMO_UM_GLOBAL_STATISTISTIC
bool DYMOUM::iswrite = false;
int DYMOUM::totalSend = 0;
int DYMOUM::totalRreqSend = 0;
int DYMOUM::totalRreqRec = 0;
int DYMOUM::totalRrepSend = 0;
int DYMOUM::totalRrepRec = 0;
int DYMOUM::totalRrepAckSend = 0;
int DYMOUM::totalRrepAckRec = 0;
int DYMOUM::totalRerrSend = 0;
int DYMOUM::totalRerrRec = 0;
#endif


std::map<L3Address,u_int32_t *> DYMOUM::mapSeqNum;


void DYMOUM::initialize(int stage)
{
    ManetRoutingBase::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {

#ifndef DYMO_UM_GLOBAL_STATISTISTIC
        iswrite = false;
        totalSend = 0;
        totalRreqSend = 0;
        totalRreqRec = 0;
        totalRrepSend = 0;
        totalRrepRec = 0;
        totalRrepAckSend = 0;
        totalRrepAckRec = 0;
        totalRerrSend = 0;
        totalRerrRec = 0;
#endif

        macToIpAdress = new MacToIpAddress;
        sendMessageEvent = new cMessage();

        //sendMessageEvent = new cMessage();
        PromiscOperation = true;

        /* From main.c */
        progname = strdup("Dymo-UM");

        /* From debug.c */
        /* Note: log_nmsgs was never used anywhere */

        gateWayAddress = new IPv4Address("0.0.0.0");
        /* Set host parameters */
        memset(&this_host, 0, sizeof(struct host_info));
        memset(dev_indices, 0, sizeof(dev_indices));
        this_host.seqnum = 1;
        this_host.nif = 1;
        this_host.prefix = 0;
        this_host.is_gw = 0;

        /* Search the 80211 interface */

        registerRoutingModule();

        NS_DEV_NR = getWlanInterfaceIndexByAddress();
        NS_IFINDEX = getWlanInterfaceIndexByAddress();

        for (int i = 0; i < DYMO_MAX_NR_INTERFACES; i++)
        {
            DEV_NR(i).enabled = 0;
        }

        numInterfacesActive = 0;
        for (int i = 0; i < getNumInterfaces(); i++)
        {
            DEV_NR(i).ifindex = i;
            dev_indices[i] = i;
            strcpy(DEV_NR(i).ifname, getInterfaceEntry(i)->getName());
            if (isInMacLayer())
                DEV_NR(i).ipaddr.s_addr = L3Address(getInterfaceEntry(i)->getMacAddress());
            else
                DEV_NR(i).ipaddr.s_addr = L3Address(getInterfaceEntry(i)->ipv4Data()->getIPAddress());
            if (getInterfaceEntry(i)->isLoopback())
                continue;
            if (isInMacLayer())
            {
                mapSeqNum[DEV_NR(i).ipaddr.s_addr] = &this_host.seqnum;
            }
        }
        /* Set network interface parameters */
        for (int i=0; i < getNumWlanInterfaces(); i++)
        {
            DEV_NR(getWlanInterfaceIndex(i)).enabled = 1;
            DEV_NR(getWlanInterfaceIndex(i)).sock = -1;
            DEV_NR(getWlanInterfaceIndex(i)).bcast.s_addr = L3Address(IPv4Address(DYMO_BROADCAST));
            numInterfacesActive++;
        }

        no_path_acc = 0;
        reissue_rreq = 0;
        s_bit = 0;

        if ((bool)par("no_path_acc_"))
            no_path_acc = 1;
        if ((bool)par("reissue_rreq_"))
            reissue_rreq = 1;
        if ((bool)par("s_bit_"))
            s_bit = 1;
        hello_ival = par("hello_ival_");

        intermediateRREP = par("intermediateRREP");

        if ((DYMO_RATELIMIT = (int) par("MaxPktSec"))==-1)
            DYMO_RATELIMIT = 10;
        if (DYMO_RATELIMIT>50)
            DYMO_RATELIMIT = 50;
        if ((NET_DIAMETER = (int) par("NetDiameter"))==-1)
            NET_DIAMETER = 10;
        if ((ROUTE_TIMEOUT = (long) par("RouteTimeOut"))==-1)
            ROUTE_TIMEOUT = 5000;
        if ((ROUTE_DELETE_TIMEOUT = (long) par("RouteDeleteTimeOut"))==-1)
            ROUTE_DELETE_TIMEOUT = 2*ROUTE_TIMEOUT;

        if ((RREQ_WAIT_TIME = (long) par("RREQWaitTime"))==-1)
            RREQ_WAIT_TIME = 2000;
        if ((RREQ_TRIES = (int) par("RREQTries"))==-1)
            RREQ_TRIES = 3;

        if (interface80211ptr->ipv4Data())
            ipNodeId = new IPv4Address(interface80211ptr->ipv4Data()->getIPAddress());

        rtable_init();

        if (hello_ival<=0)
            linkLayerFeeback();
        if ((bool) par("promiscuous"))
        {
            linkPromiscuous();
        }

        attachPacket = (bool) par("RREQattachPacket");
        if (isInMacLayer())
            attachPacket = false;

        norouteBehaviour = par("noRouteBehaviour");
        useIndex = par("UseIndex");
        isRoot = par("isRoot");
        proactive_rreq_timeout= par("proactiveRreqTimeout").longValue();
        if (isRoot)
        {
            timer_init(&proactive_rreq_timer, &DYMOUM::rreq_proactive,nullptr);
            timer_set_timeout(&proactive_rreq_timer, proactive_rreq_timeout);
            timer_add(&proactive_rreq_timer);
        }

        path_acc_proactive = par("path_acc_proactive");
        propagateProactive = par("propagateProactive");

        strcpy(nodeName, getContainingNode(this)->getFullName());
        dymo_socket_init();
        startDYMOUMAgent();
        is_init = true;
        // Initialize the timer
        scheduleNextEvent();
        costStatic = par("costStatic").longValue();
        costMobile = par("costMobile").longValue();
        useHover = par("useHover");
        EV_INFO << "Dymo active" << "\n";

    }
}

DYMOUM::DYMOUM()
{
    attachPacket = false;
    is_init = false;
    log_file_fd_init = false;
    ipNodeId = nullptr;
    gateWayAddress = nullptr;
    numInterfacesActive = 0;
    timer_elem = 0;
    sendMessageEvent = nullptr; /*&messageEvent;*/
    macToIpAdress = nullptr;
    mapSeqNum.clear();
    isRoot = false;
    this->setStaticNode(true);
#ifdef MAPROUTINGTABLE
    dymoRoutingTable = new DymoRoutingTable;
    dymoPendingRreq = new DymoPendingRreq;
    dymoNbList = new DymoNbList;
    dymoBlackList = new DymoBlackList;
#else
    INIT_DLIST_HEAD(&TQ);
    INIT_DLIST_HEAD(&PENDING_RREQ);
    INIT_DLIST_HEAD(&BLACKLIST);
    INIT_DLIST_HEAD(&NBLIST);
#endif
#ifdef TIMERMAPLIST
    dymoTimerList = new DymoTimerMap;
#endif
    rtable_init();
    packet_queue_init();
}

/* Destructor for the AODV-UU routing agent */
DYMOUM::~ DYMOUM()
{
// Clean all internal tables
    packet_queue_destroy();
    if (macToIpAdress)
        delete macToIpAdress;
// Routing table
#ifndef MAPROUTINGTABLE
    dlist_head_t *pos, *tmp;
    pos = tmp = nullptr;
    pos = tmp = nullptr;
    dlist_for_each_safe(pos, tmp, &rtable.l)
    {
        rtable_entry_t *e = (rtable_entry_t *) pos;
        timer_remove(&e->rt_deltimer);
        timer_remove(&e->rt_validtimer);
        dlist_del(&e->l);
        free(e);
    }
    pos = tmp = nullptr;
    // RREQ table
    dlist_for_each_safe(pos, tmp, &PENDING_RREQ)
    {
        dlist_del(pos);
        free(pos);
    }
    pos = tmp = nullptr;
    // black list table
    dlist_for_each_safe(pos, tmp, &BLACKLIST)
    {
        dlist_del(pos);
        free(pos);
    }
    pos = tmp = nullptr;
    // neigbourd list table
    dlist_for_each_safe(pos, tmp, &NBLIST)
    {
        dlist_del(pos);
        free(pos);
    }
#else
    rtable_destroy();
    while (!dymoPendingRreq->empty())
    {
        timer_remove(&dymoPendingRreq->begin()->second->timer);
        delete dymoPendingRreq->begin()->second;
        dymoPendingRreq->erase(dymoPendingRreq->begin());
    }
    while (!dymoBlackList->empty())
    {
        timer_remove(&dymoBlackList->begin()->second->timer);
        delete dymoBlackList->begin()->second;
        dymoBlackList->erase(dymoBlackList->begin());
    }
    while (!dymoNbList->empty())
    {
        timer_remove(&dymoNbList->back()->timer);
        delete dymoNbList->back();
        dymoNbList->pop_back();
    }
    delete dymoRoutingTable;
    delete dymoPendingRreq;
    delete dymoNbList;
    delete dymoBlackList;
#endif

    cancelAndDelete(sendMessageEvent);
    //log_cleanup();
    if (gateWayAddress)
        delete gateWayAddress;
    if (ipNodeId)
        delete ipNodeId;
    free(progname);


#ifdef TIMERMAPLIST
    delete dymoTimerList;
#endif
}

/*
  Moves pending packets with a certain next hop from the interface
  queue to the packet buffer or simply drops it.
*/


/* Entry-level packet reception */
void DYMOUM::handleMessage(cMessage *msg)
{
    DYMO_element *dymoMsg = nullptr;
    IPv4Datagram * ipDgram = nullptr;
    UDPPacket * udpPacket = nullptr;
    cMessage *msg_aux;
    struct in_addr src_addr;
    struct in_addr dest_addr;

    if (is_init==false)
        throw cRuntimeError("Dymo-UM has not been initialized ");
    if (msg==sendMessageEvent)
    {
        // timer event
        scheduleNextEvent();
        return;
    }
    /* Handle packet depending on type */


    if (dynamic_cast<ControlManetRouting *>(msg))
    {
        ControlManetRouting * control = check_and_cast <ControlManetRouting *> (msg);
        if (control->getOptionCode() == MANET_ROUTE_NOROUTE)
        {
            if (isInMacLayer())
            {
                if (control->getDestAddress().toMAC().isBroadcast())
                {
                    delete control;
                    return;
                }
                cMessage* msgAux = control->decapsulate();

                if (msgAux)
                    processMacPacket(PK(msgAux), control->getDestAddress(), control->getSrcAddress(), NS_IFINDEX);
                else
                {
                    if (!addressIsForUs(control->getSrcAddress()))
                    {
                        struct in_addr dest_addr;
                        dest_addr.s_addr = control->getDestAddress();
                        rtable_entry_t *entry = rtable_find(dest_addr);
                        rerr_send(dest_addr, NET_DIAMETER, entry);
                    }
                }
            }
            else
            {
                ipDgram = (IPv4Datagram*) control->decapsulate();
                EV_INFO << "Dymo rec datagram  " << ipDgram->getName() << " with dest=" << ipDgram->getDestAddress().str() << "\n";
                processPacket(ipDgram, NS_IFINDEX);  /// Always use ns interface
            }
        }
        else if (control->getOptionCode() == MANET_ROUTE_UPDATE)
        {
            src_addr.s_addr = control->getSrcAddress();
            dest_addr.s_addr = control->getDestAddress();
            rtable_entry_t *src_entry = rtable_find(src_addr);
            rtable_entry_t *dest_entry = rtable_find(dest_addr);
            rtable_update_timeout(src_entry);
            rtable_update_timeout(dest_entry);
        }
        delete msg;
        scheduleNextEvent();
        return;
    }
    else if (dynamic_cast<UDPPacket *>(msg) || dynamic_cast<DYMO_element  *>(msg))
    {

        udpPacket = nullptr;

        if (!isInMacLayer())
        {
            udpPacket = check_and_cast<UDPPacket*>(msg);
            if (udpPacket->getDestinationPort() != DYMO_PORT)
            {
                delete  msg;
                scheduleNextEvent();
                return;
            }
            msg_aux = udpPacket->decapsulate();
        }
        else
            msg_aux = msg;

        if (dynamic_cast<DYMO_element  *>(msg_aux))
        {
            dymoMsg = check_and_cast  <DYMO_element *>(msg_aux);
            if (!isInMacLayer())
            {
                INetworkProtocolControlInfo *controlInfo = check_and_cast<INetworkProtocolControlInfo*>(udpPacket->removeControlInfo());
                src_addr.s_addr = controlInfo->getSourceAddress();
                dymoMsg->setControlInfo(check_and_cast<cObject *>(controlInfo));
            }
            else
            {
                Ieee802Ctrl *controlInfo = check_and_cast<Ieee802Ctrl*>(dymoMsg->getControlInfo());
                src_addr.s_addr = L3Address(controlInfo->getSrc());
                EV_INFO << "rec packet from " << controlInfo->getSrc() <<endl;
            }
        }
        else
        {
            if (udpPacket)
                delete udpPacket;
            delete msg_aux;
            scheduleNextEvent();
            return;

        }

        if (udpPacket)
            delete udpPacket;
    }
    else
    {
        delete msg;
        scheduleNextEvent();
        return;
    }
    /* Detect routing loops */
    if (isLocalAddress(src_addr.s_addr))
    {
        delete dymoMsg;
        dymoMsg = nullptr;
        scheduleNextEvent();
        return;
    }

    recvDYMOUMPacket(dymoMsg);
    scheduleNextEvent();
}


/* Starts the Dymo routing agent */
int DYMOUM::startDYMOUMAgent()
{

    /* Set up the wait-on-reboot timer */
    /*
        if (wait_on_reboot) {
            timer_init(&worb_timer, &NS_CLASS wait_on_reboot_timeout, &wait_on_reboot);
            timer_set_timeout(&worb_timer, DELETE_PERIOD);
            DEBUG(LOG_NOTICE, 0, "In wait on reboot for %d milliseconds.",DELETE_PERIOD);
        }
    */
    /* Schedule the first HELLO */
    //if (!llfeedback && !optimized_hellos)
    hello_init();

    /* Initialize routing table logging */

    /* Initialization complete */
    initialized = 1;
    /*
        DEBUG(LOG_DEBUG, 0, "Routing agent with IP = %s : %d started.",
              ip_to_str(DEV_NR(NS_DEV_NR).ipaddr), DEV_NR(NS_DEV_NR).ipaddr);

        DEBUG(LOG_DEBUG, 0, "Settings:");
        DEBUG(LOG_DEBUG, 0, "unidir_hack %s", unidir_hack ? "ON" : "OFF");
        DEBUG(LOG_DEBUG, 0, "rreq_gratuitous %s", rreq_gratuitous ? "ON" : "OFF");
        DEBUG(LOG_DEBUG, 0, "expanding_ring_search %s", expanding_ring_search ? "ON" : "OFF");
        DEBUG(LOG_DEBUG, 0, "local_repair %s", local_repair ? "ON" : "OFF");
        DEBUG(LOG_DEBUG, 0, "receive_n_hellos %s", receive_n_hellos ? "ON" : "OFF");
        DEBUG(LOG_DEBUG, 0, "hello_jittering %s", hello_jittering ? "ON" : "OFF");
        DEBUG(LOG_DEBUG, 0, "wait_on_reboot %s", wait_on_reboot ? "ON" : "OFF");
        DEBUG(LOG_DEBUG, 0, "optimized_hellos %s", optimized_hellos ? "ON" : "OFF");
        DEBUG(LOG_DEBUG, 0, "ratelimit %s", ratelimit ? "ON" : "OFF");
        DEBUG(LOG_DEBUG, 0, "llfeedback %s", llfeedback ? "ON" : "OFF");
        DEBUG(LOG_DEBUG, 0, "internet_gw_mode %s", internet_gw_mode ? "ON" : "OFF");
        DEBUG(LOG_DEBUG, 0, "ACTIVE_ROUTE_TIMEOUT=%d", ACTIVE_ROUTE_TIMEOUT);
        DEBUG(LOG_DEBUG, 0, "TTL_START=%d", TTL_START);
        DEBUG(LOG_DEBUG, 0, "DELETE_PERIOD=%d", DELETE_PERIOD);
    */
    /* Schedule the first timeout */
    scheduleNextEvent();
    return 0;

}



// for use with gateway in the future
IPv4Datagram * DYMOUM::pkt_encapsulate(IPv4Datagram *p, IPv4Address gateway)
{
    IPv4Datagram *datagram = new IPv4Datagram(p->getName());
    datagram->setByteLength(IP_HEADER_BYTES);
    datagram->encapsulate(p);

    // set source and destination address
    datagram->setDestAddress(gateway);

    IPv4Address src = p->getSrcAddress();

    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    // set other fields
    datagram->setTypeOfService(p->getTypeOfService());
    datagram->setIdentification(p->getIdentification());
    datagram->setMoreFragments(false);
    datagram->setDontFragment(p->getDontFragment());
    datagram->setFragmentOffset(0);
    datagram->setTimeToLive(
        p->getTimeToLive() > 0 ?
        p->getTimeToLive() :
        0);

    datagram->setTransportProtocol(IP_PROT_IP);
    return datagram;
}



IPv4Datagram *DYMOUM::pkt_decapsulate(IPv4Datagram *p)
{

    if (p->getTransportProtocol() == IP_PROT_IP)
    {
        IPv4Datagram *datagram = check_and_cast  <IPv4Datagram *>(p->decapsulate());
        datagram->setTimeToLive(p->getTimeToLive());
        delete p;
        return datagram;
    }
    return nullptr;
}



/*
  Reschedules the timer queue timer to go off at the time of the
  earliest event (so that the timer queue will be investigated then).
  Should be called whenever something might have changed the timer queue.
*/
void DYMOUM::scheduleNextEvent()
{
    struct timeval *timeout;
    double delay;
    simtime_t timer;
    timeout = timer_age_queue();
    if (timeout)
    {
        delay = (double)(((double)timeout->tv_usec/(double)1000000.0) +(double)timeout->tv_sec);
        timer = simTime()+delay;
        if (sendMessageEvent->isScheduled())
        {
            if (timer < sendMessageEvent->getArrivalTime())
            {
                cancelEvent(sendMessageEvent);
                scheduleAt(timer, sendMessageEvent);
            }
        }
        else
        {
            scheduleAt(timer, sendMessageEvent);
        }
    }
}

/*
  Replacement for if_indextoname(), used in routing table logging.
*/
const char *DYMOUM::if_indextoname(int ifindex, char *ifname)
{
    InterfaceEntry *   ie;
    assert(ifindex >= 0);
    ie = getInterfaceEntry(ifindex);
    return ie->getName();
}


void DYMOUM::storeMacAddressIpAddressPairOf(INetworkDatagram *dgram) const
{
    if (dgram)
    {
        Ieee802Ctrl *ctrl = check_and_cast_nullable<Ieee802Ctrl *>(check_and_cast<cPacket *>(dgram)->getControlInfo());

        if (ctrl != nullptr)
        {
            MACAddress macAddressConv = ctrl->getSrc();
            auto it = macToIpAdress->find(macAddressConv);
            if (it == macToIpAdress->end())
            {
                L3Address ip_src = dgram->getSourceAddress();
                macToIpAdress->insert(std::make_pair(macAddressConv, ip_src));
            }
        }
    }
}

void DYMOUM::recvDYMOUMPacket(cMessage * msg)
{
    struct in_addr src, dst;
    int interfaceId = -1;

    DYMO_element  *dymo_msg = check_and_cast<DYMO_element *> (msg);
    int ifIndex = NS_IFINDEX;

    if (!isInMacLayer())
    {
        IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo *>(msg->removeControlInfo());
        src.s_addr = ctrl->getSourceAddress();
        dst.s_addr = ctrl->getDestinationAddress();
        interfaceId = ctrl->getInterfaceId();
        INetworkDatagram *dgram = ctrl->removeOrigDatagram();
        storeMacAddressIpAddressPairOf(dgram);
        delete dgram;
        delete ctrl;
    }
    else
    {
        Dymo_RE *dymoRe = dynamic_cast<Dymo_RE *> (dymo_msg);

        if (dymoRe && dymoRe->a)
        {
            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
            src.s_addr = L3Address(ctrl->getSrc());
            dst.s_addr = L3Address(ctrl->getDest());
        }
        else
        {
            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
            src.s_addr = L3Address(ctrl->getSrc());
            dst.s_addr = L3Address(ctrl->getDest());
            delete ctrl;
        }
    }

    InterfaceEntry *   ie;
    if (isLocalAddress(src.s_addr))
    {
        delete   dymo_msg;
        return;
    }

    if (!isInMacLayer())
    {
        for (int i = 0; i < getNumWlanInterfaces(); i++)
        {
            ie = getWlanInterfaceEntry(i);
            if (interfaceId == ie->getInterfaceId())
            {
                ifIndex = getWlanInterfaceIndex(i);
                break;
            }
        }
    }
    recv_dymoum_pkt(dymo_msg, src, ifIndex);
}


void DYMOUM::processPacket(IPv4Datagram * p, unsigned int ifindex )
{
    struct in_addr dest_addr, src_addr;
    bool isLocal = false;
    IPAddressVector phops;

    src_addr.s_addr = L3Address(p->getSrcAddress());
    dest_addr.s_addr = L3Address(p->getDestAddress());
    isLocal = true;
    if (!p->getSrcAddress().isUnspecified())
    {
        isLocal = isLocalAddress(src_addr.s_addr);

    }
    InterfaceEntry *   ie = getInterfaceEntry(ifindex);
    bool isMcast = ie->ipv4Data()->isMemberOfMulticastGroup(dest_addr.s_addr.toIPv4());

    /* If the packet is not interesting we just let it go through... */
    if (dest_addr.s_addr == L3Address(IPv4Address(DYMO_BROADCAST)) || isMcast)
    {
        if (p->getControlInfo())
            delete p->removeControlInfo();
        send(p, "to_ip");
        return;
    }
    rtable_entry_t *entry = rtable_find(dest_addr);
    if (!entry || entry->rt_state == RT_INVALID)
    {
        // If I am the originating node, then a route discovery
        // must be performed
        if (isLocal)
        {
            if (p->getControlInfo())
                delete p->removeControlInfo();
            packet_queue_add(p, dest_addr);
            route_discovery(dest_addr);
        }
        // Else we must send a RERR message to the source if
        // the route has been previously used
        else
        {
            struct in_addr addr;
            switch (norouteBehaviour)
            {
            case 3:
                if (p->getControlInfo())
                    delete p->removeControlInfo();
                packet_queue_add(p, dest_addr);
                route_discovery(dest_addr);
                break;
            case 2:
                // if (entry && entry->rt_is_used)
                // mac_address macAddressConv;
                cObject * ctrl;
                ctrl = p->removeControlInfo();
                if (ctrl!=nullptr)
                {
                    Ieee802Ctrl * ctrlmac = check_and_cast<Ieee802Ctrl *> (ctrl);
                    if (ctrlmac)
                    {
                        MACAddress macAddressConv = ctrlmac->getSrc(); /* destination eth addr */
                        // ctrlmac->getSrc().getAddressBytes(macAddressConv.address);  /* destination eth addr */
                        // memcpy (&dest,ctrlmac->getDest().getAddressBytes(),6);   /* destination eth addr */
                        delete ctrl;
                        auto it = macToIpAdress->find(macAddressConv);
                        if (it!=macToIpAdress->end())
                        {
                            addr.s_addr = it->second;
                            rerr_send(dest_addr, 1, entry, addr);
                        }
                    }
                }
                delete p;
                break;
            case 1:
                rerr_send(dest_addr, NET_DIAMETER, entry);
            default:
                //  icmpAccess.get()->sendErrorMessage(p, ICMP_DESTINATION_UNREACHABLE, 0);
                sendICMP(p);
                // delete p;
                break;
            }
        }
        scheduleNextEvent();
        return;
    }
    else
    {
        /* DEBUG(LOG_DEBUG, 0, "Sending pkt uid=%d", ch->uid()); */
        if (p->getControlInfo())
            delete p->removeControlInfo();
        send(p, "to_ip");
        /* When forwarding data, make sure we are sending HELLO messages */
        //gettimeofday(&this_host.fwd_time, nullptr);
        hello_init();
    }
}


void DYMOUM::processMacPacket(cPacket * p, const L3Address &dest, const L3Address &src, int ifindex)
{
    struct in_addr dest_addr, src_addr;
    bool isLocal = false;
    dest_addr.s_addr = dest;
    src_addr.s_addr = src;

    //InterfaceEntry *   ie = getInterfaceEntry(ifindex);
    isLocal = isLocalAddress(src);
    rtable_entry_t *entry = rtable_find(dest_addr);
    if (!entry || entry->rt_state == RT_INVALID)
    {
        // If I am the originating node, then a route discovery
        // must be performed
        if (isLocal)
        {
            if (p->getControlInfo())
                delete p->removeControlInfo();
            packet_queue_add(p, dest_addr);
            route_discovery(dest_addr);
        }
        // Else we must send a RERR message to the source if
        // the route has been previously used
        else
        {
            struct in_addr addr;
            switch (norouteBehaviour)
            {
            case 3:
                if (p->getControlInfo())
                    delete p->removeControlInfo();
                packet_queue_add(p, dest_addr);
                route_discovery(dest_addr);
                break;
            case 2:
                // if (entry && entry->rt_is_used)
                // mac_address macAddressConv;
                cObject * ctrl;
                ctrl = p->removeControlInfo();
                if (ctrl!=nullptr)
                {
                    Ieee802Ctrl * ctrlmac = check_and_cast<Ieee802Ctrl *> (ctrl);
                    if (ctrlmac)
                    {
                       // MACAddress macAddressConv = ctrlmac->getSrc(); /* destination eth addr */
                        // ctrlmac->getSrc().getAddressBytes(macAddressConv.address);  /* destination eth addr */
                        // ctrlmac->getDest().getAddressBytes(&dest);   /* destination eth addr */
                        delete ctrl;
                        rerr_send(dest_addr, 1, entry, addr);
                    }
                }
                sendICMP(p);
                //delete p;
                break;
            case 1:
                rerr_send(dest_addr, NET_DIAMETER, entry);
            default:
                //  icmpAccess.get()->sendErrorMessage(p, ICMP_DESTINATION_UNREACHABLE, 0);
                sendICMP(p);
                //delete p;
                break;
            }
        }
        scheduleNextEvent();
        return;
    }
    else
    {
        /* DEBUG(LOG_DEBUG, 0, "Sending pkt uid=%d", ch->uid()); */
        if (p->getControlInfo())
            delete p->removeControlInfo();
        if (isInMacLayer())
        {
            Ieee802Ctrl *ctrl = new Ieee802Ctrl();
            ctrl->setDest(entry->rt_nxthop_addr.s_addr.toMAC());
            //TODO ctrl->setEtherType(...);
            p->setControlInfo(ctrl);
        }

        send(p, "to_ip");
        /* When forwarding data, make sure we are sending HELLO messages */
        //gettimeofday(&this_host.fwd_time, nullptr);
        hello_init();
    }
}


/*
struct dev_info NS_CLASS dev_ifindex (int ifindex)
{
 int index = ifindex2devindex(ifindex);
 return  (this_host.devs[index]);

}

struct dev_info NS_CLASS dev_nr(int n)
{
    return (this_host.devs[n]);
}

int NS_CLASS ifindex2devindex(unsigned int ifindex)
{
    int i;

    for (i = 0; i < this_host.nif; i++)
        if (dev_indices[i] == ifindex)
            return i;

    return -1;
}
*/

void DYMOUM::processLinkBreak(const cObject *details)
{
    IPv4Datagram  *dgram = nullptr;
    if (dynamic_cast<IPv4Datagram *>(const_cast<cObject*> (details)))
    {
        dgram = check_and_cast<IPv4Datagram *>(const_cast<cObject*>(details));
        if (hello_ival<=0)
        {
            packetFailed(dgram);
        }
    }
    else if (dynamic_cast<Ieee80211DataFrame *>(const_cast<cObject*> (details)))
    {
        Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame *>(const_cast<cObject*>(details));
        if (hello_ival<=0)
        {
            packetFailedMac(frame);
        }
    }
    else
        return;

}

void DYMOUM::processPromiscuous(const cObject *details)
{
    Ieee80211DataOrMgmtFrame *frame = nullptr;

    IPv4Datagram * ip_msg = nullptr;
    struct in_addr source;

    source.s_addr = L3Address();

    if (dynamic_cast<Ieee80211DataOrMgmtFrame *>(const_cast<cObject*> (details)))
    {
        MACAddress macAddressConv;
        // mac_address macAddressConv;
        struct in_addr addr;
        struct in_addr gatewayAddr;

        frame = const_cast<Ieee80211DataOrMgmtFrame *>(check_and_cast<const Ieee80211DataOrMgmtFrame *>(details));
        if (!isInMacLayer())
            ip_msg = dynamic_cast<IPv4Datagram *>(frame->getEncapsulatedPacket());
        /////////////////////////////////////
        /////////////////////////////////////
        /////////////////////////////////////
#if 1
        /////////////////////////////////////
        ////
        ////
        //// Promiscuous procesing of packets for refresh reverse route
        ////
        ////
        ////////////////////////////////////

        rtable_entry_t *entry = nullptr;
        if (!isInMacLayer())
        {
            // memcpy (macAddressConv.address,frame->getTransmitterAddress().getAddressBytes(),6);
            // frame->getTransmitterAddress().getAddressBytes(macAddressConv.address);
            macAddressConv = frame->getTransmitterAddress();
            auto it = macToIpAdress->find(macAddressConv);

            if (ip_msg)
                source.s_addr = L3Address(ip_msg->getSrcAddress());

            if (it!=macToIpAdress->end())
            {
                gatewayAddr.s_addr = it->second;
            }
            else
            {
                if (ip_msg && ip_msg->getTransportProtocol()==IP_PROT_MANET)
                {
                    L3Address ip_src = ip_msg->getSourceAddress();
                    macToIpAdress->insert(std::make_pair(macAddressConv, ip_src));
                    gatewayAddr.s_addr = ip_src;
                }
                else
                    return; // can procces the message, don't know the sender
            }
        }
        else
        {
            gatewayAddr.s_addr = L3Address(frame->getTransmitterAddress());
        }


        entry = rtable_find(gatewayAddr);

        if (entry)
        {
            uint32_t cost = 1;
            uint8_t hopfix = 0;
            if (this->isStaticNode())
                hopfix++;
            if (entry->rt_hopcnt == 1)
            {
                cost = entry->cost;
                hopfix = entry->rt_hopfix;
            }
            rtable_update(entry,            // routing table entry
                          gatewayAddr,    // dest
                          gatewayAddr,    // nxt hop
                          NS_DEV_NR,      // iface
                          entry->rt_seqnum,           // seqnum
                          entry->rt_prefix,       // prefix
                          1,  // hop count
                          entry->rt_is_gw,       // is gw
                          cost,
                          hopfix);
            //rtable_update_timeout(entry);
        }

        if (gatewayAddr.s_addr!=source.s_addr && !source.s_addr.isUnspecified())
        {
            entry = rtable_find(source);
            if (entry && gatewayAddr.s_addr == entry->rt_nxthop_addr.s_addr)
            {
                rtable_update(entry,            // routing table entry
                              source, // dest
                              gatewayAddr,    // nxt hop
                              NS_DEV_NR,      // iface
                              entry->rt_seqnum,           // seqnum
                              entry->rt_prefix,       // prefix
                              entry->rt_hopcnt,   // hop count
                              entry->rt_is_gw,       // is gw
                              entry->cost,
                              entry->rt_hopfix);
                //rtable_update_timeout(entry);
            }
        }

        /////////////////
        //// endif  /////
        /////////////////

#endif

        // if rrep proccess the packet
        if (!no_path_acc)
        {
            DYMO_element *dymo_msg = nullptr;
            if (!isInMacLayer())
            {
                if (ip_msg && ip_msg->getTransportProtocol()==IP_PROT_MANET)
                    dymo_msg = dynamic_cast<DYMO_element *>(ip_msg->getEncapsulatedPacket()->getEncapsulatedPacket());
            }
            else
            {
                dymo_msg = dynamic_cast<DYMO_element *>(frame->getEncapsulatedPacket());
            }
            if (dymo_msg)
            {
                // check if RREP
                if ((dymo_msg->type==DYMO_RE_TYPE) && (((RE *) dymo_msg)->a==0))
                {
                    //  proccess RREP
                    addr.s_addr = L3Address(ip_msg->getSrcAddress());
                    promiscuous_rrep((RE*)dymo_msg, addr);
                } // end if promiscuous
                //else if (dymo_msg->type==DYMO_RERR_TYPE)
                //{
                //}
            }  // end if dymo msg
        } // end if no_path_acc
    }
}

void DYMOUM::processFullPromiscuous(const cObject *details)
{
    rtable_entry_t *entry;
    Ieee80211TwoAddressFrame *twoAddressFrame;
    if (dynamic_cast<Ieee80211TwoAddressFrame *>(const_cast<cObject*> (details)))
    {
        MACAddress macAddressConv;
        struct in_addr addr;
        twoAddressFrame = const_cast<Ieee80211TwoAddressFrame *>(check_and_cast<const Ieee80211TwoAddressFrame *>(details));
        if (!isInMacLayer())
        {
            macAddressConv = twoAddressFrame->getTransmitterAddress();
            auto it = macToIpAdress->find(macAddressConv);
            if (it!=macToIpAdress->end())
                addr.s_addr = it->second;
            else
            {
                IPv4Datagram * ip_msg = dynamic_cast<IPv4Datagram *>(twoAddressFrame->getEncapsulatedPacket());
                if (ip_msg && ip_msg->getTransportProtocol()==IP_PROT_MANET)
                {
                    L3Address ip_src = ip_msg->getSourceAddress();
                    macToIpAdress->insert(std::make_pair(macAddressConv, ip_src));
                    //  gatewayAddr.s_addr = ip_msg->getSrcAddress().getInt();
                }
                else
                    return; // can procces the message, don't know the sender
            }
        }
        else
        {
            addr.s_addr = L3Address(twoAddressFrame->getTransmitterAddress());
        }

        entry = rtable_find(addr);
        if (entry)
        {
            uint32_t cost = 1;
            uint8_t hopfix = 0;
            if (this->isStaticNode())
                hopfix++;
            if (entry->rt_hopcnt == 1)
            {
                cost = entry->cost;
                hopfix = entry->rt_hopfix;
            }
            rtable_update(entry,            // routing table entry
                          addr,   // dest
                          addr,   // nxt hop
                          NS_DEV_NR,      // iface
                          entry->rt_seqnum,           // seqnum
                          entry->rt_prefix,       // prefix
                          1,  // hop count
                          entry->rt_is_gw,        // is gw
                          cost,
                          hopfix);
            //rtable_update_timeout(entry);
        }
        // if rrep proccess the packet

        if (!no_path_acc)
        {
            IPv4Datagram *ip_msg = nullptr;
            DYMO_element *dymo_msg = nullptr;
            if (!isInMacLayer())
            {
                ip_msg = dynamic_cast<IPv4Datagram *>(twoAddressFrame->getEncapsulatedPacket());
                if (ip_msg)
                {
                    if (ip_msg->getTransportProtocol()==IP_PROT_MANET)
                        dymo_msg = dynamic_cast<DYMO_element *>(ip_msg->getEncapsulatedPacket()->getEncapsulatedPacket());
                }
            }
            else
            {
                dymo_msg = dynamic_cast<DYMO_element *>(twoAddressFrame->getEncapsulatedPacket());
            }
            if (dymo_msg)
            {
                // check if RREP
                if ((dymo_msg->type==DYMO_RE_TYPE) && (((RE *) dymo_msg)->a==0))
                {
                    //  proccess RREP
                    addr.s_addr = L3Address(ip_msg->getSrcAddress());    //FIXME ip_msg may be nullptr!
                    promiscuous_rrep((RE*)dymo_msg, addr);
                } // end if promiscuous
                //else if (dymo_msg->type==DYMO_RERR_TYPE)
                //{
                //}
            }  // end if dymo msg
        } // end if no_path_acc
    }
}



void DYMOUM::promiscuous_rrep(RE * dymo_re, struct in_addr ip_src)
{
    struct in_addr node_addr;
    rtable_entry_t *entry;


    // check if my address is in the message
    int num_blk = dymo_re->numBlocks();

    for (int i=0; i<num_blk; i++)
    {
        if (addressIsForUs(dymo_re->re_blocks[i].re_node_addr))
            return;
    }

    for (int i=0; i<num_blk; i++)
    {
        //if (dymo_re->re_blocks[i].re_hopcnt+1>2)
        //  continue;
        node_addr.s_addr = dymo_re->re_blocks[i].re_node_addr;
        entry = rtable_find(node_addr);
        uint32_t seqnum = ntohl(dymo_re->re_blocks[i].re_node_seqnum);
        struct re_block b;
        memcpy(&b, &dymo_re->re_blocks[i], sizeof(struct re_block));
        b.re_hopcnt += 1;
        if (this->isStaticNode())
            b.re_hopfix++;

        int rb_state;
        if (isInMacLayer())
        {
            if (!entry)
                rb_state = RB_FRESH;
            else if (seqnum > entry->rt_seqnum)
                rb_state = RB_FRESH;
            else if (seqnum == entry->rt_seqnum && b.re_hopcnt < entry->rt_hopcnt)
                rb_state = RB_FRESH;
            else if (seqnum == 0)
                rb_state = RB_FRESH; // Asume Olsr update
            else
                rb_state = RB_STALE;
        }
        else
        {
            rb_state = re_info_type(&b, entry, 0);
        }


        if (rb_state != RB_FRESH)
            continue;

        if (entry)
        {
            bool update = false;
            if (((int32_t) seqnum) - ((int32_t) entry->rt_seqnum)>0)
                update = true;
            else if (seqnum == entry->rt_seqnum && b.re_hopcnt < entry->rt_hopcnt)
                update = true;
            else if (isInMacLayer() && seqnum == 0)
                update = true;

            if (update)
                rtable_update(
                    entry,          // routing table entry
                    node_addr,      // dest
                    ip_src,         // nxt hop
                    NS_DEV_NR,      // iface
                    seqnum,         // seqnum
                    b.prefix,       // prefix
                    b.re_hopcnt,    // hop count
                    b.g,            // is gw
                    b.cost,
                    b.re_hopfix);
        }
        else
        {
            rtable_insert(
                node_addr,      // dest
                ip_src,         // nxt hop
                NS_DEV_NR,      // iface
                seqnum,         // seqnum
                b.prefix,       // prefix
                b.re_hopcnt,    // hop count
                b.g,       // is gw
                b.cost,
                b.re_hopfix);
        }

    }
    // add this packet
}

/* Called for packets whose delivery fails at the link layer */
void DYMOUM::packetFailed(IPv4Datagram *dgram)
{
    rtable_entry_t *rt;
    struct in_addr dest_addr, src_addr, next_hop;

    src_addr.s_addr = L3Address(dgram->getSrcAddress());
    dest_addr.s_addr = L3Address(dgram->getDestAddress());

    /* We don't care about link failures for broadcast or non-data packets */
    if (dgram->getDestAddress().getInt() == IP_BROADCAST ||
            dgram->getDestAddress().getInt() == DYMO_BROADCAST)
    {
        scheduleNextEvent();
        return;
    }
    EV_WARN << "LINK FAILURE for dest=" << dgram->getDestAddress();
    rt = rtable_find(dest_addr);
    if (rt)
    {
        next_hop.s_addr = rt->rt_nxthop_addr.s_addr;
        /*
            DEBUG(LOG_DEBUG, 0, "LINK FAILURE for next_hop=%s dest=%s uid=%d",
              ip_to_str(next_hop), ip_to_str(dest_addr), ch->uid());
        */
//      rtable_expire_timeout_all(next_hop, NS_IFINDEX);
        int count = 0;
#ifndef MAPROUTINGTABLE
        dlist_head_t *pos;
        dlist_for_each(pos, &rtable.l)
        {
            rtable_entry_t *entry = (rtable_entry_t *) pos;
            if (entry->rt_nxthop_addr.s_addr == next_hop.s_addr &&
                    entry->rt_ifindex == NS_IFINDEX)
            {
#ifdef RERRPACKETFAILED
                rerr_send(entry->rt_dest_addr, NET_DIAMETER, entry);
#endif
                count += rtable_expire_timeout(entry);
            }
        }
#else
        for (auto & elem : *dymoRoutingTable)
        {
            rtable_entry_t * entry = elem.second;
            if (entry->rt_nxthop_addr.s_addr == next_hop.s_addr)
            {
#ifdef RERRPACKETFAILED
                rerr_send(entry->rt_dest_addr, NET_DIAMETER, entry);
#endif
                count += rtable_expire_timeout(entry);
            }
        }
#endif
    }
    else
    {
        struct in_addr nm;
        nm.s_addr = L3Address(IPv4Address::ALLONES_ADDRESS);
        omnet_chg_rte(dest_addr,dest_addr, nm,0,true);
    }
    scheduleNextEvent();
}

/* Called for packets whose delivery fails at the link layer */
void DYMOUM::packetFailedMac(Ieee80211DataFrame *dgram)
{
    struct in_addr dest_addr, src_addr, next_hop;
    if (dgram->getReceiverAddress().isBroadcast())
    {
        scheduleNextEvent();
        return;
    }

    src_addr.s_addr = L3Address(dgram->getAddress3());
    dest_addr.s_addr = L3Address(dgram->getAddress4());
    next_hop.s_addr = L3Address(dgram->getReceiverAddress());
    int count = 0;

    if (isStaticNode() && getCollaborativeProtocol())
    {
        L3Address next;
        int iface;
        double cost;
        if (getCollaborativeProtocol()->getNextHop(next_hop.s_addr, next, iface, cost))
            if(next == next_hop.s_addr)
            {
                scheduleNextEvent();
                return; // both nodes are static, do nothing
            }
    }
#ifndef MAPROUTINGTABLE
    dlist_head_t *pos;
    int count = 0;
    dlist_for_each(pos, &rtable.l)
    {
       rtable_entry_t *entry = (rtable_entry_t *) pos;
       if (entry->rt_nxthop_addr.s_addr == next_hop.s_addr)
       {
#ifdef RERRPACKETFAILED
           rerr_send(entry->rt_dest_addr, NET_DIAMETER, entry);
#endif
           count += rtable_expire_timeout(entry);
        }
    }
#else
    for (auto & elem : *dymoRoutingTable)
    {
        rtable_entry_t *entry = elem.second;
        if (entry->rt_nxthop_addr.s_addr == next_hop.s_addr)
        {
#ifdef RERRPACKETFAILED
            rerr_send(entry->rt_dest_addr, NET_DIAMETER, entry);
#endif
            count += rtable_expire_timeout(entry);
        }
    }
#endif
    /* We don't care about link failures for broadcast or non-data packets */
    scheduleNextEvent();
}


void DYMOUM::finish()
{
    simtime_t t = simTime();
    packet_queue_destroy();

    if (t==0) return;

    if (iswrite)
        return;

    iswrite = true;

    recordScalar("simulated time", t);
    recordScalar("Dymo totalSend ", totalSend);

    recordScalar("Dymo Rreq send", totalRreqSend);
    recordScalar("Dymo Rreq rec", totalRreqRec);

    recordScalar("Dymo Rrep send", totalRrepSend);
    recordScalar("Dymo Rrep rec", totalRrepRec);
    /*
        recordScalar("rrep ack send", totalRrepAckSend);
        recordScalar("rrep ack rec", totalRrepAckRec);
    */
    recordScalar("Dymo Rerr send", totalRerrSend);
    recordScalar("Dymo Rerr rec", totalRerrRec);
}


std::string DYMOUM::detailedInfo() const
{
    std::stringstream out;

    if (ipNodeId)
        out << "Node  : "  << *ipNodeId  << "  " << ipNodeId->getInt() << "\n";
    else
        out << "Node  : "  << interface80211ptr->getMacAddress() << "  " << interface80211ptr->getMacAddress().getInt() << "\n";
    out << "Seq Num  : "  <<this_host.seqnum  << "\n";

    return out.str();
}


uint32_t DYMOUM::getRoute(const L3Address &dest, std::vector<L3Address> &add)
{
    return 0;
}


bool  DYMOUM::getNextHop(const L3Address &dest, L3Address &add, int &iface, double &cost)
{
    L3Address destAddr = dest;
    L3Address apAddr;
    if (getAp(dest,apAddr))
    {
        destAddr = apAddr;
    }
    rtable_entry_t * fwd_rt = nullptr;
    auto it = dymoRoutingTable->find(destAddr);
    if (it != dymoRoutingTable->end())
    {
          if (it->second)
          {
              // sanity check
              if (it->second->rt_dest_addr.s_addr != destAddr)
              {
                  throw cRuntimeError("Dymo routing data base error");
              }
              fwd_rt = it->second;
          }
          else
              throw cRuntimeError("Dymo routing data base error, nullptr entry");
    }
    if (fwd_rt)
    {
        if (fwd_rt->rt_state != RT_VALID)
            return false;
        add = fwd_rt->rt_nxthop_addr.s_addr;
        InterfaceEntry * ie = getInterfaceEntry(fwd_rt->rt_ifindex);
        iface = ie->getInterfaceId();
        cost = fwd_rt->rt_hopcnt;
        return true;
    }
    return false;
}

bool DYMOUM::isProactive()
{
    return false;
}

void DYMOUM::setRefreshRoute(const L3Address &destination, const L3Address & nextHop,bool isReverse)
{
    struct in_addr dest_addr, next_hop;

    dest_addr.s_addr = destination;
    next_hop.s_addr = nextHop;

    rtable_entry_t *route = nullptr;
    rtable_entry_t *fwd_pre_rt = nullptr;
    L3Address dest = destination;
    L3Address next = nextHop;

    bool change = false;
    L3Address apAddr;
    if (getAp(destination,apAddr))
    {
        dest = apAddr;
    }
    if (getAp(nextHop,apAddr))
    {
        next = apAddr;
    }

    auto it = dymoRoutingTable->find(next);
    if (it != dymoRoutingTable->end())
    {
          if (it->second)
          {
              // sanity check
              if (it->second->rt_dest_addr.s_addr != next)
              {
                  throw cRuntimeError("Dymo routing data base error");
              }
              route = it->second;
          }
          else
              throw cRuntimeError("Dymo routing data base error, nullptr entry");
    }

    it = dymoRoutingTable->find(dest);
    if (it != dymoRoutingTable->end())
    {
          if (it->second)
          {
              // sanity check
              if (it->second->rt_dest_addr.s_addr != dest)
              {
                  throw cRuntimeError("Dymo routing data base error");
              }
              fwd_pre_rt = it->second;
          }
          else
              throw cRuntimeError("Dymo routing data base error, nullptr entry");
    }


    if (par("checkNextHop").boolValue())
    {
        if (route && (route->rt_nxthop_addr.s_addr == next))
        {
            rtable_update_timeout(route);
            change = true;
        }

        if (fwd_pre_rt && (fwd_pre_rt->rt_nxthop_addr.s_addr == next))
        {
            rtable_update_timeout(fwd_pre_rt);
            change = true;
        }
    }
    else
    {
        if (route)
        {
            rtable_update_timeout(route);
            change = true;
        }
        if (fwd_pre_rt)
        {
            rtable_update_timeout(fwd_pre_rt);
            change = true;
        }
    }

    if (change)
    {
        Enter_Method_Silent();
        scheduleNextEvent();
    }
    return;
    /*
    if (isReverse && !route && nextHop!=(Address)0)
    {
    // Gratuitous Return Path

        struct in_addr node_addr;
        struct in_addr  ip_src;
        node_addr.s_addr = destination;
        ip_src.s_addr = nextHop;
        rtable_insert(
            node_addr,      // dest
            ip_src,         // nxt hop
            NS_DEV_NR,      // iface
            0,  // seqnum
            0,  // prefix
            0,  // hop count
            0); // is gw
        change = true;
    }
    */
}

bool DYMOUM::isOurType(cPacket * msg)
{
    DYMO_element *re = dynamic_cast <DYMO_element *>(msg);
    if (re)
        return true;
    return false;
}

bool DYMOUM::getDestAddress(cPacket *msg, L3Address &dest)
{
    RE *re = dynamic_cast <RE *>(msg);
    if (!re)
        return false;
    if (re->a)
    {
        dest = re->target_addr;
        return true;
    }
    else
        return false;
}

// Group methods, allow the anycast procedure
int DYMOUM::getRouteGroup(const AddressGroup &gr, std::vector<L3Address> &addr)
{
    return 0;
}

int  DYMOUM::getRouteGroup(const L3Address& dest, std::vector<L3Address> &add, L3Address& gateway, bool &isGroup, int group)
{
    return 0;
}

bool DYMOUM::getNextHopGroup(const AddressGroup &gr, L3Address &add, int &iface, L3Address& gw)
{
    int distance = 1000;
    for (const auto & elem : gr)
    {
        struct in_addr destAddr;
        destAddr.s_addr = elem;
        rtable_entry_t * fwd_rt = rtable_find(destAddr);
        if (!fwd_rt)
            continue;
        if (fwd_rt->rt_state != RT_VALID)
            continue;
        if (distance<fwd_rt->rt_hopcnt || (distance==fwd_rt->rt_hopcnt && intrand(1)))
            continue;
        distance = fwd_rt->rt_hopcnt;
        add = fwd_rt->rt_nxthop_addr.s_addr;
        InterfaceEntry * ie = getInterfaceEntry(fwd_rt->rt_ifindex);
        iface = ie->getInterfaceId();
        gw = elem;
    }
    if (distance==1000)
        return false;
    return true;
}

bool DYMOUM::getNextHopGroup(const L3Address& dest, L3Address &next, int &iface, L3Address& gw, bool &isGroup, int group)
{
    AddressGroup gr;
    bool find = false;
    if (findInAddressGroup(dest, group))
    {
        getAddressGroup(gr, group);
        find = getNextHopGroup(gr, next, iface, gw);
        isGroup = true;

     }
    else
    {
        double cost;
        find = getNextHop(dest, next, iface, cost);
        isGroup = false;
    }
    return find;
}


//// End group methods
#ifndef DYMO_USE_STL
cPacket * DYMOUM::get_packet_queue(struct in_addr dest_addr)
{
    dlist_head_t *pos;
    dlist_for_each(pos, &PQ.head)
    {
        struct q_pkt *qp = (struct q_pkt *)pos;
        if (qp->dest_addr.s_addr == dest_addr.s_addr)
        {
            qp->inTransit = true;
            return qp->p;
        }
    }
    return nullptr;
}
#else
cPacket * DYMOUM::get_packet_queue(struct in_addr dest_addr)
{
    for (auto & elem : PQ.pkQueue)
    {
        struct q_pkt *qp = elem;
        if (qp->dest_addr.s_addr == dest_addr.s_addr)
        {
            qp->inTransit = true;
            return qp->p;
        }
    }
    return nullptr;
}
#endif
// proactive RREQ
void DYMOUM::rreq_proactive(void *arg)
{
    struct in_addr dest;
    if (!isRoot)
         return;
    if (this->isInMacLayer())
         dest.s_addr = L3Address(MACAddress::BROADCAST_ADDRESS);
    else
         dest.s_addr = L3Address(IPv4Address::ALLONES_ADDRESS);
    re_send_rreq(dest, 0, NET_DIAMETER);
    timer_set_timeout(&proactive_rreq_timer, proactive_rreq_timeout);
    timer_add(&proactive_rreq_timer);
}


int DYMOUM::re_info_type(struct re_block *b, rtable_entry_t *e, u_int8_t is_rreq)
{
    u_int32_t node_seqnum;
    int32_t sub;

    assert(b);

    // If the block was issued from one interface of the processing node,
    // then the block is considered stale
    if (addressIsForUs(b->re_node_addr))
        return RB_SELF_GEN;

    if (e)
    {
        node_seqnum = ntohl(b->re_node_seqnum);
        sub = ((int32_t) node_seqnum) - ((int32_t) e->rt_seqnum);

        if (b->from_proactive)
        {
            if (e->rt_state != RT_VALID)
                return RB_PROACTIVE;

            if (sub == 0 && e->rt_hopcnt != 0 && b->re_hopcnt != 0 && b->re_hopcnt < e->rt_hopcnt)
                return RB_PROACTIVE;
        }

        if (sub < 0)
            return RB_STALE;
        if (!useHover)
        {
            if (sub == 0)
            {
                if (e->rt_hopcnt == 0 || b->re_hopcnt == 0 || b->re_hopcnt > e->rt_hopcnt + 1)
                    return RB_LOOP_PRONE;
                if (e->rt_state == RT_VALID && (b->re_hopcnt > e->rt_hopcnt || (b->re_hopcnt == e->rt_hopcnt && is_rreq)))
                    return RB_INFERIOR;
            }
        }
        else
        {
            if (sub == 0)
            {
                if (e->rt_hopcnt == 0 || b->re_hopcnt == 0 || b->re_hopcnt > e->rt_hopcnt + 1)
                    return RB_LOOP_PRONE;
                if (e->rt_state == RT_VALID && (b->cost > e->cost || (b->cost == e->cost && is_rreq)))
                    return RB_INFERIOR;
            }
        }
    }
    return RB_FRESH;
}


void DYMOUM::processLocatorAssoc(const cObject *details)
{
#ifdef WITH_80211MESH
    LocatorNotificationInfo *infoLoc = check_and_cast<LocatorNotificationInfo*>(details);
    L3Address destAddr;
    L3Address apAddr;
    if (isInMacLayer())
        destAddr = L3Address(infoLoc->getMacAddr());
    else
    {
        if (infoLoc->getIpAddr().isUnspecified())
            return;
        destAddr = L3Address(infoLoc->getIpAddr());
    }

    if (getAp(destAddr, apAddr))
    {
        struct in_addr dest_addrAux;
        dest_addrAux.s_addr = destAddr;
        rtable_entry_t * fwd_rt = rtable_find(dest_addrAux);
        dest_addrAux.s_addr = apAddr;
        rtable_entry_t * fwd_rt_ap = rtable_find(dest_addrAux);

        if (!fwd_rt_ap && fwd_rt)
        {
            rtable_delete(fwd_rt);
            return;
        }
        if (fwd_rt_ap && fwd_rt && memcmp(fwd_rt_ap, fwd_rt_ap, sizeof(rtable_entry_t)) != 0)
        {
            dest_addrAux.s_addr = destAddr;
            if (fwd_rt)
                rtable_update(fwd_rt, dest_addrAux, fwd_rt_ap->rt_nxthop_addr, fwd_rt_ap->rt_ifindex,
                        fwd_rt_ap->rt_seqnum, fwd_rt_ap->rt_prefix, fwd_rt_ap->rt_hopcnt, 0, fwd_rt_ap->cost,
                        fwd_rt_ap->rt_hopfix);
            else
                rtable_insert(dest_addrAux, fwd_rt_ap->rt_nxthop_addr, fwd_rt_ap->rt_ifindex, fwd_rt_ap->rt_seqnum,
                        fwd_rt_ap->rt_prefix, fwd_rt_ap->rt_hopcnt, 0, fwd_rt_ap->cost, fwd_rt_ap->rt_hopfix);
        }
        return;
    }
#endif
}

void DYMOUM::processLocatorDisAssoc(const cObject *details)
{
#ifdef WITH_80211MESH
    LocatorNotificationInfo *infoLoc = check_and_cast<LocatorNotificationInfo*>(details);
    L3Address destAddr;
    if (isInMacLayer())
         destAddr = L3Address(infoLoc->getMacAddr());
    else
    {
        if (infoLoc->getIpAddr().isUnspecified())
            return;
        destAddr = L3Address(infoLoc->getIpAddr());
    }
    struct in_addr dest_addrAux;
    dest_addrAux.s_addr = destAddr;
    rtable_entry_t * fwd_rt = rtable_find(dest_addrAux);
    if (fwd_rt)
    {
        rtable_delete(fwd_rt);
        return;
    }
#endif
}

} // namespace inetmanet

} // namespace inet

