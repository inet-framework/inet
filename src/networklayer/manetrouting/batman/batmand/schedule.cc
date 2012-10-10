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


#include "batman.h"
#include "BatmanMain.h"
#include "IPv4InterfaceData.h"


void Batman::schedule_own_packet(BatmanIf *batman_if)
{
    ForwNode *forw_node_new = NULL;
    OrigNode *orig_node;

    //debug_output(4, "schedule_own_packet(): %s \n", batman_if->dev);

    forw_node_new = new ForwNode;

    do
    {
       forw_node_new->send_time = getTime() + originator_interval + par("jitter2");
    } while (forw_node_new->send_time < simTime()); // avoid schedule in the past  // FIXME handle this correctly. this can lead to infinite loop

    EV << "Send own packet in "<< forw_node_new->send_time <<endl;

    forw_node_new->if_incoming = batman_if;
    forw_node_new->own = 1;
    forw_node_new->num_packets = 0;
    forw_node_new->direct_link_flags = 0;
    forw_node_new->pack_buff = buildDefaultBatmanPkt(batman_if);

    /* non-primary interfaces do not send hna information */
    if ((hna_list.size() > 0) && (batman_if->if_num == 0)) {
        forw_node_new->pack_buff->setHnaMsgArraySize(hna_buff_local.size());
        forw_node_new->pack_buff->setByteLength(forw_node_new->pack_buff->getByteLength()+(hna_buff_local.size() * BATMAN_HNA_MSG_SIZE));
        for (unsigned int i = 0; i<hna_buff_local.size(); i++)
        {
            BatmanHnaMsg aux;
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
    }

    if (forwListIt == forw_list.end()) {
          forw_list.push_back(forw_node_new);
    }

    batman_if->seqno++;
    for (OrigMap::iterator it = origMap.begin(); it!=origMap.end(); it++)
    {
        orig_node = it->second;
        //debug_output(4, "count own bcast (schedule_own_packet): old = %i, ", orig_node->bcast_own_sum[batman_if->if_num]);

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
        //debug_output(4, "new = %i \n", orig_node->bcast_own_sum[batman_if->if_num]);
    }
}


void Batman::schedule_forward_packet(OrigNode *orig_node, BatmanPacket *in, const Uint128 &neigh, uint8_t directlink, int16_t hna_buff_len, BatmanIf *if_incoming, const simtime_t &curr_time)
{
    ForwNode *forw_node_new = NULL, *forw_node_aggregate = NULL, *forw_node_pos = NULL;
    //struct list_head *list_pos = forw_list.next, *prev_list_head = (struct list_head *)&forw_list;
    BatmanPacket *bat_packet;
    uint8_t tq_avg = 0;
    simtime_t send_time;

    //debug_output(4, "schedule_forward_packet():  \n");

    if (in->getTtl() <= 1) {
        EV << "ttl exceeded \n";
        return;
    }

    do
    {
        if (aggregation_enabled)
            send_time = curr_time + par("MAX_AGGREGATION_MS") + par("jitter2").doubleValue()/2.0;
        else
            send_time = curr_time + par("jitter").doubleValue()/2.0;
    }while (simTime()>send_time); // avoid schedule in the past

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

        if ((forw_node_pos->send_time - send_time) > 0)
            break;
    }

    /* nothing to aggregate with - either aggregation disabled or no suitable aggregation packet found */
    if (forw_node_aggregate == NULL) {
        forw_node_new = new  ForwNode();
        forw_node_new->pack_buff = in;
        forw_node_new->own = 0;
        forw_node_new->if_incoming = if_incoming;
        forw_node_new->num_packets = 0;
        forw_node_new->direct_link_flags = 0;

        forw_node_new->send_time = send_time;
    } else {
        // It's necessary decapsulate and recapsulate the packets
        appendPacket(forw_node_aggregate->pack_buff, in);
        forw_node_aggregate->num_packets++;

        forw_node_new = forw_node_aggregate;
    }

    /* save packet direct link flag status */
    if (directlink)
        forw_node_new->direct_link_flags = forw_node_new->direct_link_flags | (1 << forw_node_new->num_packets);

    bat_packet = in;
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
    EV << "Fordward packet " << bat_packet << "at :" << forw_node_new->send_time << endl;
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
            EV << "Error - can't forward packet: incoming iface not specified \n";
            delete forw_node->pack_buff;
            delete forw_node;
            continue;
        }

        /* multihomed peer assumed */
        /* non-primary interfaces are only broadcasted on their interface */
        if (((directlink) && (bat_packet->getTtl() == 1)) ||
            ((forw_node->own) && (forw_node->if_incoming->if_num > 0))) {
            //debug_output(4, "%s packet (originator %s, seqno %d, TTL %d) on interface %s\n", (forw_node->own ? "Sending own" : "Forwarding"), orig_str, ntohs(bat_packet->seqno), bat_packet->ttl, forw_node->if_incoming->dev);

            if (send_udp_packet(forw_node->pack_buff->dup(), forw_node->pack_buff_len, forw_node->if_incoming->broad, BATMAN_PORT, forw_node->if_incoming) < 0)
                    deactivate_interface(forw_node->if_incoming);
        } else {
           for (unsigned int i = 0; i<if_list.size(); i++) {
               batman_if = if_list[i];
               curr_packet_num = curr_packet_len = 0;
               bat_packet = forw_node->pack_buff->dup();
               BatmanPacket *bat_packetAux = bat_packet;

               while (bat_packetAux!=NULL) {
                   if ((forw_node->direct_link_flags & (1 << curr_packet_num)) && (forw_node->if_incoming == batman_if))
                       bat_packetAux->setFlags(bat_packetAux->getFlags() | DIRECTLINK);
                   else
                       bat_packetAux->setFlags(bat_packetAux->getFlags()&~DIRECTLINK);

//                   if (curr_packet_num > 0)
//                       addr_to_string(bat_packet->orig, orig_str, ADDR_STR_LEN);

                /**
                 * if the outgoing interface is a wifi interface and equal to the incoming interface
                 * add extra penalty (own packets are to be ignored)
                 */
                   if ((batman_if->wifi_if) && (!forw_node->own) && (forw_node->if_incoming == batman_if))
                       bat_packetAux->setTq((bat_packetAux->getTq() * (TQ_MAX_VALUE - (2 * hop_penalty))) / (TQ_MAX_VALUE));

                   //debug_output(4, "%s %spacket (originator %s, seqno %d, TQ %d, TTL %d, IDF %s) on interface %s\n", (curr_packet_num > 0 ? "Forwarding" : (forw_node->own ? "Sending own" : "Forwarding")), (curr_packet_num > 0 ? "aggregated " : ""), orig_str, ntohs(bat_packet->seqno), bat_packet->tq, bat_packet->ttl, (bat_packet->flags & DIRECTLINK ? "on" : "off"), batman_if->dev);

                   bat_packetAux = const_cast<BatmanPacket *> (dynamic_cast<BatmanPacket *>(bat_packetAux->getEncapsulatedPacket()));
               }

               if (send_udp_packet(bat_packet, forw_node->pack_buff_len, batman_if->broad, batman_if->udp_send_sock, batman_if) < 0)
                   deactivate_interface(batman_if);
           }
        }

        if (forw_node->own)
            schedule_own_packet(forw_node->if_incoming);

        delete forw_node->pack_buff;
        delete forw_node;
    }
}


