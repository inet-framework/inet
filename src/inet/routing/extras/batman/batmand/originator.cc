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

NeighNode * Batman::create_neighbor(OrigNode *orig_node, OrigNode *orig_neigh_node, const L3Address &neigh, BatmanIf *if_incoming)
{
    return new NeighNode(orig_node, orig_neigh_node, neigh, if_incoming, num_words, global_win_size);
}

#if 0
/* needed for hash, compares 2 OrigNode, but only their ip-addresses. assumes that
 * the ip address is the first field in the struct */
int compare_orig(void *data1, void *data2)
{
    return (memcmp(data1, data2, 4) == 0 ? 1 : 0);
}


/* hashfunction to choose an entry in a hash table of given size */
/* hash algorithm from http://en.wikipedia.org/wiki/Hash_table */
int choose_orig(void *data, int32_t size) {
    unsigned char *key= data;
    uint32_t hash = 0;
    size_t i;

    for (i = 0; i < 4; i++) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return (hash%size);
}
#endif


/* this function finds or creates an originator entry for the given address if it does not exits */
OrigNode *Batman::get_orig_node(const L3Address &addr)
{
    OrigNode *orig_node;

    auto it = origMap.find(addr);

    if (it != origMap.end())
        return it->second;

    //addr_to_string(addr, orig_str, ADDR_STR_LEN);
    //debug_output(4, "Creating new originator: %s \n", orig_str);

    orig_node = new OrigNode();
    orig_node->bcast_own.resize(found_ifs * num_words);
    orig_node->bcast_own_sum.resize(found_ifs);
    orig_node->clear();
    orig_node->orig = addr;
    origMap.insert(std::pair<L3Address, OrigNode *>(addr, orig_node));
    return orig_node;
}

void Batman::update_orig(OrigNode *orig_node, BatmanPacket *in, const L3Address &neigh, BatmanIf *if_incoming, HnaElement *hna_recv_buff, int16_t hna_buff_len, uint8_t is_duplicate, const simtime_t &curr_time)
{
    GwNode *gw_node;
    NeighNode *neigh_node = nullptr, *tmp_neigh_node = nullptr, *best_neigh_node = nullptr;
    uint8_t max_bcast_own = 0, max_tq = 0;

    EV_INFO << "update_originator(): Searching and updating originator entry of received packet,  \n";

    for (unsigned int list_pos = 0; list_pos < orig_node->neigh_list.size(); list_pos++) {
        tmp_neigh_node = orig_node->neigh_list[list_pos];

        if ((tmp_neigh_node->addr == neigh) && (tmp_neigh_node->if_incoming == if_incoming)) {
            neigh_node = tmp_neigh_node;
        } else {
            if (!is_duplicate) {
                ring_buffer_set(tmp_neigh_node->tq_recv, tmp_neigh_node->tq_index, 0);
                tmp_neigh_node->tq_avg = ring_buffer_avg(tmp_neigh_node->tq_recv);
            }

            /* if we got have a better tq value via this neighbour or same tq value if it is currently our best neighbour (to avoid route flipping) */
            if ((tmp_neigh_node->tq_avg > max_tq) || ((tmp_neigh_node->tq_avg == max_tq) && (tmp_neigh_node->orig_node->bcast_own_sum[if_incoming->if_num] > max_bcast_own)) || ((orig_node->router == tmp_neigh_node) && (tmp_neigh_node->tq_avg == max_tq))) {
                max_tq = tmp_neigh_node->tq_avg;
                max_bcast_own = tmp_neigh_node->orig_node->bcast_own_sum[if_incoming->if_num];
                best_neigh_node = tmp_neigh_node;
            }
        }
    }

    if (neigh_node == nullptr) {
        neigh_node = new NeighNode(orig_node, get_orig_node(neigh), neigh, if_incoming, num_words, global_win_size);
    } else {
        EV_INFO << "Updating existing last-hop neighbour of originator\n";
    }

    neigh_node->last_valid = curr_time;

    ring_buffer_set(neigh_node->tq_recv, neigh_node->tq_index, in->getTq());
    neigh_node->tq_avg = ring_buffer_avg(neigh_node->tq_recv);

/*     is_new_seqno = bit_get_packet(neigh_node->seq_bits, in->seqno - orig_node->last_seqno, 1);
     is_new_seqno = ! get_bit_status(neigh_node->real_bits, orig_node->last_real_seqno, in->seqno); */

    if (!is_duplicate)
    {
        orig_node->last_ttl = in->getTtl();
        neigh_node->last_ttl = in->getTtl();
        neigh_node->num_hops = in->getHops();
    }

    if ((neigh_node->tq_avg > max_tq) || ((neigh_node->tq_avg == max_tq) && (neigh_node->orig_node->bcast_own_sum[if_incoming->if_num] > max_bcast_own)) || ((orig_node->router == neigh_node) && (neigh_node->tq_avg == max_tq))) {
        best_neigh_node = neigh_node;
    }

    /* update routing table and check for changed hna announcements */
    update_routes(orig_node, best_neigh_node, hna_recv_buff, hna_buff_len);

    if (orig_node->gwflags != in->getGatewayFlags())
        update_gw_list(orig_node, in->getGatewayFlags(), in->getGatewayPort());

    orig_node->gwflags = in->getGatewayFlags();
    hna_global_check_tq(orig_node);

    /* restart gateway selection if we have more packets and fast or late switching enabled */
    if ((routing_class > 2) && (orig_node->gwflags != 0) && (curr_gateway != nullptr)) {
        /* if the node is not our current gateway and
           we have preferred gateray disabled and a better tq value or we found our preferred gateway */
        if ((curr_gateway->orig_node != orig_node) &&
                   ((pref_gateway.isUnspecified() && (orig_node->router->tq_avg > curr_gateway->orig_node->router->tq_avg)) || (pref_gateway == orig_node->orig))) {
            /* it is our preferred gateway or we have fast switching or the tq is $routing_class better than our old tq */
            if ((pref_gateway == orig_node->orig) || (routing_class == 3) || (orig_node->router->tq_avg - curr_gateway->orig_node->router->tq_avg >= routing_class)) {
                gw_node = nullptr;

                for (auto & elem : gw_list) {
                    gw_node = elem;

                    if (gw_node->orig_node == orig_node)
                        break;

                    gw_node = nullptr;
                }

                /* if this gateway had not a gateway failure within the last 30 seconds */
                if ((gw_node != nullptr) && (curr_time > (gw_node->last_failure + 30.000))) {
                    EV_INFO << "Gateway client - restart gateway selection: better gateway found (tq curr: " << curr_gateway->orig_node->router->tq_avg << ", tq new: " << orig_node->router->tq_avg << ") \n";

                    del_default_route();
                }
            }
        }
    }
}


void Batman::purge_orig(const simtime_t &curr_time)
{
    OrigNode *orig_node;
    NeighNode *neigh_node, *best_neigh_node;
    GwNode *gw_node;
    uint8_t gw_purged = 0, neigh_purged, max_tq;

    /* for all origins... */
    for (auto it = origMap.begin(); it != origMap.end();) {
        orig_node = it->second;

        if (curr_time > (orig_node->last_valid + (2 * purge_timeout))) {
            //addr_to_string(orig_node->orig, orig_str, ADDR_STR_LEN);
            EV_DETAIL << "Originator timeout: originator " << orig_node->orig << ", last_valid " << orig_node->last_valid << " \n";

            if (it != origMap.begin()) {
                auto itAux = it;
                it++;
                origMap.erase(itAux);
            } else {
                origMap.erase(it);
                it = origMap.begin();
            }

            /* for all neighbours towards this originator ... */
            while (!orig_node->neigh_list.empty()) {
                NeighNode *node = orig_node->neigh_list.back();

                orig_node->neigh_list.pop_back();
                node->owner_node = nullptr;
                delete node;
            }

            for (auto & elem : gw_list) {
                gw_node = elem;

                if (gw_node->deleted != SIMTIME_ZERO)
                    continue;

                if (gw_node->orig_node == orig_node) {
                    //addr_to_string(gw_node->orig_node->orig, orig_str, ADDR_STR_LEN);
                    EV_DETAIL << "Removing gateway " << orig_node->orig << " from gateway list \n";

                    gw_node->deleted = getTime();
                    gw_purged = 1;

                    break;
                }
            }

            update_routes(orig_node, nullptr, nullptr, 0);

            delete orig_node;
            continue;
        } else {
            best_neigh_node = nullptr;
            max_tq = neigh_purged = 0;

            /* for all neighbours towards this originator ... */
            for (unsigned int j = 0; j < orig_node->neigh_list.size();) {
                neigh_node = orig_node->neigh_list[j];

                if (curr_time > (neigh_node->last_valid + purge_timeout)) {
                    if (orig_node->router == neigh_node) {
                        /* we have to delete the route towards this node before it gets purged */
                        //debug_output(4, "Deleting previous route \n");

                        /* remove old announced network(s) */
                        hna_global_del(orig_node);
                        add_del_route(orig_node->orig, 32, orig_node->router->addr, orig_node->batmanIf->if_index, orig_node->batmanIf->dev, BATMAN_RT_TABLE_HOSTS, ROUTE_TYPE_UNICAST, ROUTE_DEL);

                        /* if the neighbour is the route towards our gateway */
                        if ((curr_gateway != nullptr) && (curr_gateway->orig_node == orig_node))
                            del_default_route();

                        orig_node->router = nullptr;
                    }

                    neigh_node->owner_node = nullptr;
                    orig_node->neigh_list.erase(orig_node->neigh_list.begin()+j);
                    delete neigh_node;
                } else {
                    if ((best_neigh_node == nullptr) || (neigh_node->tq_avg > max_tq)) {
                        best_neigh_node = neigh_node;
                        max_tq = neigh_node->tq_avg;
                    }

                    j++;
                }
            }

            if ((neigh_purged) && ((best_neigh_node == nullptr) || (orig_node->router == nullptr) || (max_tq > orig_node->router->tq_avg)))
            {
                HnaElement * buffer = new HnaElement[orig_node->hna_buff.size()];
                for (unsigned int i=0; i<orig_node->hna_buff.size(); i++)
                    buffer[i] = (orig_node->hna_buff[i]);
                update_routes(orig_node, best_neigh_node, buffer, orig_node->hna_buff.size());
                delete [] buffer;
            }
        }
        it++;
    }

    for (unsigned int i = 0; i<gw_list.size();) {
        gw_node = gw_list[i];
        if ((gw_node->deleted>0) && (curr_time > (gw_node->deleted + (2 * purge_timeout)))) {
            gw_list.erase(gw_list.begin()+i);
            delete gw_node;
            continue; // other iteration
        }
        i++;
    }

    if (gw_purged)
        choose_gw();
}


OrigNode::OrigNode()
{
    clear();
}

void OrigNode::clear()
{
    router = nullptr;
    batmanIf = nullptr;
    totalRec = 0;
    for (auto & elem : bcast_own)
       elem = 0;
    for (auto & elem : bcast_own_sum)
       elem = 0;
    tq_own = 0;
    tq_asym_penalty = 0;
    last_valid = 0;        /* when last packet from this node was received */
    gwflags = 0;      /* flags related to gateway functions: gateway class */
    hna_buff.clear();
    last_real_seqno = 0;   /* last and best known squence number */
    last_ttl = 0;         /* ttl of last received packet */
    num_hops = MAX_HOPS;
    neigh_list.clear();
}

std::string OrigNode::info() const
{
    std::stringstream out;
    out << "orig:"  << orig << "  ";
    out << "totalRec:"  << totalRec << "  ";
    if (bcast_own[0])
      out << "bcast_own:" << bcast_own[0]<< "  ";
    else
        out << "bcast_own: *  ";
    if (bcast_own_sum[0])
      out << "bcast_own_sum:" << bcast_own_sum[0]<< "  ";
    else
        out << "bcast_own_sum: * ";

    out << "tq_own:" << (int)tq_own<< "  ";
    out << "tq_asym_penalty:" <<  (int)tq_asym_penalty<< "  ";
    out << "last_valid:" << last_valid;        /* when last packet from this node was received */
    out << "num_hops:" << num_hops;
    out << " \n neig info: \n";

    NeighNode *neigh_node = nullptr;
    for (auto & elem : neigh_list)
    {
        NeighNode * tmp_neigh_node = elem;
        if (tmp_neigh_node->addr == orig)
            neigh_node = tmp_neigh_node;
    }
    if (!neigh_node)
        out << "*";
    else
        out << neigh_node->info();

    for (auto & elem : neigh_list)
    {
        out << "list neig :" << elem->addr << " ";
    }

    out << "\n router info:"; if (router==nullptr) out << "*  "; else out << router->info() << "  ";
    return out.str();
}

std::string NeighNode::info() const
{
    std::stringstream out;
    out << "addr:"  << addr << "  ";
    out << "real_packet_count:" << real_packet_count<< "  ";
    out <<  "last_ttl:" << last_ttl<< "  ";
    out <<  "num_hops:" << num_hops<< "  ";
    out <<  "last_valid:" << last_valid<< "  ";            /* when last packet via this neighbour was received */
    out <<  "real_bits:" << real_bits[0]<< "  ";
    out <<  "orig_node :" << orig_node->orig << "  ";
    out <<  "owner_node :" << owner_node->orig << "  ";
    return out.str();
}
/*
OrigNode::OrigNode(const OrigNode &other)
{
    setName(other.getName());
    router=other.router;
    batmanIf=other.batmanIf;
    bcast_own=other.bcast_own;
    bcast_own_sum=other.bcast_own_sum;
    bcast_own = other.bcast_own;
    bcast_own_sum = other.bcast_own_sum;
    tq_own=other.tq_own;
    tq_asym_penalty=other.tq_asym_penalty;
    last_valid=other.last_valid;
    gwflags=other.gwflags;
    hna_buff=nullptr;
    last_real_seqno=0;
    last_ttl=0;
    neigh_list = other.neigh_list;
}
*/

OrigNode::~OrigNode()
{
    router = nullptr;
    batmanIf = nullptr;
    bcast_own.clear();
    bcast_own_sum.clear();
    hna_buff.clear();

    while (!neigh_list.empty())
    {
        NeighNode *node = neigh_list.back();
        neigh_list.pop_back();
        node->owner_node = nullptr;
        delete node;
    }

    hna_buff.clear();
}

void NeighNode::clear()
{
    addr = L3Address();
    real_packet_count = 0;
    for (auto & elem : tq_recv)
         elem = 0;
    tq_index = 0;
    tq_avg = 0;
    last_ttl = 0;
    num_hops = MAX_HOPS;
    last_valid = 0;            /* when last packet via this neighbour was received */
    for (auto & elem : real_bits)
        elem = 0;
    orig_node = nullptr;
    owner_node = nullptr;
    if_incoming = nullptr;
}


NeighNode::~NeighNode()
{
    tq_recv.clear();
    real_bits.clear();
    if (owner_node)
    {
        for (unsigned int i=0; i<owner_node->neigh_list.size(); i++)
        {
            if (this == owner_node->neigh_list[i])
            {
                owner_node->neigh_list.erase(owner_node->neigh_list.begin()+i);
                return;
            }
        }
    }
}


NeighNode::NeighNode(OrigNode* origNode, OrigNode *orig_neigh_node, const L3Address &neigh, BatmanIf* ifIncoming, const uint32_t &num_words, const uint32_t &global_win_size)
{
    tq_recv.resize(sizeof(uint16_t) * global_win_size);
    real_bits.resize(num_words);
    clear();
    addr = neigh;
    orig_node = orig_neigh_node;
    if_incoming = ifIncoming;
    owner_node = origNode;
    origNode->neigh_list.push_back(this);
}

} // namespace inetmanet

} // namespace inet

