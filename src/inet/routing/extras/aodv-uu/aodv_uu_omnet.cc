/*****************************************************************************
 *
 * Copyright (C) 2002 Uppsala University.
 * Copyright (C) 2006 Malaga University.
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

#include <string.h>
#include <assert.h>


#include "inet/common/ModuleAccess.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/extras/base/ControlManetRouting_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/linklayer/common/MacAddressTag_m.h"


#include "inet/linklayer/common/InterfaceTag_m.h"

#include "inet/routing/extras/aodv-uu/aodv_uu_omnet.h"

namespace inet {

using namespace ieee80211;

namespace inetmanet {

const int UDP_HEADER_BYTES = 8;
typedef std::vector<Ipv4Address> IPAddressVector;

Define_Module(AODVUU);

/* Constructor for the AODVUU routing agent */

bool AODVUU::log_file_fd_init=false;
int AODVUU::log_file_fd = -1;

#ifdef AODV_GLOBAL_STATISTISTIC
bool AODVUU::iswrite = false;
int AODVUU::totalSend=0;
int AODVUU::totalRreqSend=0;
int AODVUU::totalRreqRec=0;
int AODVUU::totalRrepSend=0;
int AODVUU::totalRrepRec=0;
int AODVUU::totalRrepAckSend=0;
int AODVUU::totalRrepAckRec=0;
int AODVUU::totalRerrSend=0;
int AODVUU::totalRerrRec=0;
int AODVUU::totalLocalRep =0;
#endif
std::map<L3Address,u_int32_t *> AODVUU::mapSeqNum;

void NS_CLASS initialize(int stage)
{
     /*
       Enable usage of some of the configuration variables from Tcl.

       Note: Do NOT change the values of these variables in the constructor
       after binding them! The desired default values should be set in
       ~ns/tcl/lib/ns-default.tcl instead.
     */
    ManetRoutingBase::initialize(stage);
    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {

        RERR_UDEST_SIZE = 4+getAddressSize();
        RERR_SIZE = 8+getAddressSize();
        RREP_SIZE = (getAddressSize()*2)+12;
        RREQ_SIZE = 16+(getAddressSize()*2);


#ifndef AODV_GLOBAL_STATISTISTIC
        iswrite = false;
        totalSend=0;
        totalRreqSend=0;
        totalRreqRec=0;
        totalRrepSend=0;
        totalRrepRec=0;
        totalRrepAckSend=0;
        totalRrepAckRec=0;
        totalRerrSend=0;
        totalRerrRec=0;
        totalLocalRep=0;
#endif
        log_to_file = 0;
        hello_jittering = 0;
        optimized_hellos = 0;
        expanding_ring_search = 0;
        local_repair = 0;
        debug=0;
        rreq_gratuitous =0;

        //sendMessageEvent = new cMessage();

        if ((bool)par("log_to_file"))
            log_to_file = 1;

        if ((bool) par("hello_jittering"))
            hello_jittering = 1;

        if ((bool)par("optimized_hellos"))
            optimized_hellos  = 1;

        if ((bool)par("expanding_ring_search"))
            expanding_ring_search = 1;

        if ((bool) par("local_repair"))
            local_repair = 1;

        if ((bool)par("rreq_gratuitous"))
            rreq_gratuitous = 1;

        if ((bool)par("debug"))
            debug = 1;

        if (hasPar("RreqDelayInReception"))
            storeRreq = par(("RreqDelayInReception")).boolValue();
        checkRrep = false;

        useIndex = par("UseIndex");
        unidir_hack = (int) par("unidir_hack");

        receive_n_hellos    = (int) par("receive_n_hellos");
        wait_on_reboot = (int) par ("wait_on_reboot");
        rt_log_interval = (int) par("rt_log_interval"); // Note: in milliseconds!
        ratelimit = (int) par("ratelimit");
        llfeedback = 0;
        if (par("llfeedback"))
            llfeedback = 1;
        internet_gw_mode = (int) par("internet_gw_mode");
        gateWayAddress = new L3Address(par("internet_gw_address").stringValue());

        if (llfeedback)
        {
            active_route_timeout = ACTIVE_ROUTE_TIMEOUT_LLF;
            ttl_start = TTL_START_LLF;
            delete_period =  DELETE_PERIOD_LLF;
        }
        else
        {
            active_route_timeout = (int) par("active_timeout");// ACTIVE_ROUTE_TIMEOUT_HELLO;
            ttl_start = TTL_START_HELLO;
            delete_period = DELETE_PERIOD_HELLO;
        }

        if (hasPar("avoidDupRREP") && llfeedback)
            checkRrep = par("avoidDupRREP").boolValue();

        /* Initialize common manet routing protocol structures */
        registerRoutingModule();
        if (llfeedback)
            linkLayerFeeback();

        if (hasPar("fullPromis"))
        {
            if (par("fullPromis").boolValue())
            {
                linkFullPromiscuous();
            }
        }

        /* From main.c */
        progname = strdup("AODV-UU");
        /* From debug.c */
        /* Note: log_nmsgs was never used anywhere */
        log_nmsgs = 0;
        log_rt_fd = -1;
#ifndef  _WIN32

        if (debug && !log_file_fd_init)
        {
            log_file_fd = -1;
            openlog("aodv-uu ",0,LOG_USER);
            log_init();
            log_file_fd_init=true;
        }
#else
        debug = 0;
#endif
        /* Set host parameters */
        memset(&this_host, 0, sizeof(struct host_info));
        memset(dev_indices, 0, sizeof(unsigned int) * MAX_NR_INTERFACES);
        this_host.seqno = 1;
        this_host.rreq_id = 0;
        this_host.nif = 1;


        for (int i = 0; i < MAX_NR_INTERFACES; i++)
            DEV_NR(i).enabled=0;

        for (int i = 0; i <getNumInterfaces(); i++)
        {
            DEV_NR(i).ifindex = i;
            dev_indices[i] = i;
            strcpy(DEV_NR(i).ifname, getInterfaceEntry(i)->getInterfaceName());
            if (!isInMacLayer())
            {
                auto ie = getInterfaceEntry(i);
                auto ifaceData = ie->getProtocolData<Ipv4InterfaceData>();
                DEV_NR(i).netmask.s_addr = L3Address(ifaceData->getIPAddress().getNetworkMask());
                DEV_NR(i).ipaddr.s_addr = L3Address(ifaceData->getIPAddress());
            }
            else
            {
                DEV_NR(i).netmask.s_addr = L3Address(MacAddress::BROADCAST_ADDRESS);
                DEV_NR(i).ipaddr.s_addr = L3Address(getInterfaceEntry(i)->getMacAddress());

            }
            if (getInterfaceEntry(i)->isLoopback())
                continue;
            if (isInMacLayer())
            {
                mapSeqNum[DEV_NR(i).ipaddr.s_addr] = &this_host.seqno;
            }
        }
        /* Set network interface parameters */
        for (int i=0; i < getNumWlanInterfaces(); i++)
        {
            DEV_NR(getWlanInterfaceIndex(i)).enabled = 1;
            DEV_NR(getWlanInterfaceIndex(i)).sock = -1;
            DEV_NR(getWlanInterfaceIndex(i)).broadcast.s_addr = L3Address(Ipv4Address(AODV_BROADCAST));
        }

        NS_DEV_NR = getWlanInterfaceIndexByAddress();
        NS_IFINDEX = getWlanInterfaceIndexByAddress();
#ifndef AODV_USE_STL
        list_t *lista_ptr;
        lista_ptr=&rreq_records;
        INIT_LIST_HEAD(&rreq_records);
        lista_ptr=&rreq_blacklist;
        INIT_LIST_HEAD(&rreq_blacklist);
        lista_ptr=&seekhead;
        INIT_LIST_HEAD(&seekhead);

        lista_ptr=&TQ;
        INIT_LIST_HEAD(&TQ);
#endif
        /* Initialize data structures */
        worb_timer.data = nullptr;
        worb_timer.used = 0;
        hello_timer.data = nullptr;
        hello_timer.used = 0;
        rt_log_timer.data = nullptr;
        rt_log_timer.used = 0;
        isRoot = par("isRoot");
        costStatic = par("costStatic");
        costMobile = par("costMobile");
        useHover = par("useHover");
        proactive_rreq_timeout= par("proactiveRreqTimeout").intValue();

        if (isRoot)
        {
            timer_init(&proactive_rreq_timer,&NS_CLASS rreq_proactive, nullptr);
            timer_set_timeout(&proactive_rreq_timer, par("startRreqProactive").intValue());
        }
        if (!isInMacLayer())
            registerHook();

        propagateProactive = par("propagateProactive");
        nodeName = getContainingNode(this)->getFullName();
        aodv_socket_init();
        rt_table_init();
        packet_queue_init();
        startAODVUUAgent();

        is_init=true;
        // Initialize the timer
        scheduleNextEvent();
        EV_INFO << "Aodv active"<< "\n";
    }
}

/* Destructor for the AODV-UU routing agent */
NS_CLASS ~AODVUU()
{
#ifdef AODV_USE_STL_RT
    while (!aodvRtTableMap.empty())
    {
        free (aodvRtTableMap.begin()->second);
        aodvRtTableMap.erase(aodvRtTableMap.begin());
    }
#else
    list_t *tmp = nullptr, *pos = nullptr;
    for (int i = 0; i < RT_TABLESIZE; i++)
    {
        list_foreach_safe(pos, tmp, &rt_tbl.tbl[i])
        {
            rt_table_t *rt = (rt_table_t *) pos;
            list_detach(&rt->l);
            precursor_list_destroy(rt);
            free(rt);
        }
    }
#endif
#ifndef AODV_USE_STL
    while (!list_empty(&rreq_records))
    {
        pos = list_first(&rreq_records);
        list_detach(pos);
        if (pos) free(pos);
    }

    while (!list_empty(&rreq_blacklist))
    {
        pos = list_first(&rreq_blacklist);
        list_detach(pos);
        if (pos) free(pos);
    }

    while (!list_empty(&seekhead))
    {
        pos = list_first(&seekhead);
        list_detach(pos);
        if (pos) free(pos);
    }
#else
    while (!rreq_records.empty())
    {
        free (rreq_records.back());
        rreq_records.pop_back();
    }

    while (!rreq_blacklist.empty())
    {
        free (rreq_blacklist.begin()->second);
        rreq_blacklist.erase(rreq_blacklist.begin());
    }

    while (!seekhead.empty())
    {
        delete (seekhead.begin()->second);
        seekhead.erase(seekhead.begin());
    }
#endif
    packet_queue_destroy();
    cancelAndDelete(sendMessageEvent);
    log_cleanup();
    delete gateWayAddress;
}

/*
  Moves pending packets with a certain next hop from the interface
  queue to the packet buffer or simply drops it.
*/

/* Called for packets whose delivery fails at the link layer */
void NS_CLASS packetFailed(const Packet *dgram)
{
    rt_table_t *rt_next_hop, *rt;
    struct in_addr dest_addr, src_addr, next_hop;


    const auto& networkHeader = getNetworkProtocolHeader(const_cast<Packet *>(dgram));
    const L3Address& destAddr = networkHeader->getDestinationAddress();
    const L3Address& sourceAddr = networkHeader->getSourceAddress();



    src_addr.s_addr = sourceAddr;
    dest_addr.s_addr = destAddr;


    DEBUG(LOG_DEBUG, 0, "Got failure callback");
    /* We don't care about link failures for broadcast or non-data packets */
    if (destAddr.toIpv4().getInt() == IP_BROADCAST ||
            destAddr.toIpv4().getInt() == AODV_BROADCAST)
    {
        DEBUG(LOG_DEBUG, 0, "Ignoring callback");
        scheduleNextEvent();
        return;
    }


    DEBUG(LOG_DEBUG, 0, "LINK FAILURE for next_hop=%s dest=%s ",ip_to_str(next_hop), ip_to_str(dest_addr));

    if (seek_list_find(dest_addr))
    {
        DEBUG(LOG_DEBUG, 0, "Ongoing route discovery, buffering packet...");
        packet_queue_add_inject(dgram->dup(), dest_addr);
        scheduleNextEvent();
        return;
    }


    rt = rt_table_find(dest_addr);

    if (!rt || rt->state == INVALID)
    {
        scheduleNextEvent();
        return;
    }
    next_hop.s_addr = rt->next_hop.s_addr;
    rt_next_hop = rt_table_find(next_hop);

    if (!rt_next_hop || rt_next_hop->state == INVALID)
    {
        scheduleNextEvent();
        return;
    }

    /* Do local repair? */
    if (local_repair && rt->hcnt <= MAX_REPAIR_TTL)
        /* && ch->num_forwards() > rt->hcnt */
    {
        /* Buffer the current packet */
        packet_queue_add_inject(dgram->dup(), dest_addr);

        // In omnet++ it's not possible to access to the mac queue
        //  /* Buffer pending packets from interface queue */
        //  interfaceQueue((nsaddr_t) next_hop.s_addr, IFQ_BUFFER);
        //  /* Mark the route to be repaired */
        rt_next_hop->flags |= RT_REPAIR;
        neighbor_link_break(rt_next_hop);
        rreq_local_repair(rt, src_addr, nullptr);
    }
    else
    {
        /* No local repair - just force timeout of link and drop packets */
        neighbor_link_break(rt_next_hop);
// In omnet++ it's not possible to access to the mac queue
//  interfaceQueue((nsaddr_t) next_hop.s_addr, IFQ_DROP);
    }
    scheduleNextEvent();
}


/* Called for packets whose delivery fails at the link layer */
void NS_CLASS packetFailedMac(const Packet *dgram)
{
    throw cRuntimeError("TODO: suport link layer");

    const auto &header = dgram->peekAtFront<Ieee80211DataOrMgmtHeader>();
    const auto &header2 = dgram->peekAtFront<Ieee80211DataHeader>();

    rt_table_t *rt_next_hop, *rt;
    struct in_addr dest_addr, src_addr, next_hop;
    if (header->getReceiverAddress().isBroadcast())
    {
        scheduleNextEvent();
        return;
    }

    src_addr.s_addr = L3Address(header->getAddress3());
    dest_addr.s_addr = L3Address(header2->getAddress4());
    if (seek_list_find(dest_addr))
    {
        DEBUG(LOG_DEBUG, 0, "Ongoing route discovery, buffering packet...");
        packet_queue_add(dgram->dup(), dest_addr);
        scheduleNextEvent();
        return;
    }

    next_hop.s_addr = L3Address(header->getReceiverAddress());
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

    rt = rt_table_find(dest_addr);

    if (!rt || rt->state == INVALID)
    {
        scheduleNextEvent();
        return;
    }
    next_hop.s_addr = rt->next_hop.s_addr;
    rt_next_hop = rt_table_find(next_hop);

    if (!rt_next_hop || rt_next_hop->state == INVALID)
    {
        scheduleNextEvent();
        return;
    }

    /* Do local repair? */
    if (local_repair && rt->hcnt <= MAX_REPAIR_TTL)
        /* && ch->num_forwards() > rt->hcnt */
    {
        /* Buffer the current packet */
        packet_queue_add(dgram->dup(), dest_addr);

        // In omnet++ it's not possible to access to the mac queue
        //  /* Buffer pending packets from interface queue */
        //  interfaceQueue((nsaddr_t) next_hop.s_addr, IFQ_BUFFER);
        //  /* Mark the route to be repaired */
        rt_next_hop->flags |= RT_REPAIR;
        neighbor_link_break(rt_next_hop);
        rreq_local_repair(rt, src_addr, nullptr);
    }
    else
    {
        /* No local repair - just force timeout of link and drop packets */
        neighbor_link_break(rt_next_hop);
// In omnet++ it's not possible to access to the mac queue
//  interfaceQueue((nsaddr_t) next_hop.s_addr, IFQ_DROP);
    }
    scheduleNextEvent();
}

INetfilter::IHook::Result NS_CLASS ensureRouteForDatagram(Packet *datagram)
{

    if (isInMacLayer()) {
        // TODO: MAC layer
        throw cRuntimeError("Error AODV-uu HOOK");
        return ACCEPT;
    }

    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    const L3Address& destAddr = networkHeader->getDestinationAddress();

    if (isInMacLayer())
        throw cRuntimeError("Error AODV-uu HOOK");

    if (destAddr.isBroadcast() || isLocalAddress(destAddr) || destAddr.isMulticast())
        return ACCEPT;

    unsigned int ifindex = NS_IFINDEX;  /* Always use ns interface */
    INetfilter::IHook::Result res = processPacket(datagram,ifindex);   // Data path
    scheduleNextEvent();
    return res;

}

void NS_CLASS handleMessageWhenUp(cMessage *msg)
{

    if (is_init == false)
        throw cRuntimeError ("Aodv has not been initialized ");

    if (msg == sendMessageEvent) {
        // timer event
        scheduleNextEvent();
        return;
    }

    if (msg->isSelfMessage() && checkTimer(msg))
    {
        scheduleEvent();
        return;
    }

    Packet *pkt = check_and_cast<Packet *>(msg);
    const auto aodvMsg = pkt->peekAtFront<AODV_msg>();
    if (msg->isSelfMessage() && aodvMsg != nullptr) {
        DelayInfo * delayInfo = check_and_cast<DelayInfo *>(msg->removeControlInfo());
        auto rrep = dynamicPtrCast <RREP> (constPtrCast < AODV_msg > (aodvMsg));
        if (rrep) {
            if (isThisRrepPrevSent(aodvMsg)) {
                delete msg;
                msg = nullptr;
            }
        }
        if (msg)
            aodv_socket_send(pkt, delayInfo->dst, delayInfo->len,
                    delayInfo->ttl, delayInfo->dev, 0);
        delete delayInfo;
        return;
    }

    socket.processMessage(msg);
}

void NS_CLASS  socketDataArrived(UdpSocket *socket, Packet *pkt)
{

    const auto aodvMsg = pkt->peekAtFront<AODV_msg>();
    struct in_addr src_addr;
    struct in_addr dest_addr;

    if (aodvMsg == nullptr) {
        delete pkt;
        scheduleNextEvent();
        return;
    }


    if (!isInMacLayer()) {
        if (!isInMacLayer()) {
            src_addr.s_addr = pkt->getTag<L3AddressInd>()->getSrcAddress();
            // aodvMsg->setControlInfo(check_and_cast<cObject *>(controlInfo));
        } else {
            src_addr.s_addr = L3Address(
                    pkt->getTag<MacAddressInd>()->getSrcAddress());
            //Ieee802Ctrl *controlInfo = check_and_cast<Ieee802Ctrl*>(aodvMsg->getControlInfo());
        }
    }

    if (isLocalAddress(src_addr.s_addr))
        delete pkt;
    else
        recvAODVUUPacket (pkt);
    scheduleNextEvent();
}

void NS_CLASS socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}



/* Starts the AODV-UU routing agent */
int NS_CLASS startAODVUUAgent()
{

    /* Set up the wait-on-reboot timer */
    if (wait_on_reboot)
    {
        timer_init(&worb_timer, &NS_CLASS wait_on_reboot_timeout, &wait_on_reboot);
        timer_set_timeout(&worb_timer, DELETE_PERIOD);
        DEBUG(LOG_NOTICE, 0, "In wait on reboot for %d milliseconds.",DELETE_PERIOD);
    }
    /* Schedule the first HELLO */
    if (!llfeedback && !optimized_hellos)
        hello_start();

    /* Initialize routing table logging */
    if (rt_log_interval)
        log_rt_table_init();

    /* Initialization complete */
    initialized = 1;

    DEBUG(LOG_DEBUG, 0, "Routing agent with IP = %s  started.",
          ip_to_str(DEV_NR(NS_DEV_NR).ipaddr));

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

    /* Schedule the first timeout */
    scheduleNextEvent();
    return 0;

}



// for use with gateway in the future
Packet * NS_CLASS pkt_encapsulate(Packet *p, Ipv4Address gateway)
{

    auto ipv4header = p->peekAtFront<Ipv4Header>();
    if (ipv4header == nullptr) {
        throw cRuntimeError("Is not a datagram header");
    }
    p->trimFront();
    auto oldHeader = removeNetworkProtocolHeader<Ipv4Header>(p);

    auto header = Ptr<Ipv4Header>(oldHeader->dup());
    header->setProtocolId(IP_PROT_IP);
    header->setDestAddress(gateway);
    p->insertAtFront(ipv4header);
    insertNetworkProtocolHeader(p, Protocol::ipv4, header);

    return p;
}



/*
IPv4Datagram *NS_CLASS pkt_decapsulate(IPv4Datagram *p)
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
*/



/*
  Reschedules the timer queue timer to go off at the time of the
  earliest event (so that the timer queue will be investigated then).
  Should be called whenever something might have changed the timer queue.
*/
#ifdef AODV_USE_STL
void NS_CLASS scheduleNextEvent()
{
    simtime_t timer;
    // simtime_t timeout = timer_age_queue();
    timer_age_queue();

    if (!aodvTimerMap.empty())
    {
        timer = aodvTimerMap.begin()->first;
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
    scheduleEvent();
}
#else
void NS_CLASS scheduleNextEvent()
{
    struct timeval *timeout;
    double delay;
    simtime_t timer;
    timeout = timer_age_queue();
    if (timeout)
    {
        delay  = (double)(((double)timeout->tv_usec/(double)1000000.0) +(double)timeout->tv_sec);
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
#endif



/*
  Replacement for if_indextoname(), used in routing table logging.
*/
const char *NS_CLASS if_indextoname(int ifindex, char *ifname)
{
    InterfaceEntry *   ie;
    assert(ifindex >= 0);
    ie = getInterfaceEntry(ifindex);
    return ie->getInterfaceName();
}



void NS_CLASS recvAODVUUPacket(Packet * pkt)
{
    struct in_addr src, dst;
    int ttl;
    int interfaceId;

    auto aodv_msg = pkt->peekAtFront<AODV_msg>();
    int len = B(aodv_msg->getChunkLength()).get();
    int ifIndex = NS_IFINDEX;

    ttl =  aodv_msg->ttl-1;
    if (!isInMacLayer())
    {
        auto l3AddressInd = pkt->getTag<L3AddressInd>();
        auto srcAddr = l3AddressInd->getSrcAddress();
        auto destAddr = l3AddressInd->getDestAddress();

        src.s_addr = srcAddr;
        dst.s_addr = destAddr;
        auto incomingIfTag = pkt->getTag<InterfaceInd>();
        interfaceId = incomingIfTag->getInterfaceId();
    }
    else
    {
        src.s_addr = L3Address(pkt->getTag<MacAddressInd>()->getSrcAddress());
        dst.s_addr = L3Address(pkt->getTag<MacAddressInd>()->getDestAddress());
    }

    InterfaceEntry *   ie;
    if (!isInMacLayer())
    {
        for (int i = 0; i < getNumWlanInterfaces(); i++)
        {
            ie = getWlanInterfaceEntry(i);

            {
                // IPv4InterfaceData *ipv4data = ie->ipv4Data();
                if (interfaceId == ie->getInterfaceId())
                    ifIndex = getWlanInterfaceIndex(i);
            }
        }
    }
    aodv_socket_process_packet(pkt, len, src, dst, ttl, ifIndex);
    delete pkt;
}


void NS_CLASS processMacPacket(Packet * p, const L3Address &dest, const L3Address &src, int ifindex)
{
    struct in_addr dest_addr, src_addr;
    bool isLocal = false;
    struct ip_data *ipd = nullptr;
    u_int8_t rreq_flags = 0;

    dest_addr.s_addr = dest;
    src_addr.s_addr = src;
    rt_table_t *fwd_rt, *rev_rt;

    //InterfaceEntry *   ie = getInterfaceEntry(ifindex);
    isLocal = isLocalAddress(src);

    rev_rt = rt_table_find(src_addr);
    fwd_rt = rt_table_find(dest_addr);


    rt_table_update_route_timeouts(fwd_rt, rev_rt);

    /* OK, the timeouts have been updated. Now see if either: 1. The
       packet is for this node -> ACCEPT. 2. The packet is not for this
       node -> Send RERR (someone want's this node to forward packets
       although there is no route) or Send RREQ. */

    if (!fwd_rt || fwd_rt->state == INVALID ||
            (fwd_rt->hcnt == 1 && (fwd_rt->flags & RT_UNIDIR)))
    {
        // If I am the originating node, then a route discovery
        // must be performed
        if (isLocal || (fwd_rt && (fwd_rt->flags & RT_REPAIR)))
        {
            if (p->getControlInfo())
                delete p->removeControlInfo();
            packet_queue_add(p, dest_addr);
            if (fwd_rt && (fwd_rt->flags & RT_REPAIR))
                rreq_local_repair(fwd_rt, src_addr, ipd);
            else
            {
                if (par("targetOnlyRreq").boolValue())
                    rreq_flags |= RREQ_DEST_ONLY;
                rreq_route_discovery(dest_addr, rreq_flags, ipd);
            }
        }
        // Else we must send a RERR message to the source if
        // the route has been previously used
        else
        {

            Ptr<RERR> rerr;
            DEBUG(LOG_DEBUG, 0,
                    "No route, src=%s dest=%s prev_hop=%s - DROPPING!",
                    ip_to_str(src_addr), ip_to_str(dest_addr));
            if (fwd_rt)
            {
                rerr = rerr_create(0, fwd_rt->dest_addr,fwd_rt->dest_seqno);
                rt_table_update_timeout(fwd_rt, DELETE_PERIOD);
            }
            else
                rerr = rerr_create(0, dest_addr, 0);
            DEBUG(LOG_DEBUG, 0, "Sending RERR to prev hop %s for unknown dest %s",
                    ip_to_str(src_addr), ip_to_str(dest_addr));

                /* Unicast the RERR to the source of the data transmission
                 * if possible, otherwise we broadcast it. */
            struct in_addr rerr_dest;
            if (rev_rt && rev_rt->state == VALID)
                rerr_dest = rev_rt->next_hop;
            else
                rerr_dest.s_addr = L3Address(Ipv4Address(AODV_BROADCAST));
            auto packet = new Packet("Aodv RERR");
            packet->insertAtFront(rerr);
            aodv_socket_send(packet, rerr_dest, RERR_CALC_SIZE(rerr),
                    1, &DEV_IFINDEX(ifindex));
            if (wait_on_reboot)
            {
                DEBUG(LOG_DEBUG, 0, "Wait on reboot timer reset.");
                timer_set_timeout(&worb_timer, DELETE_PERIOD);
            }
            //drop (p);
            sendICMP(p);
            /* DEBUG(LOG_DEBUG, 0, "Dropping pkt uid=%d", ch->uid()); */
            //  icmpAccess.get()->sendErrorMessage(p, ICMP_DESTINATION_UNREACHABLE, 0);
            return;
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
            p->addTagIfAbsent<MacAddressReq>()->setDestAddress(fwd_rt->next_hop.s_addr.toMac());
        }

        send(p, "socketOut");
        /* When forwarding data, make sure we are sending HELLO messages */
        //gettimeofday(&this_host.fwd_time, nullptr);
        if (!llfeedback && optimized_hellos)
            hello_start();
    }
}



INetfilter::IHook::Result  NS_CLASS processPacket(Packet * p, unsigned int ifindex)
{
    u_int8_t rreq_flags = 0;
    struct ip_data *ipd = nullptr;

    ipd = nullptr;         /* No ICMP messaging */

    bool isLocal=true;

    const auto& networkHeader = getNetworkProtocolHeader(p);
    const L3Address& destAddr = networkHeader->getDestinationAddress();
    const L3Address& sourceAddr = networkHeader->getSourceAddress();

    IRoute *route = getInetRoutingTable()->findBestMatchingRoute(destAddr);
    In_addr  dest_addr,src_add;
    dest_addr.S_addr = destAddr;
    src_add.S_addr = sourceAddr;

    rt_table_t * fwd_rt = rt_table_find(dest_addr);
    rt_table_t * rev_rt = rt_table_find(src_add);
    bool unidir = false;
    if (fwd_rt && (fwd_rt->hcnt == 1 && (fwd_rt->flags & RT_UNIDIR)))
            unidir = true;

    if (route && fwd_rt && fwd_rt->state == VALID && !unidir) {
        DEBUG(LOG_DEBUG, 0, "forwarding packets, actualize time outs");
        rt_table_update_route_timeouts(fwd_rt, rev_rt);
        /* When forwarding data, make sure we are sending HELLO messages */
        if (!llfeedback && optimized_hellos)
            hello_start();
        gettimeofday(&this_host.fwd_time, nullptr);
        scheduleNextEvent();
        return INetfilter::IHook::ACCEPT;
    }
    auto ipHeader = dynamicPtrCast<Ipv4Header>(constPtrCast<NetworkHeaderBase>(networkHeader));

    InterfaceEntry * ie;

    if (!sourceAddr.isUnspecified())
        isLocal = isLocalAddress(sourceAddr);

    ie = getInterfaceEntry (ifindex);
    if (ipHeader->getProtocolId() == IP_PROT_TCP)
        rreq_flags |= RREQ_GRATUITOUS;

    /* If this is a TCP packet and we don't have a route, we should
       set the gratuituos flag in the RREQ. */
    bool isMcast = ie->getProtocolData<Ipv4InterfaceData>()->isMemberOfMulticastGroup(dest_addr.s_addr.toIpv4());

    /* If the packet is not interesting we just let it go through... */
    if (isMcast || dest_addr.s_addr == L3Address(Ipv4Address(AODV_BROADCAST)))
    {
        //send(p,"socketOut");
        return INetfilter::IHook::ACCEPT;
    }

#ifdef CONFIG_GATEWAY
    /* Check if we have a route and it is an Internet destination (Should be
     * encapsulated and routed through the gateway). */
    if (fwd_rt && (fwd_rt->state == VALID) &&
            (fwd_rt->flags & RT_INET_DEST))
    {
        /* The destination should be relayed through the IG */

        rt_table_update_timeout(fwd_rt, ACTIVE_ROUTE_TIMEOUT);

        p = pkt_encapsulate(p, *gateWayAddress);

        if (p == nullptr)
        {
            DEBUG(LOG_ERR, 0, "IP Encapsulation failed!");
            return;
        }
        /* Update pointers to headers */
        dest_addr.s_addr = gateWayAddress->getInt();
        fwd_rt = rt_table_find(dest_addr);
    }
#endif /* CONFIG_GATEWAY */

    /* UPDATE TIMERS on active forward and reverse routes...  */
    rt_table_update_route_timeouts(fwd_rt, rev_rt);

    /* OK, the timeouts have been updated. Now see if either: 1. The
       packet is for this node -> ACCEPT. 2. The packet is not for this
       node -> Send RERR (someone want's this node to forward packets
       although there is no route) or Send RREQ. */

    if (!fwd_rt || fwd_rt->state == INVALID ||
            (fwd_rt->hcnt == 1 && (fwd_rt->flags & RT_UNIDIR)))
    {

        /* Check if the route is marked for repair or is INVALID. In
         * that case, do a route discovery. */
        struct in_addr rerr_dest;

        if ((isLocal) || (fwd_rt && (fwd_rt->flags & RT_REPAIR))) {
            /* Buffer packets... Packets are queued by the ip_queue.o
               module already. We only need to save the handle id, and
               return the proper verdict when we know what to do... */
            if (fwd_rt && (fwd_rt->flags & RT_REPAIR))
                rreq_local_repair(fwd_rt, src_add, ipd);
            else
            {
                if (par("targetOnlyRreq").boolValue())
                    rreq_flags |= RREQ_DEST_ONLY;
                rreq_route_discovery(dest_addr, rreq_flags, ipd);
            }
            return INetfilter::IHook::QUEUE;
        }

        Ptr<RERR> rerr;
        DEBUG(LOG_DEBUG, 0,
              "No route, src=%s dest=%s prev_hop=%s - DROPPING!",
              ip_to_str(src_addr), ip_to_str(dest_addr));
        if (fwd_rt)
        {
            rerr = rerr_create(0, fwd_rt->dest_addr,fwd_rt->dest_seqno);
            rt_table_update_timeout(fwd_rt, DELETE_PERIOD);
        }
        else
            rerr = rerr_create(0, dest_addr, 0);
        DEBUG(LOG_DEBUG, 0, "Sending RERR to prev hop %s for unknown dest %s",
              ip_to_str(src_addr), ip_to_str(dest_addr));

        /* Unicast the RERR to the source of the data transmission
         * if possible, otherwise we broadcast it. */

        if (rev_rt && rev_rt->state == VALID)
            rerr_dest = rev_rt->next_hop;
        else
            rerr_dest.s_addr = L3Address(Ipv4Address(AODV_BROADCAST));

        auto pkt = new Packet("Aodv RERR");
        pkt->insertAtFront(rerr);
        aodv_socket_send(pkt, rerr_dest,RERR_CALC_SIZE(rerr),
                         1, &DEV_IFINDEX(ifindex));
        if (wait_on_reboot)
        {
            DEBUG(LOG_DEBUG, 0, "Wait on reboot timer reset.");
            timer_set_timeout(&worb_timer, DELETE_PERIOD);
        }

        //drop (p);
        sendICMP(p);
        /* DEBUG(LOG_DEBUG, 0, "Dropping pkt uid=%d", ch->uid()); */
        //  icmpAccess.get()->sendErrorMessage(p, ICMP_DESTINATION_UNREACHABLE, 0);
        return INetfilter::IHook::DROP;
    }
    else
    {
        if (!route)
            throw cRuntimeError("Aodv-uu inconsistency between ip routing table and aodv routing table");
        /* DEBUG(LOG_DEBUG, 0, "Sending pkt uid=%d", ch->uid()); */
        /* When forwarding data, make sure we are sending HELLO messages */
        gettimeofday(&this_host.fwd_time, nullptr);

        if (!llfeedback && optimized_hellos)
            hello_start();
        return INetfilter::IHook::ACCEPT;
    }
}


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


void NS_CLASS processLinkBreak(const Packet *pkt)
{
    if (!llfeedback)
        return;

    if (!isInMacLayer())
        packetFailed(pkt);
    else
        packetFailedMac(pkt);

}

#if 0
void NS_CLASS processFullPromiscuous(const cObject *details)
{
    if (dynamic_cast<Ieee80211TwoAddressFrame *>(const_cast<cObject*> (details)))
    {
        Ieee80211TwoAddressFrame *frame = dynamic_cast<Ieee80211TwoAddressFrame *>(const_cast<cObject*>(details));
        L3Address sender(frame->getTransmitterAddress());
        struct in_addr destination;
        destination.s_addr = sender;
        rt_table_t * fwd_rt = rt_table_find(destination);
        if (fwd_rt)
            fwd_rt->pending = false;
    }
}

void NS_CLASS processPromiscuous(const cObject *details)
{

    if (dynamic_cast<Ieee80211DataFrame *>(const_cast<cObject*> (details)))
    {
        Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame *>(const_cast<cObject*>(details));
        cPacket * pktAux = frame->getEncapsulatedPacket();
        if (!isInMacLayer() && pktAux != nullptr)
        {
            /*
            if (dynamic_cast<IPv4Datagram *>(pktAux))
            {
                cPacket * pktAux1 = pktAux->getEncapsulatedPacket(); // Transport
                if (pktAux1)
                {
                    cPacket * pktAux2 = pktAux->getEncapsulatedPacket(); // protocol
                    if (pktAux2 && dynamic_cast<RREP *> (pktAux2))
                    {

                    }
                }
            }
            */
        }
        else if (isInMacLayer() && dynamic_cast<Ieee80211MeshFrame *>(frame))
        {
            Ieee80211MeshFrame *meshFrame = dynamic_cast<Ieee80211MeshFrame *>(frame);
            if (meshFrame->getSubType() == ROUTING)
            {
                cPacket * pktAux2 = meshFrame->getEncapsulatedPacket(); // protocol
                if (pktAux2 && dynamic_cast<RREP *> (pktAux2) && checkRrep)
                {
                    RREP* rrep = dynamic_cast<RREP *> (pktAux2);
                    PacketDestOrigin destOrigin(rrep->dest_addr,rrep->orig_addr);
                    auto it = rrepProc.find(destOrigin);
                    if (it == rrepProc.end())
                    {
                        // new
                        RREPProcessed rproc;
                        rproc.cost = rrep->cost;
                        rproc.dest_seqno = rrep->dest_seqno;
                        rproc.hcnt = rrep->hcnt;
                        rproc.hopfix = rrep->hopfix;
                        rproc.totalHops = rrep->totalHops;
                        rproc.next = L3Address(meshFrame->getReceiverAddress());
                    }
                    else if (it->second.dest_seqno < rrep->dest_seqno)
                    {
                        // actualize
                        it->second.cost = rrep->cost;
                        it->second.dest_seqno = rrep->dest_seqno;
                        it->second.hcnt = rrep->hcnt;
                        it->second.hopfix = rrep->hopfix;
                        it->second.next = L3Address(meshFrame->getReceiverAddress());
                    }
                    else if (it->second.dest_seqno == rrep->dest_seqno)
                    {
                        // equal seq num check
                        if (it->second.hcnt > rrep->hcnt)
                        {
                            it->second.cost = rrep->cost;
                            it->second.dest_seqno = rrep->dest_seqno;
                            it->second.hcnt = rrep->hcnt;
                            it->second.hopfix = rrep->hopfix;
                            it->second.next = L3Address(meshFrame->getReceiverAddress());
                        }
                    }
                }
            }
        }
    }
}
#endif

void NS_CLASS finish()
{

    simtime_t t = simTime();
    if (t==0)
        return;
    if (iswrite)
        return;
    iswrite=true;
    recordScalar("simulated time", t);
    recordScalar("Aodv totalSend ", totalSend);
    recordScalar("rreq send", totalRreqSend);
    recordScalar("rreq rec", totalRreqRec);
    recordScalar("rrep send", totalRrepSend);
    recordScalar("rrep rec", totalRrepRec);
    recordScalar("rrep ack send", totalRrepAckSend);
    recordScalar("rrep ack rec", totalRrepAckRec);
    recordScalar("rerr send", totalRerrSend);
    recordScalar("rerr rec", totalRerrRec);
    recordScalar("total repair",totalLocalRep);
}


uint32_t NS_CLASS getRoute(const L3Address &dest,std::vector<L3Address> &add)
{
    return 0;
}


bool  NS_CLASS getNextHop(const L3Address &dest,L3Address &add, int &iface,double &cost)
{
    struct in_addr destAddr;
    destAddr.s_addr = dest;
    L3Address apAddr;
    rt_table_t * fwd_rt = this->rt_table_find(destAddr);
    if (fwd_rt)
    {
        if (fwd_rt->state != VALID)
            return false;
        add = fwd_rt->next_hop.s_addr;
        InterfaceEntry * ie = getInterfaceEntry (fwd_rt->ifindex);
        iface = ie->getInterfaceId();
        cost = fwd_rt->hcnt;
        return true;
    }
    else if (getAp(dest,apAddr))
    {
        destAddr.s_addr = apAddr;
        fwd_rt = this->rt_table_find(destAddr);
        if (!fwd_rt)
            return false;
        if (fwd_rt->state != VALID)
            return false;
        add = fwd_rt->next_hop.s_addr;
        InterfaceEntry * ie = getInterfaceEntry (fwd_rt->ifindex);
        iface = ie->getInterfaceId();
        cost = fwd_rt->hcnt;
        return true;
    }
    return false;
}

bool NS_CLASS isProactive()
{
    return false;
}

void NS_CLASS setRefreshRoute(const L3Address &destination, const L3Address & nextHop,bool isReverse)
{
    struct in_addr dest_addr, next_hop;
    dest_addr.s_addr = destination;
    next_hop.s_addr = nextHop;
    rt_table_t * route  = rt_table_find(dest_addr);

    L3Address apAddr;
    bool gratuitus = false;


    if (getAp(destination,apAddr))
    {
        dest_addr.s_addr = apAddr;
    }
    if (getAp(nextHop,apAddr))
    {
        next_hop.s_addr = apAddr;
    }

    if(par ("checkNextHop").boolValue())
    {
        if (nextHop.isUnspecified())
           return;
        if (!isReverse)
        {
            if (route &&(route->next_hop.s_addr==nextHop))
                 rt_table_update_route_timeouts(route, nullptr);
        }


        if (isReverse && !route && gratuitus)
        {
            // Gratuitous Return Path
            struct in_addr node_addr;
            struct in_addr  ip_src;
            node_addr.s_addr = destination;
            ip_src.s_addr = nextHop;
            rt_table_insert(node_addr, ip_src,0,0, ACTIVE_ROUTE_TIMEOUT, VALID, 0,NS_DEV_NR,0xFFFFFFF,100);
        }
        else if (route && (route->next_hop.s_addr == nextHop))
            rt_table_update_route_timeouts(nullptr, route);

    }
    else
    {
        if (!isReverse)
        {
            if (route)
                 rt_table_update_route_timeouts(route, nullptr);
        }


        if (isReverse && !route && !nextHop.isUnspecified())
        {
            // Gratuitous Return Path
            struct in_addr node_addr;
            struct in_addr  ip_src;
            node_addr.s_addr = destination;
            ip_src.s_addr = nextHop;
            rt_table_insert(node_addr, ip_src,0,0, ACTIVE_ROUTE_TIMEOUT, VALID, 0,NS_DEV_NR,0xFFFFFFF,100);
        }
        else if (route)
            rt_table_update_route_timeouts(nullptr, route);

    }

    Enter_Method_Silent();
    scheduleNextEvent();
}


bool NS_CLASS isOurType(const Packet * msg)
{
    auto pkt = msg->peekAtFront<AODV_msg>();
    if (pkt != nullptr)
        return true;
    return false;
}

bool NS_CLASS getDestAddress(Packet *msg,L3Address &dest)
{
    RREQ *rreq = dynamic_cast <RREQ *>(msg);
    if (!rreq)
        return false;
    dest = rreq->dest_addr;
    return true;

}

bool AODVUU::getDestAddressRreq(Packet *msg,PacketDestOrigin &orgDest,RREQInfo &rreqInfo)
{
    RREQ *rreq = dynamic_cast <RREQ *>(msg);
    if (!rreq)
        return false;
    orgDest.setDests(rreq->dest_addr);
    orgDest.setOrigin(rreq->orig_addr);
    rreqInfo.origin_seqno = rreq->orig_seqno;
    rreqInfo.dest_seqno = rreq->dest_seqno;
    rreqInfo.hcnt = rreq->hcnt;
    rreqInfo.cost = rreq->cost;
    rreqInfo.hopfix = rreq->hopfix;
    return true;
}



#ifdef AODV_USE_STL_RT
bool  NS_CLASS setRoute(const L3Address &dest,const L3Address &add, const int &ifaceIndex,const int &hops,const L3Address &mask)
{
    Enter_Method_Silent();
    struct in_addr destAddr;
    struct in_addr nextAddr;
    struct in_addr rerr_dest;
    destAddr.s_addr = dest;
    nextAddr.s_addr = add;
    bool status=true;
    bool delEntry = add.isUnspecified();

    DEBUG(LOG_DEBUG, 0, "setRoute %s next hop %s",ip_to_str(destAddr),ip_to_str(nextAddr));

    rt_table_t * fwd_rt = rt_table_find(destAddr);

    if (fwd_rt)
    {
        if (delEntry)
        {
            auto rerr = rerr_create(0, destAddr, 0);
            DEBUG(LOG_DEBUG, 0, "setRoute Sending for unknown dest %s", ip_to_str(destAddr));

            /* Unicast the RERR to the source of the data transmission
             * if possible, otherwise we broadcast it. */
            rerr_dest.s_addr = L3Address(Ipv4Address(AODV_BROADCAST));

            auto pkt = new Packet("Aodv RERR");
            pkt->insertAtFront(rerr);
            aodv_socket_send(pkt, rerr_dest,RERR_CALC_SIZE(rerr),
                             1, &DEV_IFINDEX(NS_IFINDEX));
        }
        L3Address dest = fwd_rt->dest_addr.s_addr;
        auto it = aodvRtTableMap.find(dest);
        if (it != aodvRtTableMap.end())
        {
            if (it->second != fwd_rt)
                throw cRuntimeError("AODV routing table error");
        }
        aodvRtTableMap.erase(it);
        if (fwd_rt->state == VALID || fwd_rt->state == IMMORTAL)
            rt_tbl.num_active--;
        timer_remove(&fwd_rt->rt_timer);
        timer_remove(&fwd_rt->hello_timer);
        timer_remove(&fwd_rt->ack_timer);
        rt_tbl.num_entries = aodvRtTableMap.size();
        free (fwd_rt);
    }
    else
        DEBUG(LOG_DEBUG, 0, "No route entry to delete");

    if (ifaceIndex>=getNumInterfaces())
        status = false;
    ManetRoutingBase::setRoute(dest,add,ifaceIndex,hops,mask);

    if (!delEntry && ifaceIndex<getNumInterfaces())
    {
        fwd_rt = modifyAODVTables(destAddr,nextAddr,hops,(uint32_t) SIMTIME_DBL(simTime()), 0xFFFF,IMMORTAL,0, ifaceIndex);
        status = (fwd_rt!=nullptr);

    }

    return status;
}


bool  NS_CLASS setRoute(const L3Address &dest,const L3Address &add, const char  *ifaceName,const int &hops,const L3Address &mask)
{
    Enter_Method_Silent();
    struct in_addr destAddr;
    struct in_addr nextAddr;
    struct in_addr rerr_dest;
    destAddr.s_addr = dest;
    nextAddr.s_addr = add;
    bool status=true;
    int index;
    bool delEntry = add.isUnspecified();

    DEBUG(LOG_DEBUG, 0, "setRoute %s next hop %s",ip_to_str(destAddr),ip_to_str(nextAddr));
    rt_table_t * fwd_rt = rt_table_find(destAddr);

    if (fwd_rt)
    {
        if (delEntry)
        {
            auto rerr = rerr_create(0, destAddr, 0);
            DEBUG(LOG_DEBUG, 0, "setRoute Sending for unknown dest %s", ip_to_str(destAddr));

            /* Unicast the RERR to the source of the data transmission
             * if possible, otherwise we broadcast it. */
            rerr_dest.s_addr = L3Address(Ipv4Address(AODV_BROADCAST));
            auto pkt = new Packet("Aodv RERR");
            pkt->insertAtFront(rerr);
            aodv_socket_send(pkt, rerr_dest,RERR_CALC_SIZE(rerr),
                             1, &DEV_IFINDEX(NS_IFINDEX));
        }
        L3Address dest = fwd_rt->dest_addr.s_addr;
        auto it = aodvRtTableMap.find(dest);
        if (it != aodvRtTableMap.end())
        {
            if (it->second != fwd_rt)
                throw cRuntimeError("AODV routing table error");
        }
        aodvRtTableMap.erase(it);
        if (fwd_rt->state == VALID || fwd_rt->state == IMMORTAL)
            rt_tbl.num_active--;
        timer_remove(&fwd_rt->rt_timer);
        timer_remove(&fwd_rt->hello_timer);
        timer_remove(&fwd_rt->ack_timer);
        rt_tbl.num_entries = aodvRtTableMap.size();
        free (fwd_rt);
    }
    else
        DEBUG(LOG_DEBUG, 0, "No route entry to delete");

    for (index = 0; index <getNumInterfaces(); index++)
    {
        auto ie = getInterfaceEntry(index);
        if (strcmp(ifaceName, ie->getInterfaceName())==0) break;
    }
    if (index>=getNumInterfaces())
        status = false;

    ManetRoutingBase::setRoute(dest,add,index,hops,mask);

    if (!delEntry && index<getNumInterfaces())
    {
        fwd_rt = modifyAODVTables(destAddr,nextAddr,hops,(uint32_t) SIMTIME_DBL(simTime()), 0xFFFF,IMMORTAL,0, index);
        status = (fwd_rt!=nullptr);
    }


    return status;
}
#else

bool  NS_CLASS setRoute(const L3Address &dest,const L3Address &add, const int &ifaceIndex,const int &hops,const L3Address &mask)
{
    Enter_Method_Silent();
    struct in_addr destAddr;
    struct in_addr nextAddr;
    struct in_addr rerr_dest;
    destAddr.s_addr = dest;
    nextAddr.s_addr = add;
    bool status=true;
    bool delEntry = (add == (L3Address)0);

    DEBUG(LOG_DEBUG, 0, "setRoute %s next hop %s",ip_to_str(destAddr),ip_to_str(nextAddr));

    rt_table_t * fwd_rt = rt_table_find(destAddr);

    if (fwd_rt)
    {
        if (delEntry)
        {
            RERR* rerr = rerr_create(0, destAddr, 0);
            DEBUG(LOG_DEBUG, 0, "setRoute Sending for unknown dest %s", ip_to_str(destAddr));

            /* Unicast the RERR to the source of the data transmission
             * if possible, otherwise we broadcast it. */
            rerr_dest.s_addr = AODV_BROADCAST;

            aodv_socket_send((AODV_msg *) rerr, rerr_dest,RERR_CALC_SIZE(rerr),
                             1, &DEV_IFINDEX(NS_IFINDEX));
        }
        list_detach(&fwd_rt->l);
        precursor_list_destroy(fwd_rt);
        if (fwd_rt->state == VALID || fwd_rt->state == IMMORTAL)
            rt_tbl.num_active--;
        timer_remove(&fwd_rt->rt_timer);
        timer_remove(&fwd_rt->hello_timer);
        timer_remove(&fwd_rt->ack_timer);
        rt_tbl.num_entries--;
        free (fwd_rt);
    }
    else
        DEBUG(LOG_DEBUG, 0, "No route entry to delete");

    if (ifaceIndex>=getNumInterfaces())
        status = false;
    ManetRoutingBase::setRoute(dest,add,ifaceIndex,hops,mask);

    if (!delEntry && ifaceIndex<getNumInterfaces())
    {
        fwd_rt = modifyAODVTables(destAddr,nextAddr,hops,(uint32_t) SIMTIME_DBL(simTime()), 0xFFFF,IMMORTAL,0, ifaceIndex);
        status = (fwd_rt!=nullptr);

    }

    return status;
}

bool  NS_CLASS setRoute(const L3Address &dest,const L3Address &add, const char  *ifaceName,const int &hops,const L3Address &mask)
{
    Enter_Method_Silent();
    struct in_addr destAddr;
    struct in_addr nextAddr;
    struct in_addr rerr_dest;
    destAddr.s_addr = dest;
    nextAddr.s_addr = add;
    bool status=true;
    int index;
    bool delEntry = (add == (L3Address)0);

    DEBUG(LOG_DEBUG, 0, "setRoute %s next hop %s",ip_to_str(destAddr),ip_to_str(nextAddr));
    rt_table_t * fwd_rt = rt_table_find(destAddr);

    if (fwd_rt)
    {
        if (delEntry)
        {
            RERR* rerr = rerr_create(0, destAddr, 0);
            DEBUG(LOG_DEBUG, 0, "setRoute Sending for unknown dest %s", ip_to_str(destAddr));

            /* Unicast the RERR to the source of the data transmission
             * if possible, otherwise we broadcast it. */
            rerr_dest.s_addr = AODV_BROADCAST;

            aodv_socket_send((AODV_msg *) rerr, rerr_dest,RERR_CALC_SIZE(rerr),
                             1, &DEV_IFINDEX(NS_IFINDEX));
        }
        list_detach(&fwd_rt->l);
        precursor_list_destroy(fwd_rt);
        if (fwd_rt->state == VALID || fwd_rt->state == IMMORTAL)
            rt_tbl.num_active--;
        timer_remove(&fwd_rt->rt_timer);
        timer_remove(&fwd_rt->hello_timer);
        timer_remove(&fwd_rt->ack_timer);
        rt_tbl.num_entries--;
        free (fwd_rt);
    }
    else
        DEBUG(LOG_DEBUG, 0, "No route entry to delete");

    for (index = 0; index <getNumInterfaces(); index++)
    {
        if (strcmp(ifaceName, getInterfaceEntry(index)->getName())==0) break;
    }
    if (index>=getNumInterfaces())
        status = false;

    ManetRoutingBase::setRoute(dest,add,index,hops,mask);

    if (!delEntry && index<getNumInterfaces())
    {
        fwd_rt = modifyAODVTables(destAddr,nextAddr,hops,(uint32_t) SIMTIME_DBL(simTime()), 0xFFFF,IMMORTAL,0, index);
        status = (fwd_rt!=nullptr);
    }


    return status;
}
#endif

bool NS_CLASS isThisRrepPrevSent(Ptr<const AODV_msg> msg)
{
    if (!checkRrep)
        return false;
    auto rrep = dynamicPtrCast<RREP>(constPtrCast<AODV_msg>(msg));
    if (rrep->hcnt == 0)
            return false; // this packet had this node like destination, in this case the node must send the packet

    if (rrep == nullptr)
         return false; // no information, send

    PacketDestOrigin destOrigin(rrep->dest_addr,rrep->orig_addr);
    auto it = rrepProc.find(destOrigin);
    if (it != rrepProc.end()) // only send if the seq num is bigger
    {
        if (it->second.dest_seqno > rrep->dest_seqno)
        {
            return true;
        }
        else if (it->second.dest_seqno == rrep->dest_seqno && it->second.totalHops < rrep->totalHops)
        {
            return true;
        }
    }
    return false;
}

void NS_CLASS actualizeTablesWithCollaborative(const L3Address &dest)
{
    if (!getCollaborativeProtocol())
        return;

    struct in_addr next_hop,destination;
    int iface;
    double cost;

    if (getCollaborativeProtocol()->getNextHop(dest,next_hop.s_addr,iface,cost))
    {
        u_int8_t hops = cost;
        destination.s_addr = dest;
        auto it =  mapSeqNum.find(dest);
        if (it == mapSeqNum.end())
            throw cRuntimeError("node not found in mapSeqNum");
        uint32_t sqnum = *(it->second);
        uint32_t life = PATH_DISCOVERY_TIME - 2 * hops * NODE_TRAVERSAL_TIME;
        int ifindex = -1;

        rt_table_t * fwd_rt = rt_table_find(destination);

        for (int i = 0; i < getNumInterfaces(); i++)
        {
            if (getInterfaceEntry(i)->getInterfaceId() == iface)
            {
                ifindex = i;
                break;
            }
        }

        if (ifindex == -1)
            throw cRuntimeError("interface not found");

        if (fwd_rt)
            fwd_rt = rt_table_update(fwd_rt, next_hop, hops, sqnum, life, VALID, fwd_rt->flags,ifindex, cost, cost+1);
        else
            fwd_rt = rt_table_insert(destination, next_hop, hops, sqnum, life, VALID, 0, ifindex, cost, cost+1);

        hops = 1;
        rt_table_t * fwd_rtAux = rt_table_find(next_hop);
        it =  mapSeqNum.find(next_hop.s_addr);
        if (it == mapSeqNum.end())
            throw cRuntimeError("node not found in mapSeqNum");
        sqnum = *(it->second);
        life = PATH_DISCOVERY_TIME - 2 * (int)hops * NODE_TRAVERSAL_TIME;
        if (fwd_rtAux)
            fwd_rtAux = rt_table_update(fwd_rtAux, next_hop, hops, sqnum, life, VALID, fwd_rtAux->flags,ifindex, hops, hops+1);
        else
            fwd_rtAux = rt_table_insert(next_hop, next_hop, hops, sqnum, life, VALID, 0, ifindex, hops, hops+1);
    }
}


bool NS_CLASS handleNodeStart(IDoneCallback *doneCallback)
{
    if (isRoot)
    {
        timer_init(&proactive_rreq_timer,&NS_CLASS rreq_proactive, nullptr);
        timer_set_timeout(&proactive_rreq_timer, par("startRreqProactive").intValue());
    }

    propagateProactive = par("propagateProactive");

    aodv_socket_init();
    rt_table_init();
    packet_queue_init();
    startAODVUUAgent();
    return true;
}

bool NS_CLASS handleNodeShutdown(IDoneCallback *doneCallback)
{

    while (!aodvRtTableMap.empty())
    {
        free (aodvRtTableMap.begin()->second);
        aodvRtTableMap.erase(aodvRtTableMap.begin());
    }
    while (!rreq_records.empty())
    {
        free (rreq_records.back());
        rreq_records.pop_back();
    }
    while (!rreq_blacklist.empty())
    {
        free (rreq_blacklist.begin()->second);
        rreq_blacklist.erase(rreq_blacklist.begin());
    }

    while (!seekhead.empty())
    {
        delete (seekhead.begin()->second);
        seekhead.erase(seekhead.begin());
    }
    aodvTimerMap.clear();
    packet_queue_destroy();
    cancelEvent(sendMessageEvent);
    log_cleanup();
    return true;
}

void NS_CLASS handleNodeCrash()
{
    while (!aodvRtTableMap.empty())
    {
        free (aodvRtTableMap.begin()->second);
        aodvRtTableMap.erase(aodvRtTableMap.begin());
    }
    while (!rreq_records.empty())
    {
        free (rreq_records.back());
        rreq_records.pop_back();
    }
    while (!rreq_blacklist.empty())
    {
        free (rreq_blacklist.begin()->second);
        rreq_blacklist.erase(rreq_blacklist.begin());
    }

    while (!seekhead.empty())
    {
        delete (seekhead.begin()->second);
        seekhead.erase(seekhead.begin());
    }
    aodvTimerMap.clear();
    packet_queue_destroy();
    cancelEvent(sendMessageEvent);
    log_cleanup();
}

} // namespace inetmanet

} // namespace inet
