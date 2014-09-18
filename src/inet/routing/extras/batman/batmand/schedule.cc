/*
 * Copyright (C) 2006-2009 B.A.T.M.A.N. contributors:
 *
 * Simon Wunderlich, Axel Neumann, Marek Lindner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */


#include "inet/routing/extras/batman/batmand/batman.h"
#include "inet/routing/extras/batman/BatmanMain.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

namespace inet {

namespace inetmanet {

void Batman::schedule_own_packet(BatmanIf *batman_if)
{
    ForwNode *forw_node_new = NULL;
    OrigNode *orig_node;

    EV_DEBUG << "schedule_own_packet(): " << batman_if->dev << "\n";

    forw_node_new = new ForwNode();

    forw_node_new->send_time = getTime() + originator_interval + par("jitter2");
    forw_node_new->if_incoming = batman_if;
    forw_node_new->own = 1;
    forw_node_new->num_packets = 0;
    forw_node_new->direct_link_flags = 0;

    EV_INFO << "Send own packet in "<< forw_node_new->send_time << endl;


    /* non-primary interfaces do not send hna information */
    forw_node_new->pack_buff = buildDefaultBatmanPkt(batman_if);
    if ((hna_list.size() > 0) && (batman_if->if_num == 0)) {
        forw_node_new->pack_buff->setHnaMsgArraySize(hna_buff_local.size());
        forw_node_new->pack_buff->setByteLength(forw_node_new->pack_buff->getByteLength() + (hna_buff_local.size() * BATMAN_HNA_MSG_SIZE));
        for (unsigned int i = 0; i<hna_buff_local.size(); i++) {
            HnaElement aux;
            aux.addr = hna_buff_local[i].addr;
            aux.netmask = hna_buff_local[i].netmask;
            forw_node_new->pack_buff->setHnaMsg(i, aux);
        }
    }

    /* change sequence number to network order */
    //((BatmanPacket *)forw_node_new->pack_buff)->seqno = htons(((BatmanPacket *)forw_node_new->pack_buff)->seqno);


    Forwlist::iterator forwListIt;
    for (forwListIt = forw_list.begin(); forwListIt != forw_list.end(); forwListIt++) {
        ForwNode *forw_packet_tmp = *forwListIt;

        if (forw_packet_tmp->send_time > forw_node_new->send_time) {
            forw_list.insert(forwListIt, forw_node_new);
            break;
        }

//        forw_packet_tmp = NULL;
    }

    if (forwListIt == forw_list.end())
        forw_list.push_back(forw_node_new);

    batman_if->seqno++;

    for (OrigMap::iterator it = origMap.begin(); it != origMap.end(); ++it)
    {
        orig_node = it->second;

        EV_DETAIL << "count own bcast (schedule_own_packet): old = " << (unsigned)orig_node->bcast_own_sum[batman_if->if_num] << ", ";
        std::vector<TYPE_OF_WORD>vectorAux;
        for (unsigned int i=0; i<num_words; i++) {
            vectorAux.push_back(orig_node->bcast_own[(batman_if->if_num * num_words)+i]);
        }
        bit_get_packet(vectorAux, 1, 0);
        orig_node->bcast_own_sum[batman_if->if_num] = bit_packet_count(vectorAux);
        for (unsigned int i=0; i<num_words; i++) {
            orig_node->bcast_own[(batman_if->if_num * num_words)+i] = vectorAux[i];
        }
        vectorAux.clear();
        EV_DETAIL << "new = " << (unsigned)orig_node->bcast_own_sum[batman_if->if_num] << "\n";
    }
}


void Batman::schedule_forward_packet(OrigNode *orig_node, BatmanPacket *in, const L3Address &neigh, uint8_t directlink, int16_t hna_buff_len, BatmanIf *if_incoming, const simtime_t &curr_time)
{
    ForwNode *forw_node_new = NULL, *forw_node_aggregate = NULL, *forw_node_pos = NULL;
    //struct list_head *list_pos = forw_list.next, *prev_list_head = (struct list_head *)&forw_list;
    BatmanPacket *bat_packet;
    uint8_t tq_avg = 0;
    simtime_t send_time;

    EV_DEBUG << "schedule_forward_packet():  \n";

    if (in->getTtl() <= 1) {
        EV_WARN << "ttl exceeded \n";
        return;
    }

    if (aggregation_enabled)
        send_time = curr_time + par("MAX_AGGREGATION_MS") + par("jitter2").doubleValue()/2.0;
    else
        send_time = curr_time + par("jitter").doubleValue()/2.0;

    /* find position for the packet in the forward queue */
    Forwlist::iterator  it;
    for (it=forw_list.begin(); it!=forw_list.end(); it++) {
        forw_node_pos = *it;

        if (aggregation_enabled) {
            /* don't save aggregation position if aggregation is disabled */
            forw_node_aggregate = forw_node_pos;

            /**
             * we can aggregate the current packet to this packet if:
             * - the send time is within our MAX_AGGREGATION_MS time
             * - the resulting packet wont be bigger than MAX_AGGREGATION_BYTES
             */
            if ((forw_node_pos->send_time < send_time) &&
                (forw_node_pos->pack_buff->getByteLength() + in->getByteLength() <= MAX_AGGREGATION_BYTES)) {
                bat_packet = forw_node_pos->pack_buff;

                /**
                 * check aggregation compability
                 * -> direct link packets are broadcasted on their interface only
                 * -> aggregate packet if the current packet is a "global" packet
                 *    as well as the base packet
                 */

                /* packets without direct link flag and high TTL are flooded through the net  */
                if ((!directlink) && (!(bat_packet->getFlags() & DIRECTLINK)) && (bat_packet->getTtl() != 1) &&

                /* own packets originating non-primary interfaces leave only that interface */
                        ((!forw_node_pos->own) || (forw_node_pos->if_incoming->if_num == 0)))
                    break;

                /* if the incoming packet is sent via this one interface only - we still can aggregate */
                if ((directlink) && (in->getTtl() == 2) && (forw_node_pos->if_incoming == if_incoming))
                    break;
            }

            /* could not find packet to aggregate with */
            forw_node_aggregate = NULL;
        }

        if (forw_node_pos->send_time > send_time)
            break;
    }

    bat_packet = in->dup();
    delete bat_packet->decapsulate();

    /* nothing to aggregate with - either aggregation disabled or no suitable aggregation packet found */
    if (forw_node_aggregate == NULL) {
        forw_node_new = new ForwNode();
        forw_node_new->pack_buff = bat_packet;

        forw_node_new->own = 0;
        forw_node_new->if_incoming = if_incoming;
        forw_node_new->num_packets = 0;
        forw_node_new->direct_link_flags = 0;

        forw_node_new->send_time = send_time;
    } else {
        // It's necessary decapsulate and recapsulate the packets
        appendPacket(forw_node_aggregate->pack_buff, bat_packet);
        forw_node_aggregate->num_packets++;

        forw_node_new = forw_node_aggregate;
    }

    /* save packet direct link flag status */
    if (directlink)
        forw_node_new->direct_link_flags = forw_node_new->direct_link_flags | (1 << forw_node_new->num_packets);

    bat_packet->setTtl(bat_packet->getTtl()-1);
    bat_packet->setHops(bat_packet->getHops()+1);
    bat_packet->setPrevSender(neigh);

    /* rebroadcast tq of our best ranking neighbor to ensure the rebroadcast of our best tq value */
    if ((orig_node->router != NULL) && (orig_node->router->tq_avg != 0)) {
        /* rebroadcast ogm of best ranking neighbor as is */
        if (orig_node->router->addr != neigh) {
            bat_packet->setTq(orig_node->router->tq_avg);
            bat_packet->setTtl(orig_node->router->last_ttl - 1);
            bat_packet->setHops(orig_node->router->num_hops+1);
        }

        tq_avg = orig_node->router->tq_avg;
    }

    /* apply hop penalty */
    bat_packet->setTq(bat_packet->getTq() * (TQ_MAX_VALUE - hop_penalty) / (TQ_MAX_VALUE));

    //debug_output(4, "forwarding: tq_orig: %i, tq_avg: %i, tq_forw: %i, ttl_orig: %i, ttl_forw: %i \n", in->tq, tq_avg, bat_packet->tq, in->ttl - 1, bat_packet->ttl);

    /* change sequence number to network order */
    //bat_packet->seqno = htons(bat_packet->seqno);

    if (directlink)
        bat_packet->setFlags(bat_packet->getFlags()|DIRECTLINK);
    else
        bat_packet->setFlags(bat_packet->getFlags()&(~DIRECTLINK));

    /* if the packet was not aggregated */
    if (forw_node_aggregate == NULL) {
        /* if the packet should go somewhere in the queue */
        if (it != forw_list.end())
            forw_list.insert(it, forw_node_new);
        /* if the packet is the last packet in the queue */
        else
            forw_list.push_back(forw_node_new);
    }
    EV_INFO << "Fordward packet " << bat_packet << "at :" << forw_node_new->send_time << endl;
}


void Batman::send_outstanding_packets(const simtime_t &curr_time)
{
    ForwNode *forw_node;
    BatmanIf *batman_if;
    BatmanPacket *bat_packet;
    uint8_t directlink, curr_packet_num;
    int16_t curr_packet_len;

    while (true) {
        forw_node = forw_list.front();

        if (curr_time < forw_node->send_time)
            break;

        forw_list.erase(forw_list.begin());

        bat_packet = forw_node->pack_buff;

        //addr_to_string(bat_packet->orig, orig_str, ADDR_STR_LEN);

        directlink = (bat_packet->getFlags() & DIRECTLINK ? 1 : 0);

        if (forw_node->if_incoming == NULL) {
            EV_ERROR << "Error - can't forward packet: incoming iface not specified \n";
            delete forw_node->pack_buff;
            delete forw_node;
            continue;
        }

        /* multihomed peer assumed */
        /* non-primary interfaces are only broadcasted on their interface */
        if (((directlink) && (bat_packet->getTtl() == 1)) ||
            ((forw_node->own) && (forw_node->if_incoming->if_num > 0))) {
            EV_DEBUG << (forw_node->own ? "Sending own" : "Forwarding") << " packet (originator " << bat_packet->getOrig() << ", seqno " << bat_packet->getSeqNumber() << ", TTL " << bat_packet->getTtl() << ") on interface " << forw_node->if_incoming->dev << "\n";

            if (send_udp_packet(forw_node->pack_buff->dup(), forw_node->pack_buff_len, forw_node->if_incoming->broad, BATMAN_PORT, forw_node->if_incoming) < 0)
                    deactivate_interface(forw_node->if_incoming);

            goto packet_free;
        }

        for (unsigned int i = 0; i<if_list.size(); i++) {
            batman_if = if_list[i];

            curr_packet_num = curr_packet_len = 0;
            bat_packet = forw_node->pack_buff->dup();
            BatmanPacket *bat_packetAux = bat_packet;

            while (bat_packetAux != NULL) {
                if ((forw_node->direct_link_flags & (1 << curr_packet_num)) && (forw_node->if_incoming == batman_if))
                    bat_packetAux->setFlags(bat_packetAux->getFlags() | DIRECTLINK);
                else
                    bat_packetAux->setFlags(bat_packetAux->getFlags() & ~DIRECTLINK);

//                if (curr_packet_num > 0)
//                    addr_to_string(bat_packet->orig, orig_str, ADDR_STR_LEN);

                /**
                 * if the outgoing interface is a wifi interface and equal to the incoming interface
                 * add extra penalty (own packets are to be ignored)
                 */
                if ((batman_if->wifi_if) && (!forw_node->own) && (forw_node->if_incoming == batman_if))
                    bat_packetAux->setTq((bat_packetAux->getTq() * (TQ_MAX_VALUE - (2 * hop_penalty))) / (TQ_MAX_VALUE));

                EV_DETAIL << (curr_packet_num > 0 ? "Forwarding" : (forw_node->own ? "Sending own" : "Forwarding")) << " " << (curr_packet_num > 0 ? "aggregated " : "") << "packet (originator " << bat_packet->getOrig() << ", seqno " << bat_packet->getSeqNumber() << ", TQ " << (unsigned)bat_packet->getTq() << ", TTL " << bat_packet->getTtl() << ", IDF " << (bat_packet->getFlags() & DIRECTLINK ? "on" : "off") << ") on interface " << batman_if->dev << "\n";

                bat_packetAux = const_cast<BatmanPacket *> (dynamic_cast<BatmanPacket *>(bat_packetAux->getEncapsulatedPacket()));
            }

            if (send_udp_packet(bat_packet, forw_node->pack_buff_len, batman_if->broad, batman_if->udp_send_sock, batman_if) < 0)
                deactivate_interface(batman_if);

        }

packet_free:    //list_del((struct list_head *)&forw_list, forw_pos, &forw_list);

        if (forw_node->own)
            schedule_own_packet(forw_node->if_incoming);

        delete forw_node->pack_buff;
        delete forw_node;
    }
}

} // namespace inetmanet

} // namespace inet

