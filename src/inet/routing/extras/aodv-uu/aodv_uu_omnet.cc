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


#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/ipv4/ICMPMessage_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

#include "inet/common/ProtocolMap.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/extras/base/ControlManetRouting_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/routing/extras/aodv-uu/aodv_uu_omnet.h"

namespace inet {

using namespace ieee80211;

namespace inetmanet {

typedef std::vector<IPv4Address> IPAddressVector;

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
#endif

void NS_CLASS initialize(int stage)
{
     /*
       Enable usage of some of the configuration variables from Tcl.

       Note: Do NOT change the values of these variables in the constructor
       after binding them! The desired default values should be set in
       ~ns/tcl/lib/ns-default.tcl instead.
     */
    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
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
        gateWayAddress = new IPv4Address(par("internet_gw_address").stringValue());

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

        /* Initialize common manet routing protocol structures */
        registerRoutingModule();
        if (llfeedback)
            linkLayerFeeback();

        /* From main.c */
        progname = "AODV-UU";
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
            strcpy(DEV_NR(i).ifname, getInterfaceEntry(i)->getName());
            if (!isInMacLayer())
            {
                DEV_NR(i).netmask.s_addr =
                    L3Address(getInterfaceEntry(i)->ipv4Data()->getIPAddress().getNetworkMask());
                DEV_NR(i).ipaddr.s_addr =
                        L3Address(getInterfaceEntry(i)->ipv4Data()->getIPAddress());
            }
            else
            {
                DEV_NR(i).netmask.s_addr = L3Address(MACAddress::BROADCAST_ADDRESS);
                DEV_NR(i).ipaddr.s_addr = L3Address(getInterfaceEntry(i)->getMacAddress());

            }
            if (getInterfaceEntry(i)->isLoopback())
                continue;
        }
        /* Set network interface parameters */
        for (int i=0; i < getNumWlanInterfaces(); i++)
        {
            DEV_NR(getWlanInterfaceIndex(i)).enabled = 1;
            DEV_NR(getWlanInterfaceIndex(i)).sock = -1;
            DEV_NR(getWlanInterfaceIndex(i)).broadcast.s_addr = L3Address(IPv4Address(AODV_BROADCAST));
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
        proactive_rreq_timeout= par("proactiveRreqTimeout").longValue();

        if (isRoot)
        {
            timer_init(&proactive_rreq_timer,&NS_CLASS rreq_proactive, nullptr);
            timer_set_timeout(&proactive_rreq_timer, par("startRreqProactive").longValue());
        }

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
NS_CLASS ~ AODVUU()
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
void NS_CLASS packetFailed(IPv4Datagram *dgram)
{
    rt_table_t *rt_next_hop, *rt;
    struct in_addr dest_addr, src_addr, next_hop;

    src_addr.s_addr = L3Address(dgram->getSrcAddress());
    dest_addr.s_addr = L3Address(dgram->getDestAddress());


    DEBUG(LOG_DEBUG, 0, "Got failure callback");
    /* We don't care about link failures for broadcast or non-data packets */
    if (dgram->getDestAddress().getInt() == IP_BROADCAST ||
            dgram->getDestAddress().getInt() == AODV_BROADCAST)
    {
        DEBUG(LOG_DEBUG, 0, "Ignoring callback");
        scheduleNextEvent();
        return;
    }


    DEBUG(LOG_DEBUG, 0, "LINK FAILURE for next_hop=%s dest=%s ",ip_to_str(next_hop), ip_to_str(dest_addr));

    if (seek_list_find(dest_addr))
    {
        DEBUG(LOG_DEBUG, 0, "Ongoing route discovery, buffering packet...");
        packet_queue_add((IPv4Datagram *)dgram->dup(), dest_addr);
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
        packet_queue_add((IPv4Datagram *) dgram->dup(), dest_addr);

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
void NS_CLASS packetFailedMac(Ieee80211DataFrame *dgram)
{
    rt_table_t *rt_next_hop, *rt;
    struct in_addr dest_addr, src_addr, next_hop;
    if (dgram->getReceiverAddress().isBroadcast())
    {
        scheduleNextEvent();
        return;
    }

    src_addr.s_addr = L3Address(dgram->getAddress3());
    dest_addr.s_addr = L3Address(dgram->getAddress4());
    if (seek_list_find(dest_addr))
    {
        DEBUG(LOG_DEBUG, 0, "Ongoing route discovery, buffering packet...");
        packet_queue_add(dgram->dup(), dest_addr);
        scheduleNextEvent();
        return;
    }

    next_hop.s_addr = L3Address(dgram->getReceiverAddress());
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



/* Entry-level packet reception */
void NS_CLASS handleMessage (cMessage *msg)
{
    AODV_msg *aodvMsg=nullptr;
    IPv4Datagram * ipDgram=nullptr;
    UDPPacket * udpPacket=nullptr;

    cMessage *msg_aux;
    struct in_addr src_addr;
    struct in_addr dest_addr;

    if (is_init==false)
        throw cRuntimeError("Aodv has not been initialized ");
    if (msg==sendMessageEvent)
    {
        // timer event
        scheduleNextEvent();
        return;
    }
    /* Handle packet depending on type */
    if (dynamic_cast<ControlManetRouting *>(msg))
    {
        ControlManetRouting * control =  check_and_cast <ControlManetRouting *> (msg);
        if (control->getOptionCode()== MANET_ROUTE_NOROUTE)
        {
            if (isInMacLayer())
            {
                if (control->getDestAddress().isBroadcast())
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
                        rt_table_t * fwd_rt = rt_table_find(dest_addr);

                        RERR *rerr;
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
                        struct in_addr src_addr;
                        src_addr.s_addr = control->getSrcAddress();
                        rt_table_t * rev_rt = rt_table_find(src_addr);

                        struct in_addr rerr_dest;

                        if (rev_rt && rev_rt->state == VALID)
                            rerr_dest = rev_rt->next_hop;
                        else
                            rerr_dest.s_addr = L3Address(IPv4Address(AODV_BROADCAST));

                        aodv_socket_send((AODV_msg *) rerr, rerr_dest,RERR_CALC_SIZE(rerr), 1, &DEV_IFINDEX(NS_IFINDEX));
                    }
                }
            }
            else
            {
                ipDgram = (IPv4Datagram*) control->decapsulate();
                cObject * ctrl = ipDgram->removeControlInfo();
                unsigned int ifindex = NS_IFINDEX;  /* Always use ns interface */
                if (ctrl)
                {
                    if (dynamic_cast<Ieee802Ctrl*> (ctrl))
                    {
                        Ieee802Ctrl *ieeectrl = static_cast<Ieee802Ctrl*> (ctrl);
                        L3Address address(ieeectrl->getDest());
                        int index = getWlanInterfaceIndexByAddress(address);
                        if (index!=-1)
                            ifindex = index;
                    }
                    delete ctrl;
                }
                EV_INFO << "Aodv rec datagram  " << ipDgram->getName() << " with dest=" << ipDgram->getDestAddress().str() << "\n";
                processPacket(ipDgram,ifindex);   // Data path
            }
        }
        else if (control->getOptionCode()== MANET_ROUTE_UPDATE)
        {
            DEBUG(LOG_DEBUG, 0, "forwarding packers, actualize time outs");
            src_addr.s_addr = control->getSrcAddress();
            dest_addr.s_addr = control->getDestAddress();
            rt_table_t * fwd_rt = rt_table_find(dest_addr);
            rt_table_t * rev_rt = rt_table_find(src_addr);
            rt_table_update_route_timeouts(fwd_rt, rev_rt);
            /* When forwarding data, make sure we are sending HELLO messages */
            gettimeofday(&this_host.fwd_time, nullptr);
        }
        delete msg;
        scheduleNextEvent();
        return;
    }
    else if (dynamic_cast<UDPPacket *>(msg) || dynamic_cast<AODV_msg  *>(msg))
    {
        udpPacket = nullptr;
        if (!isInMacLayer())
        {
            udpPacket = check_and_cast<UDPPacket*>(msg);
            if (udpPacket->getDestinationPort()!= 654)
            {
                delete  msg;
                scheduleNextEvent();
                return;
            }
            msg_aux  = udpPacket->decapsulate();
        }
        else
            msg_aux = msg;

        if (dynamic_cast<AODV_msg *>(msg_aux))
        {
            aodvMsg = check_and_cast<AODV_msg *>(msg_aux);
            if (!isInMacLayer())
            {
                INetworkProtocolControlInfo *controlInfo = check_and_cast<INetworkProtocolControlInfo*>(udpPacket->removeControlInfo());
                src_addr.s_addr = controlInfo->getSourceAddress();
                aodvMsg->setControlInfo(check_and_cast<cObject *>(controlInfo));
            }
            else
            {
                Ieee802Ctrl *controlInfo = check_and_cast<Ieee802Ctrl*>(aodvMsg->getControlInfo());
                src_addr.s_addr = L3Address(controlInfo->getSrc());
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
        delete aodvMsg;
        aodvMsg=nullptr;
        scheduleNextEvent();
        return;
    }
    recvAODVUUPacket(aodvMsg);
    scheduleNextEvent();
}
/*
      case PT_ENCAPSULATED:
    // Decapsulate...
    if (internet_gw_mode) {
        rt_table_t *rev_rt, *next_hop_rt;
         rev_rt = rt_table_find(saddr);

         if (rev_rt && rev_rt->state == VALID) {
         rt_table_update_timeout(rev_rt, ACTIVE_ROUTE_TIMEOUT);

         next_hop_rt = rt_table_find(rev_rt->next_hop);

         if (next_hop_rt && next_hop_rt->state == VALID &&
             rev_rt && next_hop_rt->dest_addr.s_addr != rev_rt->dest_addr.s_addr)
             rt_table_update_timeout(next_hop_rt, ACTIVE_ROUTE_TIMEOUT);
         }
         p = pkt_decapsulate(p);

         target_->recv(p, (Handler *)0);
         break;
    }

    processPacket(p);   // Data path
    }
*/



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
IPv4Datagram * NS_CLASS pkt_encapsulate(IPv4Datagram *p, IPv4Address gateway)
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
    datagram->setDontFragment (p->getDontFragment());
    datagram->setFragmentOffset(0);
    datagram->setTimeToLive(
        p->getTimeToLive() > 0 ?
        p->getTimeToLive() :
        0);

    datagram->setTransportProtocol(IP_PROT_IP);
    return datagram;
}



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



/*
  Reschedules the timer queue timer to go off at the time of the
  earliest event (so that the timer queue will be investigated then).
  Should be called whenever something might have changed the timer queue.
*/
#ifdef AODV_USE_STL
void NS_CLASS scheduleNextEvent()
{
    simtime_t timer;
    simtime_t timeout = timer_age_queue(); (void)timeout; // UNUSED

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
    return ie->getName();
}



void NS_CLASS recvAODVUUPacket(cMessage * msg)
{
    struct in_addr src, dst;
    int ttl;
    int interfaceId;

    AODV_msg *aodv_msg = check_and_cast<AODV_msg *> (msg);
    int len = aodv_msg->getByteLength();
    int ifIndex = NS_IFINDEX;

    ttl =  aodv_msg->ttl-1;
    if (!isInMacLayer())
    {
        INetworkProtocolControlInfo *ctrl = check_and_cast<INetworkProtocolControlInfo *>(msg->getControlInfo());
        L3Address srcAddr = ctrl->getSourceAddress();
        L3Address destAddr = ctrl->getDestinationAddress();

        src.s_addr = srcAddr;
        dst.s_addr =  destAddr;
        interfaceId = ctrl->getInterfaceId();

    }
    else
    {
        Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
        src.s_addr = L3Address(ctrl->getSrc());
        dst.s_addr =  L3Address(ctrl->getDest());
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

    aodv_socket_process_packet(aodv_msg, len, src, dst, ttl, ifIndex);
    delete   aodv_msg;
}


void NS_CLASS processMacPacket(cPacket * p, const L3Address &dest, const L3Address &src, int ifindex)
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

            RERR *rerr;
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
                rerr_dest.s_addr = L3Address(IPv4Address(AODV_BROADCAST));
            aodv_socket_send((AODV_msg *) rerr, rerr_dest,RERR_CALC_SIZE(rerr),
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
            Ieee802Ctrl *ctrl = new Ieee802Ctrl();
            ctrl->setDest(fwd_rt->next_hop.s_addr.toMAC());
            //TODO ctrl->setEtherType(...);
            p->setControlInfo(ctrl);
        }

        send(p, "to_ip");
        /* When forwarding data, make sure we are sending HELLO messages */
        //gettimeofday(&this_host.fwd_time, nullptr);
        if (!llfeedback && optimized_hellos)
            hello_start();
    }
}


void NS_CLASS processPacket(IPv4Datagram * p,unsigned int ifindex)
{
    rt_table_t *fwd_rt, *rev_rt;
    struct in_addr dest_addr, src_addr;
    u_int8_t rreq_flags = 0;
    struct ip_data *ipd = nullptr;


    fwd_rt = nullptr;      /* For broadcast we provide no next hop */
    ipd = nullptr;         /* No ICMP messaging */

    bool isLocal=true;

    src_addr.s_addr = L3Address(p->getSrcAddress());
    dest_addr.s_addr = L3Address(p->getDestAddress());

    InterfaceEntry *   ie;

    if (!p->getSrcAddress().isUnspecified())
    {
        isLocal = isLocalAddress(L3Address(p->getSrcAddress()));
    }

    ie = getInterfaceEntry (ifindex);
    if (p->getTransportProtocol()==IP_PROT_TCP)
        rreq_flags |= RREQ_GRATUITOUS;

    /* If this is a TCP packet and we don't have a route, we should
       set the gratuituos flag in the RREQ. */
    bool isMcast = ie->ipv4Data()->isMemberOfMulticastGroup(dest_addr.s_addr.toIPv4());

    /* If the packet is not interesting we just let it go through... */
    if (isMcast || dest_addr.s_addr == L3Address(IPv4Address(AODV_BROADCAST)))
    {
        send(p,"to_ip");
        return;
    }
    /* Find the entry of the neighboring node and the destination  (if any). */
    rev_rt = rt_table_find(src_addr);
    fwd_rt = rt_table_find(dest_addr);

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

        if (isLocal)
            goto route_discovery;

        if (fwd_rt && (fwd_rt->flags & RT_REPAIR))
            goto route_discovery;



        RERR *rerr;
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
            rerr_dest.s_addr = L3Address(IPv4Address(AODV_BROADCAST));

        aodv_socket_send((AODV_msg *) rerr, rerr_dest,RERR_CALC_SIZE(rerr),
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

route_discovery:
        /* Buffer packets... Packets are queued by the ip_queue.o
           module already. We only need to save the handle id, and
           return the proper verdict when we know what to do... */

        packet_queue_add(p, dest_addr);

        if (fwd_rt && (fwd_rt->flags & RT_REPAIR))
            rreq_local_repair(fwd_rt, src_addr, ipd);
        else
        {
            if (par("targetOnlyRreq").boolValue())
                rreq_flags |= RREQ_DEST_ONLY;
            rreq_route_discovery(dest_addr, rreq_flags, ipd);
        }

        return;

    }
    else
    {
        /* DEBUG(LOG_DEBUG, 0, "Sending pkt uid=%d", ch->uid()); */
        send(p,"to_ip");
        /* When forwarding data, make sure we are sending HELLO messages */
        gettimeofday(&this_host.fwd_time, nullptr);

        if (!llfeedback && optimized_hellos)
            hello_start();
    }
}


struct dev_info NS_CLASS dev_ifindex (int ifindex)
{
    int index = ifindex2devindex(ifindex);
    ASSERT(index >= 0);
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


void NS_CLASS processLinkBreak(const cObject *details)
{
    if (llfeedback)
    {
        if (dynamic_cast<IPv4Datagram *>(const_cast<cObject*> (details)))
        {
            IPv4Datagram  *dgram = static_cast<IPv4Datagram *>(const_cast<cObject*>(details));
            packetFailed(dgram);
        }
        else if (dynamic_cast<Ieee80211DataFrame *>(const_cast<cObject*> (details)))
        {
            Ieee80211DataFrame *frame = static_cast<Ieee80211DataFrame *>(const_cast<cObject*>(details));
            packetFailedMac(frame);
        }
    }
}


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


bool NS_CLASS isOurType(cPacket * msg)
{
    AODV_msg *re = dynamic_cast <AODV_msg *>(msg);
    if (re)
        return true;
    return false;
}

bool NS_CLASS getDestAddress(cPacket *msg,L3Address &dest)
{
    RREQ *rreq = dynamic_cast <RREQ *>(msg);
    if (!rreq)
        return false;
    dest = rreq->dest_addr;
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
            RERR* rerr = rerr_create(0, destAddr, 0);
            DEBUG(LOG_DEBUG, 0, "setRoute Sending for unknown dest %s", ip_to_str(destAddr));

            /* Unicast the RERR to the source of the data transmission
             * if possible, otherwise we broadcast it. */
            rerr_dest.s_addr = L3Address(IPv4Address(AODV_BROADCAST));

            aodv_socket_send((AODV_msg *) rerr, rerr_dest,RERR_CALC_SIZE(rerr),
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
            RERR* rerr = rerr_create(0, destAddr, 0);
            DEBUG(LOG_DEBUG, 0, "setRoute Sending for unknown dest %s", ip_to_str(destAddr));

            /* Unicast the RERR to the source of the data transmission
             * if possible, otherwise we broadcast it. */
            rerr_dest.s_addr = L3Address(IPv4Address(AODV_BROADCAST));

            aodv_socket_send((AODV_msg *) rerr, rerr_dest,RERR_CALC_SIZE(rerr),
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

void NS_CLASS actualizeTablesWithCollaborative(const L3Address &dest)
{
   return;
}

} // namespace inetmanet

} // namespace inet

