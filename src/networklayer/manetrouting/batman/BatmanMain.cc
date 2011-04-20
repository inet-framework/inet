/*
  */
#include "ManetRoutingBase.h"
#include "batman.h"
#include "IPControlInfo.h"
#include "UDPPacket_m.h"

Define_Module (Batman);

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
    debug_level=0;
    debug_level_max=4;
    gateway_class=0;
    routing_class=0;
    originator_interval=1;
    debug_timeout=0;
    pref_gateway=0;

    nat_tool_avail=0;
    disable_client_nat=0;

    curr_gateway=NULL;

    found_ifs=0;
    active_ifs=0;
    receive_max_sock=0;

    unix_client=0;
    log_facility_active=0;

    origMap.clear();

    if_list.clear();
    gw_list.clear();
    forw_list.clear();
    // struct vis_if vis_if;


    tunnel_running=0;

    hop_penalty = TQ_HOP_PENALTY;
    purge_timeout = PURGE_TIMEOUT;
    minimum_send = TQ_LOCAL_BIDRECT_SEND_MINIMUM;
    minimum_recv = TQ_LOCAL_BIDRECT_RECV_MINIMUM;
    global_win_size = TQ_GLOBAL_WINDOW_SIZE;
    local_win_size = TQ_LOCAL_WINDOW_SIZE;
    num_words = (TQ_LOCAL_WINDOW_SIZE / WORD_BIT_SIZE);
    aggregation_enabled = 1;
    timer=NULL;

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
    while (!hna_chg_list.empty())
    {
        delete hna_chg_list.back();
        hna_chg_list.pop_back();
    }
}


void Batman::initialize(int stage)
{

    if (stage!=4)
        return;


    int32_t download_speed = 0, upload_speed = 0;
    char routing_class_opt=0, pref_gw_opt = 0;
    char purge_timeout_opt = 0;
    found_ifs =0;

    registerRoutingModule();
    //createTimerQueue();

    debug_level =par ("debugLevel");
    if ( debug_level > debug_level_max ) {
            opp_error( "Invalid debug level: %i\nDebug level has to be between 0 and %i.\n", debug_level, debug_level_max );
    }
    simtime_t purge = par("purgeTimeout");
    if (SIMTIME_RAW(purge)>0)
    {
        purge_timeout = purge;
        purge_timeout_opt = 1;
    }
    else
        purge_timeout = PURGE_TIMEOUT;

    simtime_t  originator_i = par("originatorInterval");

    if (SIMTIME_DBL(originator_i)>0.001)
    {
        originator_interval = originator_i;
    }
    else
        originator_interval = 1;// 1000 msec

    const char *preferedGateWay=par("preferedGateWay");
    IPAddress tmp_ip_holder(preferedGateWay);
    if (!tmp_ip_holder.isUnspecified())
    {
        pref_gateway = tmp_ip_holder.getInt();
        pref_gw_opt = 1;
    }
    routing_class = par ("routingClass");
    if (routing_class > 0)
        routing_class_opt = 1;
    else
        routing_class =0;

/*
    IPAddress vis = par("visualizationServer");

    if (!vis.isUnspecified())
    {
        vis_server = vis.getInt();
    }
*/
    if (par("agregationEnable"))
        aggregation_enabled = 1;
    else
        aggregation_enabled = 0;
    disable_client_nat = 1;

    download_speed = par ("GWClass_download_speed");
    upload_speed = par ("GWClass_upload_speed");
    MAX_AGGREGATION_BYTES=par("MAX_AGGREGATION_BYTES");

    if ( ( download_speed > 0 ) && ( upload_speed == 0 ) )
        upload_speed = download_speed / 5;

    if (download_speed > 0) {
        gateway_class = get_gw_class(download_speed, upload_speed);
        get_gw_speeds(gateway_class, &download_speed, &upload_speed);
    }

    if ( ( gateway_class != 0 ) && ( routing_class != 0 ) ) {
        opp_error ("Error - routing class can't be set while gateway class is in use !\n");
    }

    if ( ( gateway_class != 0 ) && ( pref_gateway != 0 ) ) {
        opp_error ("Error - preferred gateway can't be set while gateway class is in use !\n" );
    }

    /* use routing class 1 if none specified */
    if ( ( routing_class == 0 ) && ( pref_gateway != 0 ) )
        routing_class = DEFAULT_ROUTING_CLASS;

    //if (((routing_class != 0 ) || ( gateway_class != 0 ))&& (!probe_tun(1)))
    //    opp_error("");

    for (int i = 0;i<getNumWlanInterfaces();i++) {
        InterfaceEntry * iEntry = getWlanInterfaceEntry(i);

        BatmanIf *batman_if;
        batman_if = new BatmanIf();
        batman_if->dev = iEntry;
        batman_if->if_num = found_ifs;
        batman_if->seqno=1;

        batman_if->wifi_if=true;
        batman_if->if_active = true;
        if (isInMacLayer())
        {
            batman_if->address=(Uint128)iEntry->getMacAddress();
            batman_if->broad=(Uint128)MACAddress::BROADCAST_ADDRESS;
        }
        else
        {
            batman_if->address=(Uint128)iEntry->ipv4Data()->getIPAddress();
            batman_if->broad=(Uint128)IPAddress::ALLONES_ADDRESS;
        }

        batman_if->if_rp_filter_old = -1;
        batman_if->if_send_redirects_old = -1;
        if_list.push_back(batman_if);
        if (batman_if->if_num > 0)
            hna_local_task_add_ip(batman_if->address, 32, ROUTE_ADD);
            found_ifs++;
    }
    log_facility_active = 1;

    /* add rule for hna networks */
    //add_del_rule(0, 0, BATMAN_RT_TABLE_NETWORKS, BATMAN_RT_PRIO_UNREACH - 1, 0, RULE_TYPE_DST, RULE_ADD);

        /* add unreachable routing table entry */
    //add_del_route(0, 0, 0, 0, 0, "unknown", BATMAN_RT_TABLE_UNREACH, ROUTE_TYPE_UNREACHABLE, ROUTE_ADD);

    if (routing_class > 0) {
        if (add_del_interface_rules(RULE_ADD) < 0) {
            opp_error("BATMAN Interface error");
        }
    }

    //if (gateway_class != 0)
    //    init_interface_gw();

    for (unsigned int i = 0;i< if_list.size();i++)
    {
        BatmanIf * batman_if = if_list[i];
        schedule_own_packet(batman_if);
    }


    timer = new cMessage();
    WATCH_PTRMAP (origMap);

    //simtime_t curr_time = simTime();
    //simtime_t select_timeout = (forw_list[0]->send_time - curr_time) > 0 ?forw_list[0]->send_time - curr_time : 10;
    simtime_t select_timeout = forw_list[0]->send_time > 0 ?forw_list[0]->send_time : 10;
    scheduleAt(select_timeout,timer);

}



void Batman::handleMessage(cMessage *msg)
{
    OrigNode *orig_neigh_node, *orig_node;
    BatmanIf *if_incoming;
    Uint128 neigh;
    simtime_t vis_timeout, select_timeout, curr_time;

    BatmanHnaMsg *hna_recv_buff;
    //char orig_str[ADDR_STR_LEN], neigh_str[ADDR_STR_LEN], ifaddr_str[ADDR_STR_LEN], prev_sender_str[ADDR_STR_LEN];
    int16_t hna_buff_len, curr_packet_len;
    uint8_t is_my_addr, is_my_orig, is_my_oldorig, is_broadcast, is_duplicate, is_bidirectional, has_directlink_flag;

    curr_time = getTime();
    check_active_inactive_interfaces();
    if (timer == msg)
    {
        sendPackets(curr_time);
        numOrig=origMap.size();
        return;
    }


    /* harden select_timeout against sudden time change (e.g. ntpdate) */
    //select_timeout = ((int)(((struct forw_node *)forw_list.next)->send_time - curr_time) > 0 ?
    //            ((struct forw_node *)forw_list.next)->send_time - curr_time : 10);

    IPControlInfo *ctrl = check_and_cast<IPControlInfo *>(msg->removeControlInfo());
    IPvXAddress srcAddr = ctrl->getSrcAddr();
    IPvXAddress destAddr = ctrl->getDestAddr();
    neigh = srcAddr.get4().getInt();
    for (unsigned int i=0;i<if_list.size();i++)
    {
        if (if_list[i]->dev->getInterfaceId()==ctrl->getInterfaceId())
        {
            if_incoming=if_list[i];
            break;
        }
    }

    if (ctrl)
        delete ctrl;

    curr_packet_len = 0;

    BatmanPacket * bat_packet = NULL;
    UDPPacket * udpPacket=dynamic_cast<UDPPacket*>(msg);
    if (udpPacket)
    {
        if (udpPacket->getDestinationPort()!= BATMAN_PORT)
        {
            delete  msg;
            sendPackets(curr_time);
            return;
        }
        cMessage* msg_aux  = udpPacket->decapsulate();
        bat_packet = dynamic_cast <BatmanPacket*>(msg_aux);
        if (!bat_packet)
        {
            delete msg;
            delete msg_aux;
            sendPackets(curr_time);
            numOrig=origMap.size();
            return;
        }
        delete msg;
    }

    //addr_to_string(neigh, neigh_str, sizeof(neigh_str));
    //addr_to_string(if_incoming->addr.sin_addr.s_addr, ifaddr_str, sizeof(ifaddr_str));

    while (bat_packet) {
        //addr_to_string(bat_packet->orig, orig_str, sizeof(orig_str));
        //addr_to_string(bat_packet->prev_sender, prev_sender_str, sizeof(prev_sender_str));
        if (isInMacLayer())
            EV << "packet receive from :" <<bat_packet->getOrig().getMACAddress() << endl;
        else
            EV << "packet receive from :" <<bat_packet->getOrig().getIPAddress() << endl;
        is_my_addr = is_my_orig = is_my_oldorig = is_broadcast = 0;

        has_directlink_flag = (bat_packet->getFlags() & DIRECTLINK ? 1 : 0);

        hna_buff_len = bat_packet->getHnaLen() * 5;


        unsigned char  hnaLen =  bat_packet->getHnaMsgArraySize();
        if (hnaLen!=0)
            hna_recv_buff = &bat_packet->getHnaMsg(0);
        else
            hna_recv_buff = NULL;


        if (isLocalAddress(neigh))
            is_my_addr = 1;
        if (isLocalAddress(bat_packet->getOrig()))
            is_my_orig = 1;
        if (isMulticastAddress(neigh))
            is_broadcast = 1;
        if (isLocalAddress(bat_packet->getPrevSender()))
            is_my_oldorig = 1;


        if (bat_packet->getVersion() != 0) {
            EV << "Drop packet: incompatible batman version "<< bat_packet->getVersion() <<endl;
            delete bat_packet;
            break;
        }

        if (is_my_addr) {
            EV << "Drop packet: received my own broadcast sender:" << srcAddr <<endl;
            delete bat_packet;
            break;
        }

        if (is_broadcast) {
            EV<< "Drop packet: ignoring all packets with broadcast source IP sender: " << srcAddr <<endl;
            delete bat_packet;
            break;
        }


        if (is_my_orig) {
            orig_neigh_node = get_orig_node(neigh);
            bool sameIf =false;
            if (if_incoming->dev->ipv4Data()->getIPAddress().getInt() == bat_packet->getOrig().getIPAddress().getInt())
                sameIf=true;

            if ((has_directlink_flag) && (sameIf) && (bat_packet->getSeqNumber() - if_incoming->seqno + 2 == 0))
            {
                std::vector<TYPE_OF_WORD>vectorAux;
                for (unsigned int i=0;i<num_words;i++)
                {
                	vectorAux.push_back(orig_neigh_node->bcast_own[(if_incoming->if_num * num_words)+i]);
                }
                bit_mark(vectorAux, 0);
                orig_neigh_node->bcast_own_sum[if_incoming->if_num] = bit_packet_count(vectorAux);
                for (unsigned int i=0;i<num_words;i++)
                {
                	orig_neigh_node->bcast_own[(if_incoming->if_num * num_words)+i]= vectorAux[i];
                }
                EV<< "count own bcast (is_my_orig): old = " << orig_neigh_node->bcast_own_sum[if_incoming->if_num]<<endl;
            }
            EV << "Drop packet: originator packet from myself (via neighbour) \n";
            delete bat_packet;
            break;
        }

        if (bat_packet->getTq() == 0) {
            count_real_packets(bat_packet, neigh, if_incoming);

            EV<< "Drop packet: originator packet with tq is 0 \n";
            delete bat_packet;
            break;
        }

        if (is_my_oldorig) {
            EV << "Drop packet: ignoring all rebroadcast echos sender: " << srcAddr << endl;
            delete bat_packet;
            break;
        }

        is_duplicate = count_real_packets(bat_packet, neigh, if_incoming);

        orig_node = get_orig_node(bat_packet->getOrig());


        /* if sender is a direct neighbor the sender ip equals originator ip */
        orig_neigh_node = (bat_packet->getOrig() == neigh ? orig_node : get_orig_node(neigh));

        /* drop packet if sender is not a direct neighbor and if we no route towards it */
        if ((bat_packet->getOrig() != neigh) && (orig_neigh_node->router == NULL))
        {
            delete bat_packet;
            break;
        }
        orig_node->totalRec++;
        is_bidirectional = isBidirectionalNeigh(orig_node, orig_neigh_node, bat_packet, curr_time, if_incoming);

        /* update ranking if it is not a duplicate or has the same seqno and similar ttl as the non-duplicate */
        if ((is_bidirectional) && ((!is_duplicate) ||
             ((orig_node->last_real_seqno == bat_packet->getSeqNumber()) &&
             (orig_node->last_ttl - 3 <= bat_packet->getTtl()))))
            update_orig(orig_node, bat_packet, neigh, if_incoming, hna_recv_buff, hna_buff_len, is_duplicate, curr_time);

            /* is single hop (direct) neighbour */
        if (bat_packet->getOrig() == neigh) {
                /* mark direct link on incoming interface */
            schedule_forward_packet(orig_node, bat_packet, neigh, 1, hna_buff_len, if_incoming, curr_time);
            //delete bat_packet;
            EV << "Forward packet: rebroadcast neighbour packet with direct link flag \n";
            break;
        }

            /* multihop originator */
        if (!is_bidirectional) {
            EV << "Drop packet: not received via bidirectional link\n";
            delete bat_packet;
            break;
        }

        if (is_duplicate) {
            EV << "Drop packet: duplicate packet received\n";
            delete bat_packet;
            break;
        }

        BatmanPacket * bat_packetAux = bat_packet;
        bat_packet = (BatmanPacket*) bat_packet->decapsulate();
        schedule_forward_packet(orig_node,bat_packetAux, neigh, 0, hna_buff_len, if_incoming, curr_time);
    }
    sendPackets(curr_time);
    numOrig=origMap.size();
}

void Batman::sendPackets(const simtime_t &curr_time)
{
    send_outstanding_packets(curr_time);
    if (curr_time - debug_timeout  > 1) {

        debug_timeout = curr_time;
        purge_orig( curr_time );
        //check_inactive_interfaces();
        if ( ( routing_class != 0 ) && ( curr_gateway == NULL ) )
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
     simtime_t select_timeout = forw_list[0]->send_time > 0 ?forw_list[0]->send_time : getTime()+10;
     if (timer->isScheduled())
     {
         if (timer->getArrivalTime()>select_timeout)
         {
             cancelEvent(timer);
             if (select_timeout>simTime())
                 scheduleAt(select_timeout,timer);
             else
                 scheduleAt(simTime(),timer);
         }
     }
     else
     {
         if (select_timeout>simTime())
             scheduleAt(select_timeout,timer);
         else
             scheduleAt(simTime(),timer);
     }
}

uint32_t Batman::getRoute(const Uint128 &dest,std::vector<Uint128> &add)
{

	OrigMap::iterator it = origMap.find(dest);
	if (it != origMap.end())
	{
		OrigNode * node =it->second;
		add.resize(0);
		add.push_back(node->router->addr);
		return -1;
	}
	return 0;
}

bool Batman::getNextHop(const Uint128 &dest,Uint128 &add,int &iface, double &val)
{
	OrigMap::iterator it = origMap.find(dest);
	if (it != origMap.end())
	{
		OrigNode * node =it->second;
		add=node->router->addr;
		return true;
	}
	return false;
}



