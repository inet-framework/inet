/*
  */
#include "inet/routing/extras/batman/BatmanMain.h"

#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"

namespace inet {

namespace inetmanet {

Define_Module(Batman);

std::ostream& operator<<(std::ostream& os, const OrigNode& e)
{
    os << e.info();
    return os;
};

std::ostream& operator<<(std::ostream& os, const NeighNode& e)
{
    os << e.info();
    return os;
};


Batman::Batman()
{
    debug_level = 0;
    debug_level_max = 4;
    gateway_class = 0;
    routing_class = 0;
    originator_interval = 1;
    debug_timeout = 0;
    pref_gateway = L3Address();

    nat_tool_avail = 0;
    disable_client_nat = 0;

    curr_gateway = nullptr;

    found_ifs = 0;
    active_ifs = 0;
    receive_max_sock = 0;

    unix_client = 0;
    log_facility_active = 0;

    origMap.clear();

    if_list.clear();
    gw_list.clear();
    forw_list.clear();
    // struct vis_if vis_if;


    tunnel_running = 0;

    hop_penalty = TQ_HOP_PENALTY;
    purge_timeout = PURGE_TIMEOUT;
    minimum_send = TQ_LOCAL_BIDRECT_SEND_MINIMUM;
    minimum_recv = TQ_LOCAL_BIDRECT_RECV_MINIMUM;
    global_win_size = TQ_GLOBAL_WINDOW_SIZE;
    local_win_size = TQ_LOCAL_WINDOW_SIZE;
    num_words = (TQ_LOCAL_WINDOW_SIZE / WORD_BIT_SIZE);
    aggregation_enabled = true;
    timer = nullptr;

    hna_list.clear();
    hna_chg_list.clear();
    hnaMap.clear();
    hna_buff_local.clear();
}

Batman::~Batman()
{
    while (!origMap.empty())
    {
        delete origMap.begin()->second;
        origMap.erase(origMap.begin());
    }
    while (!if_list.empty())
    {
        delete if_list.back();
        if_list.pop_back();
    }
    while (!gw_list.empty())
    {
        delete gw_list.back();
        gw_list.pop_back();
    }
    while (!forw_list.empty())
    {
        delete forw_list.back()->pack_buff;
        delete forw_list.back();
        forw_list.pop_back();
    }
    cancelAndDelete(timer);
    while (!hnaMap.empty())
    {
        delete hnaMap.begin()->second;
        hnaMap.erase(hnaMap.begin());
    }
    hna_list.clear();
    hna_buff_local.clear();
    hna_chg_list.clear();
}

void Batman::initialize(int stage)
{
    ManetRoutingBase::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        found_ifs = 0;

        debug_level = par("debugLevel");
        if (debug_level > debug_level_max) {
                throw cRuntimeError( "Invalid debug level: %i\nDebug level has to be between 0 and %i.\n", debug_level, debug_level_max );
        }
        purge_timeout = par("purgeTimeout");
        if (purge_timeout <= SIMTIME_ZERO)
            throw cRuntimeError("Invalid 'purgeTimeout' parameter");

        originator_interval = par("originatorInterval");

        if (originator_interval < 0.001)
            throw cRuntimeError("Invalid 'originatorInterval' parameter");

        routing_class = par("routingClass");
        if (routing_class < 0)
            throw cRuntimeError("Invalid 'routingClass' parameter");

        aggregation_enabled = par("aggregationEnable").boolValue();
        disable_client_nat = 1;

        MAX_AGGREGATION_BYTES = par("MAX_AGGREGATION_BYTES");

        int32_t download_speed = 0, upload_speed = 0;

        registerRoutingModule();
        //createTimerQueue();

        const char *preferedGateWay = par("preferedGateWay");
        pref_gateway =  L3AddressResolver().resolve(preferedGateWay, L3AddressResolver::ADDR_IPv4);

        /*
        IPv4Address vis = par("visualizationServer");

        if (!vis.isUnspecified())
        {
            vis_server = vis.getInt();
        }
        */

        download_speed = par("GWClass_download_speed");
        upload_speed = par("GWClass_upload_speed");

        if ((download_speed > 0) && (upload_speed == 0))
            upload_speed = download_speed / 5;

        if (download_speed > 0) {
            gateway_class = get_gw_class(download_speed, upload_speed);
            get_gw_speeds(gateway_class, &download_speed, &upload_speed);
        }

        if ((gateway_class != 0) && (routing_class != 0)) {
            throw cRuntimeError("Error - routing class can't be set while gateway class is in use !\n");
        }

        if ((gateway_class != 0) && (!pref_gateway.isUnspecified())) {
            throw cRuntimeError("Error - preferred gateway can't be set while gateway class is in use !\n" );
        }

        /* use routing class 1 if none specified */
        if ((routing_class == 0) && (!pref_gateway.isUnspecified()))
            routing_class = DEFAULT_ROUTING_CLASS;

        //if (((routing_class != 0 ) || ( gateway_class != 0 ))&& (!probe_tun(1)))
        //    throw cRuntimeError("");

        for (int i = 0; i<getNumWlanInterfaces(); i++)
        {
            InterfaceEntry *iEntry = getWlanInterfaceEntry(i);

            BatmanIf *batman_if;
            batman_if = new BatmanIf();
            batman_if->dev = iEntry;
            batman_if->if_num = found_ifs;
            batman_if->seqno = 1;

            batman_if->wifi_if = true;
            batman_if->if_active = true;
            if (isInMacLayer())
            {
                batman_if->address = L3Address(iEntry->getMacAddress());
                batman_if->broad = L3Address(MACAddress::BROADCAST_ADDRESS);
            }
            else
            {
                batman_if->address = L3Address(iEntry->ipv4Data()->getIPAddress());
                batman_if->broad = L3Address(IPv4Address::ALLONES_ADDRESS);
            }

            batman_if->if_rp_filter_old = -1;
            batman_if->if_send_redirects_old = -1;
            if_list.push_back(batman_if);
            if (batman_if->if_num > 0)
                hna_local_task_add_ip(batman_if->address, 32, ROUTE_ADD);  // XXX why is it sending an HNA record at all? HNA should be sent only for networks
            found_ifs++;
        }
        log_facility_active = 1;

        // parse announcedNetworks parameter
        const char *announcedNetworks = par("announcedNetworks");
        cStringTokenizer tokenizer(announcedNetworks);
        const char *token;
        while ((token = tokenizer.nextToken()) != nullptr)
        {
            std::vector<std::string> addrPair = cStringTokenizer(token, "/").asVector();
            if (addrPair.size() != 2)
                throw cRuntimeError("invalid 'announcedNetworks' parameter content: '%s'", token);

            IPv4Address addr = L3AddressResolver().resolve(addrPair[0].c_str()).toIPv4();
            IPv4Address mask = L3AddressResolver().resolve(addrPair[1].c_str(), L3AddressResolver::ADDR_MASK).toIPv4();
            addr.doAnd(mask);

            // add to HNA:
            hna_local_task_add_ip(L3Address(addr), mask.getNetmaskLength(), ROUTE_ADD);
        }

        /* add rule for hna networks */
        //add_del_rule(0, 0, BATMAN_RT_TABLE_NETWORKS, BATMAN_RT_PRIO_UNREACH - 1, 0, RULE_TYPE_DST, RULE_ADD);

        /* add unreachable routing table entry */
        //add_del_route(0, 0, 0, 0, 0, "unknown", BATMAN_RT_TABLE_UNREACH, ROUTE_TYPE_UNREACHABLE, ROUTE_ADD);

        if (routing_class > 0) {
            if (add_del_interface_rules(RULE_ADD) < 0) {
                throw cRuntimeError("BATMAN Interface error");
            }
        }

        //if (gateway_class != 0)
        //    init_interface_gw();

        for (auto & elem : if_list)
        {
            BatmanIf *batman_if = elem;
            schedule_own_packet(batman_if);
        }

        timer = new cMessage();
        WATCH_PTRMAP(origMap);

        simtime_t curr_time = simTime();
        simtime_t select_timeout = forw_list[0]->send_time > curr_time ? forw_list[0]->send_time : curr_time+10;
        scheduleAt(select_timeout, timer);
    }
}

void Batman::handleMessage(cMessage *msg)
{
    BatmanIf *if_incoming = nullptr;
    L3Address neigh;
    simtime_t vis_timeout, select_timeout, curr_time;

    curr_time = getTime();
    check_active_inactive_interfaces();
    if (timer == msg)
    {
        sendPackets(curr_time);
        return;
    }

    /* harden select_timeout against sudden time change (e.g. ntpdate) */
    //select_timeout = ((int)(((struct forw_node *)forw_list.next)->send_time - curr_time) > 0 ?
    //            ((struct forw_node *)forw_list.next)->send_time - curr_time : 10);

    if (!this->isInMacLayer())
    {
        INetworkProtocolControlInfo *ctrl = check_and_cast<INetworkProtocolControlInfo *>(msg->removeControlInfo());
        L3Address srcAddr = ctrl->getSourceAddress();
        neigh = srcAddr;
        for (auto & elem : if_list)
        {
            if (elem->dev->getInterfaceId() == ctrl->getInterfaceId())
            {
                if_incoming = elem;
                break;
            }
        }
        delete ctrl;
    }
    else
    {
        Ieee802Ctrl* ctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());
        MACAddress srcAddr = ctrl->getSrc();
        neigh = srcAddr;
        if_incoming = if_list[0];
        delete ctrl;
    }

    if (!if_incoming)
        throw cRuntimeError("model error: if_incoming is nullptr");

    BatmanPacket *bat_packet = nullptr;
    UDPPacket *udpPacket = dynamic_cast<UDPPacket*>(msg);
    if (udpPacket)
    {
        if (udpPacket->getDestinationPort() != BATMAN_PORT)
        {
            delete  msg;
            sendPackets(curr_time);
            return;
        }

        cMessage* msg_aux = udpPacket->decapsulate();
        bat_packet = dynamic_cast <BatmanPacket*>(msg_aux);
        if (!bat_packet)
        {
            delete msg;
            delete msg_aux;
            sendPackets(curr_time);
            return;
        }

        delete msg;
    }
    else if (this->isInMacLayer())
    {
        if (!dynamic_cast<BatmanPacket*>(msg))
        {
            delete  msg;
            sendPackets(curr_time);
            return;
        }
    }

    parseIncomingPacket(neigh, if_incoming, bat_packet);

    sendPackets(curr_time);
}

void Batman::sendPackets(const simtime_t &curr_time)
{
    send_outstanding_packets(curr_time);
    if (curr_time - debug_timeout > 1) {

        debug_timeout = curr_time;
        purge_orig( curr_time );
        //check_inactive_interfaces();
        if ( ( routing_class != 0 ) && ( curr_gateway == nullptr ) )
            choose_gw();
#if 0
        if ((vis_if.sock) && ((int)(curr_time - (vis_timeout + 10000)) > 0)) {

            vis_timeout = curr_time;
            send_vis_packet();

        }
#endif
        hna_local_task_exec();
    }
    scheduleNextEvent();
}

void Batman::scheduleNextEvent()
{
     simtime_t select_timeout = forw_list[0]->send_time > 0 ? forw_list[0]->send_time : getTime()+10;
     if (timer->isScheduled())
     {
         if (timer->getArrivalTime()>select_timeout)
         {
             cancelEvent(timer);
             if (select_timeout>simTime())
                 scheduleAt(select_timeout, timer);
             else
                 scheduleAt(simTime(), timer);
         }
     }
     else
     {
         if (select_timeout>simTime())
             scheduleAt(select_timeout, timer);
         else
             scheduleAt(simTime(), timer);
     }
}

uint32_t Batman::getRoute(const L3Address &dest, std::vector<L3Address> &add)
{
    auto it = origMap.find(dest);
    if (it != origMap.end())
    {
        OrigNode *node = it->second;
        add.resize(0);
        add.push_back(node->router->addr);
        return -1;
    }
    L3Address apAddr;
    if (getAp(dest,apAddr))
    {
        auto it = origMap.find(apAddr);
        if (it != origMap.end())
        {
            OrigNode *node = it->second;
            add.resize(0);
            add.push_back(node->router->addr);
            return -1;
        }
    }
    return 0;
}

bool Batman::getNextHop(const L3Address &dest, L3Address &add, int &iface, double &val)
{
    auto it = origMap.find(dest);
    if (it != origMap.end())
    {
        OrigNode *node = it->second;
        add = node->router->addr;
        return true;
    }
    L3Address apAddr;
    if (getAp(dest,apAddr))
    {
        auto it = origMap.find(apAddr);
        if (it != origMap.end())
        {
            OrigNode *node = it->second;
            add = node->router->addr;
            return true;
        }
    }
    return false;
}

void Batman::appendPacket(cPacket *oldPacket, cPacket *packetToAppend)
{
    if (oldPacket->getEncapsulatedPacket()==nullptr)
    {
        oldPacket->encapsulate(packetToAppend);
        return;
    }
    std::vector<cPacket*> vectorPacket;
    cPacket *pkt = oldPacket;
    while (pkt->getEncapsulatedPacket())
    {
        vectorPacket.push_back(pkt->decapsulate());
        pkt = vectorPacket.back();
    }
    pkt = packetToAppend;
    while (!vectorPacket.empty())
    {
          cPacket *pktAux = vectorPacket.back();
          pktAux->encapsulate(pkt);
          pkt = pktAux;
          vectorPacket.pop_back();
    }
    oldPacket->encapsulate(pkt);
}

BatmanPacket *Batman::buildDefaultBatmanPkt(const BatmanIf *batman_if)
{
    std::string str = "BatmanPkt:" + batman_if->address.str();
    BatmanPacket *pkt = new BatmanPacket(str.c_str());

    pkt->setVersion(0);
    pkt->setFlags(0x00);
    pkt->setTtl((batman_if->if_num > 0 ? 2 : TTL));
    pkt->setGatewayFlags(batman_if->if_num > 0 ? 0 : gateway_class);
    pkt->setSeqNumber(batman_if->seqno);
    pkt->setGatewayPort(GW_PORT);
    pkt->setTq(TQ_MAX_VALUE);
    if (batman_if->if_active)
    {
       pkt->setOrig(batman_if->address);
       pkt->setPrevSender(batman_if->address);
    }
    return pkt;
}

} // namespace inetmanet

} // namespace inet

