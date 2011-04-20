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


#include "BatmanData.h"
#include "batman.h"


OrigNode::OrigNode() 
{
    clear();
}

void OrigNode::clear()
{
    router=NULL;
    batmanIf=NULL;
    totalRec=0;
    for (unsigned int i=0;i<bcast_own.size();i++)
       bcast_own[i]=0;
    for (unsigned int i=0;i<bcast_own_sum.size();i++)
       bcast_own_sum[i]=0;
    tq_own=0;
    tq_asym_penalty=0;
    last_valid=0;        /* when last packet from this node was received */
    gwflags=0;      /* flags related to gateway functions: gateway class */
    hna_buff.clear();
    last_real_seqno=0;   /* last and best known squence number */
    last_ttl=0;         /* ttl of last received packet */
    num_hops =MAX_HOPS;
    neigh_list.clear();
}

std::string OrigNode::info() const
{
    std::stringstream out;
    out << "orig:"  << orig.getIPAddress() << "  ";
    out << "totalRec:"  << this->totalRec << "  ";
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
    out << " \n neig info: \n" ;

    NeighNode *neigh_node=NULL;
    for (unsigned int i = 0; i< neigh_list.size();i++)
    {
    	NeighNode * tmp_neigh_node = neigh_list[i];
        if (tmp_neigh_node->addr == orig )
            neigh_node = tmp_neigh_node;

    }
    if (!neigh_node)
    	out << "*";
    else
    	out << neigh_node->info();

    for (unsigned int i = 0; i< neigh_list.size();i++)
    {
       	out << "list neig :" << neigh_list[i]->addr.getIPAddress() << " ";
    }

    out << "\n router info:"; if (router==NULL) out << "*  "; else out << router->info() << "  ";
    return out.str();
}

std::string NeighNode::info() const
{
    std::stringstream out;
    out << "addr:"  << addr.getIPAddress() << "  ";
    out << "real_packet_count:" << real_packet_count<< "  ";
    out <<  "last_ttl:" << last_ttl<< "  ";
    out <<  "num_hops:" << num_hops<< "  ";
    out <<  "last_valid:" << last_valid<< "  ";            /* when last packet via this neighbour was received */
    out <<  "real_bits:" << real_bits[0]<< "  ";
    out <<  "orig_node :" << orig_node->orig.getIPAddress()<< "  ";
    out <<  "owner_node :" << owner_node->orig.getIPAddress()<< "  ";
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
    hna_buff=NULL;
    last_real_seqno=0;
    last_ttl=0;
    neigh_list = other.neigh_list;
}
*/

OrigNode::~OrigNode() 
{
    router=NULL;
    batmanIf=NULL;
    bcast_own.clear();
    bcast_own_sum.clear();
    hna_buff.clear();

    while (!neigh_list.empty())
    {
        NeighNode *node = neigh_list.back();
        neigh_list.pop_back();
        node->owner_node=NULL;
        delete node;
    }
    while (!hna_buff.empty())
    {
        BatmanHnaMsg * aux=hna_buff.back();
        hna_buff.pop_back();
        delete aux;
    }
}

void NeighNode::clear()
{
    addr=(Uint128)0;
    real_packet_count=0;
    for (unsigned int i=0;i<tq_recv.size();i++)
         tq_recv[i]=0;
    tq_index=0;
    tq_avg=0;
    last_ttl=0;
    num_hops =MAX_HOPS;
    last_valid=0;            /* when last packet via this neighbour was received */
    for (unsigned int i=0;i<real_bits.size();i++)
        real_bits[i]=0;
    orig_node=NULL;
    owner_node=NULL;
    if_incoming=NULL;
}


NeighNode::~NeighNode()
{
    tq_recv.clear();
    real_bits.clear();
    if (this->owner_node)
    {
        for (unsigned int i=0; i<this->owner_node->neigh_list.size(); i++)
        {
            if (this == this->owner_node->neigh_list[i])
            {
                this->owner_node->neigh_list.erase(this->owner_node->neigh_list.begin()+i);
                return;
            }
        }
    }
}


NeighNode::NeighNode(OrigNode* origNode, OrigNode *orig_neigh_node, const Uint128 &neigh, BatmanIf* ifIncoming,const uint32_t &num_words, const uint32_t &global_win_size)
{
    tq_recv.resize(sizeof(uint16_t) * global_win_size);
    real_bits.resize(num_words);
    clear();
    this->addr = neigh;
    this->orig_node = orig_neigh_node;
    this->if_incoming = ifIncoming;
    this->owner_node=origNode;
    origNode->neigh_list.push_back(this);
}



/* this function finds or creates an originator entry for the given address if it does not exits */
OrigNode  * Batman::getOrigNode(const Uint128 &addr)
{
    OrigNode *orig_node;
    OrigMap::iterator it;
    it = origMap.find(addr);
    if (it!=origMap.end())
        return it->second;

    orig_node = new OrigNode();
    orig_node->bcast_own.resize(found_ifs * num_words);
    orig_node->bcast_own_sum.resize(found_ifs);
    orig_node->clear();
    orig_node->orig=addr;
    origMap.insert(std::pair<Uint128, OrigNode *>(addr,orig_node));
    return orig_node;
}

NeighNode * Batman::create_neighbor(OrigNode *orig_node, OrigNode *orig_neigh_node, const Uint128 &neigh, BatmanIf* if_incoming)
{
    return new NeighNode(orig_node,orig_neigh_node,neigh,if_incoming,num_words,global_win_size);
}

OrigNode * Batman::get_orig_node(const Uint128 &addr )
{
    return getOrigNode(addr);
}

void Batman::ring_buffer_set(std::vector<uint8_t> &tq_recv, uint8_t &tq_index, uint8_t value)
{
    tq_recv[tq_index] = value;
    tq_index = (tq_index + 1) % global_win_size;
}

uint8_t Batman::ring_buffer_avg(std::vector<uint8_t> &tq_recv)
{
    uint16_t count = 0;
    uint32_t sum = 0;
    for (unsigned int i=0;i < tq_recv.size();i++)
    {
        if (tq_recv[i] != 0) {
            count++;
            sum += tq_recv[i];
        }
    }
    if (count == 0)
        return 0;
    return (uint8_t)(sum / count);
}


void Batman::update_orig(OrigNode *orig_node, BatmanPacket *in, const Uint128 &neigh,BatmanIf *if_incoming, BatmanHnaMsg *hna_recv_buff, int16_t hna_buff_len, uint8_t is_duplicate, const simtime_t &curr_time )
{
    GwNode *gw_node;
    NeighNode *neigh_node = NULL, *tmp_neigh_node = NULL, *best_neigh_node = NULL;
    uint8_t max_bcast_own = 0, max_tq = 0;


    for (unsigned int i=0;i<orig_node->neigh_list.size();i++)
    {
        tmp_neigh_node = orig_node->neigh_list[i];
        if ( ( tmp_neigh_node->addr == neigh ) && ( tmp_neigh_node->if_incoming == if_incoming ) )
        {
            neigh_node = tmp_neigh_node;
        }
        else
        {
            if ( !is_duplicate )
            {
                ring_buffer_set(tmp_neigh_node->tq_recv, tmp_neigh_node->tq_index, 0);
                tmp_neigh_node->tq_avg = ring_buffer_avg(tmp_neigh_node->tq_recv);
            }
            /* if we got have a better tq value via this neighbour or same tq value if it is currently our best neighbour (to avoid route flipping) */
            if ( ( tmp_neigh_node->tq_avg > max_tq ) || ( ( tmp_neigh_node->tq_avg == max_tq ) && ( tmp_neigh_node->orig_node->bcast_own_sum[if_incoming->if_num] > max_bcast_own ) ) || ( ( orig_node->router == tmp_neigh_node ) && ( tmp_neigh_node->tq_avg == max_tq ) ) ) {

                max_tq = tmp_neigh_node->tq_avg;
                max_bcast_own = tmp_neigh_node->orig_node->bcast_own_sum[if_incoming->if_num];
                best_neigh_node = tmp_neigh_node;
            }
        }
    }

    if ( neigh_node == NULL ) {

        neigh_node = new NeighNode(orig_node, get_orig_node(neigh), neigh, if_incoming,num_words,global_win_size);
    }

    neigh_node->last_valid = curr_time;

    ring_buffer_set(neigh_node->tq_recv, neigh_node->tq_index, in->getTq());
    neigh_node->tq_avg = ring_buffer_avg(neigh_node->tq_recv);

/*     is_new_seqno = bit_get_packet( neigh_node->seq_bits, in->seqno - orig_node->last_seqno, 1 );
     is_new_seqno = ! get_bit_status( neigh_node->real_bits, orig_node->last_real_seqno, in->seqno ); */


    if ( !is_duplicate )
    {
        orig_node->last_ttl = in->getTtl();
        neigh_node->num_hops = in->getHops();
        neigh_node->last_ttl = in->getTtl();
    }

    if (( neigh_node->tq_avg > max_tq ) || ((neigh_node->tq_avg == max_tq) && ( neigh_node->orig_node->bcast_own_sum[if_incoming->if_num] > max_bcast_own ) ) || ( ( orig_node->router == neigh_node ) && ( neigh_node->tq_avg == max_tq ) ) )
    {
        best_neigh_node = neigh_node;
    }

    /* update routing table and check for changed hna announcements */
    update_routes(orig_node, best_neigh_node, hna_recv_buff, hna_buff_len );

    if ( orig_node->gwflags != in->getGatewayFlags() )
        update_gw_list( orig_node, in->getGatewayFlags(), in->getGatewayPort());

    orig_node->gwflags = in->getGatewayFlags();
    hna_global_check_tq(orig_node);

    /* restart gateway selection if we have more packets and fast or late switching enabled */
    if ((routing_class > 2) && (orig_node->gwflags != 0) && (curr_gateway != NULL)) {

        /* if the node is not our current gateway and
           we have preferred gateray disabled and a better tq value or we found our preferred gateway */
        if ((curr_gateway->orig_node != orig_node) &&
                   (((pref_gateway == 0) && (orig_node->router->tq_avg > curr_gateway->orig_node->router->tq_avg)) || (pref_gateway == orig_node->orig))) {

            /* it is our preferred gateway or we have fast switching or the tq is $routing_class better than our old tq */
            if ((pref_gateway == orig_node->orig) || (routing_class == 3) || (orig_node->router->tq_avg - curr_gateway->orig_node->router->tq_avg >= routing_class)) {
                gw_node = NULL;
                for (unsigned int i = 0;i<gw_list.size();i++)
                {
                    if (gw_list[i]->orig_node == orig_node)
                    {
                        gw_node = gw_list[i];
                        break;
                    }
                }
                /* if this gateway had not a gateway failure within the last 30 seconds */
                if ((gw_node != NULL) && (curr_time > (gw_node->last_failure + 30)))
                {
                    del_default_route();
                }

            }

        }

    }
}


void Batman::purge_orig(const simtime_t &curr_time)
{
    OrigNode * orig_node;
    NeighNode * neigh_node, *best_neigh_node;
    GwNode *gw_node;
    uint8_t gw_purged = 0, neigh_purged, max_tq;

    /* for all origins... */
    OrigMap::iterator it;
    OrigMap::iterator itAux;

    for (it = origMap.begin();it != origMap.end();)
    {
        orig_node = it->second;
        if (curr_time > (orig_node->last_valid + (2 * purge_timeout)))
        {
            if (it != origMap.begin())
            {
                itAux = it;
                it++;
                origMap.erase(itAux);
            }
            else
            {
                origMap.erase(it);
                it = origMap.begin();
            }

            /* for all neighbours towards this originator ... */
            while (!orig_node->neigh_list.empty())
            {
                NeighNode *node = orig_node->neigh_list.back();
                orig_node->neigh_list.pop_back();
                node->owner_node=NULL;
                delete node;
            }

            for (unsigned int i =0;i<gw_list.size();i++)
            {
                gw_node = gw_list[i];
                if (gw_node->deleted.raw())
                    continue;
                if ( gw_node->orig_node == orig_node ) {
                    //addr_to_string( gw_node->orig_node->orig, orig_str, ADDR_STR_LEN );
                    //debug_output( 3, "Removing gateway %s from gateway list \n", orig_str );
                    gw_node->deleted = getTime();
                    gw_purged = 1;
                    break;
                }
            }
            update_routes( orig_node, NULL, NULL, 0 );
            delete orig_node;
            continue;
        }
        else
        {
            best_neigh_node = NULL;
            max_tq = neigh_purged = 0;
            for (unsigned int j = 0;j< orig_node->neigh_list.size();)
            {
                neigh_node = orig_node->neigh_list[j];
                if (curr_time > (neigh_node->last_valid + purge_timeout)) {
                    if (orig_node->router == neigh_node) {
                        /* remove old announced network(s) */
                        hna_global_del(orig_node);
                        add_del_route(orig_node->orig, 32, orig_node->router->addr, orig_node->batmanIf->if_index, orig_node->batmanIf->dev, BATMAN_RT_TABLE_HOSTS, ROUTE_TYPE_UNICAST, ROUTE_DEL);

                        /* if the neighbour is the route towards our gateway */
                        if ((curr_gateway != NULL) && (curr_gateway->orig_node == orig_node))
                            del_default_route();
                        orig_node->router = NULL;
                    }
                    neigh_node->owner_node=NULL;
                    orig_node->neigh_list.erase(orig_node->neigh_list.begin()+j);
                    delete neigh_node;
                } else {
                    if ((best_neigh_node == NULL) || (neigh_node->tq_avg > max_tq)) {
                        best_neigh_node = neigh_node;
                        max_tq = neigh_node->tq_avg;
                    }
                    j++;
                }

            }
            if ((neigh_purged) && ((best_neigh_node == NULL) || (orig_node->router == NULL) || (max_tq > orig_node->router->tq_avg)))
            {
                BatmanHnaMsg * buffer = new BatmanHnaMsg[orig_node->hna_buff.size()];
                for (unsigned int i=0;i<orig_node->hna_buff.size();i++)
                    buffer[i]= *(orig_node->hna_buff[i]);
                update_routes(orig_node, best_neigh_node, buffer, orig_node->hna_buff.size());
                delete [] buffer;
            }
        }
        it++;
    }

    for (unsigned int i =0;i<gw_list.size();)
    {
        gw_node = gw_list[i];
        if ((gw_node->deleted>0) && (curr_time > (gw_node->deleted + (2 * purge_timeout))))
        {
            gw_list.erase(gw_list.begin()+i);
            delete gw_node;
            continue; // other iteration
        }
        i++;
    }
    if ( gw_purged )
        choose_gw();
}



void Batman::choose_gw(void)
{
    GwNode *gw_node, *tmp_curr_gw = NULL;
    uint8_t max_gw_class = 0, max_tq = 0;
    uint32_t max_gw_factor = 0, tmp_gw_factor = 0;
    int download_speed, upload_speed;

    simtime_t current_time = getTime();

    if ((routing_class == 0) || ((routing_class < 4) && ((int64_t)(current_time.raw() - (originator_interval.raw() * local_win_size)) < 0))) {
        return;
    }

    if ( gw_list.empty() ) {
        if ( curr_gateway != NULL ) {
            //debug_output( 3, "Removing default route - no gateway in range\n" );
            del_default_route();
        }
        return;
    }

    for (unsigned int i = 0; i<gw_list.size();i++ )
    {
        gw_node = gw_list[i];

        /* ignore this gateway if recent connection attempts were unsuccessful */
        /* if it is our only gateway retry immediately */
        if (gw_list.size()) {
            if (current_time < (gw_node->last_failure + 30))
                continue;
        }

        if ( gw_node->orig_node->router == NULL )
            continue;

        if ( gw_node->deleted >0)
            continue;
        switch ( routing_class ) {
            case 1: /* fast connection */
                get_gw_speeds( gw_node->orig_node->gwflags, &download_speed, &upload_speed );

                if (((tmp_gw_factor = (((gw_node->orig_node->router->tq_avg * 100 ) / local_win_size) *
                                  ((gw_node->orig_node->router->tq_avg * 100) / local_win_size) *
                                     (download_speed / 64))) > max_gw_factor) ||
                                  ((tmp_gw_factor == max_gw_factor) && (gw_node->orig_node->router->tq_avg > max_tq)))
                    tmp_curr_gw = gw_node;
                break;
            default: /* stable connection (use best statistic) */
                 /* fast-switch (use best statistic but change as soon as a better gateway appears) */
                 /* late-switch (use best statistic but change as soon as a better gateway appears which has $routing_class more tq points) */
                if (gw_node->orig_node->router->tq_avg > max_tq)
                    tmp_curr_gw = gw_node;
                break;
        }

        if ( gw_node->orig_node->gwflags > max_gw_class )
            max_gw_class = gw_node->orig_node->gwflags;

        if (gw_node->orig_node->router->tq_avg > max_tq)
            max_tq = gw_node->orig_node->router->tq_avg;

        if ( tmp_gw_factor > max_gw_factor )
            max_gw_factor = tmp_gw_factor;

        if ( ( pref_gateway != 0 ) && ( pref_gateway == gw_node->orig_node->orig ) ) {

            tmp_curr_gw = gw_node;

//            addr_to_string( tmp_curr_gw->orig_node->orig, orig_str, ADDR_STR_LEN );
//            debug_output( 3, "Preferred gateway found: %s (gw_flags: %i, tq: %i, gw_product: %i)\n", orig_str, gw_node->orig_node->gwflags, gw_node->orig_node->router->tq_avg, tmp_gw_factor );

            break;
        }

    }


    if ( curr_gateway != tmp_curr_gw ) {

        if ( curr_gateway != NULL ) {

//            if ( tmp_curr_gw != NULL )
//                debug_output( 3, "Removing default route - better gateway found\n" );
//            else
//                debug_output( 3, "Removing default route - no gateway in range\n" );
            del_default_route();
        }

        curr_gateway = tmp_curr_gw;

        /* may be the last gateway is now gone */
        if ( ( curr_gateway != NULL ) && ( !is_aborted() ) ) {
//            addr_to_string( curr_gateway->orig_node->orig, orig_str, ADDR_STR_LEN );
//            debug_output( 3, "Adding default route to %s (gw_flags: %i, tq: %i, gw_product: %i)\n", orig_str, max_gw_class, max_tq, max_gw_factor );
            add_default_route();

        }
    }
}

void Batman::update_routes(OrigNode *orig_node, NeighNode *neigh_node, BatmanHnaMsg *hna_recv_buff, int16_t hna_buff_len)
{
    NeighNode *old_router;

    old_router = orig_node->router;

    /* also handles orig_node->router == NULL and neigh_node == NULL */
    if ((orig_node != NULL) && (orig_node->router != neigh_node))
    {

        /* adds duplicated code but makes it more readable */

        /* new route added */
        if ((orig_node->router == NULL) && (neigh_node != NULL)) {

            add_del_route(orig_node->orig, 32, neigh_node->addr,
                    neigh_node->if_incoming->if_index, neigh_node->if_incoming->dev, BATMAN_RT_TABLE_HOSTS, ROUTE_TYPE_UNICAST, ROUTE_ADD);
            orig_node->batmanIf = neigh_node->if_incoming;
            orig_node->router = neigh_node;

            /* add new announced network(s) */
            hna_global_add(orig_node, hna_recv_buff, hna_buff_len);

        /* route deleted */
        }
        else if ((orig_node->router != NULL) && (neigh_node == NULL))
        {

            EV << "Deleting previous route\n";

            /* remove old announced network(s) */
            hna_global_del(orig_node);
            add_del_route(orig_node->orig, 32, orig_node->router->addr, orig_node->batmanIf->if_index,
                    orig_node->batmanIf->dev, BATMAN_RT_TABLE_HOSTS, ROUTE_TYPE_UNICAST, ROUTE_DEL);
            orig_node->router = neigh_node;

        /* route changed */
        }
        else
        {
            bool Change=true;
#ifdef __USE_MINHOP__
            if (orig_node->router->num_hops<neigh_node->num_hops)
            {
                 // evaluate
                 if (neigh_node->tq_avg<=orig_node->router->tq_avg+1)
                    Change=false;
            }
            else if (orig_node->router->num_hops==neigh_node->num_hops)
            {
                if (neigh_node->tq_avg==orig_node->router->tq_avg+1)
                   Change=false;
            }
#endif
            /* add new route */
            if (Change)
            {
                 add_del_route(orig_node->orig, 32, neigh_node->addr,
                      neigh_node->if_incoming->if_index, neigh_node->if_incoming->dev, BATMAN_RT_TABLE_HOSTS, ROUTE_TYPE_UNICAST, ROUTE_ADD);

            /* delete old route */ // Not necessary ADD delete the old route before write
            // add_del_route(orig_node->orig, 32, orig_node->router->addr, orig_node->batmanIf->if_index,
            //        orig_node->batmanIf->dev, BATMAN_RT_TABLE_HOSTS, ROUTE_TYPE_UNICAST, ROUTE_DEL);

                 orig_node->batmanIf = neigh_node->if_incoming;
                 orig_node->router = neigh_node;
                 orig_node->num_hops = neigh_node->num_hops+1;
            /* update announced network(s) */
                 hna_global_update(orig_node, hna_recv_buff, hna_buff_len, old_router);
            }
        }

    } else if (orig_node != NULL) {
        hna_global_update(orig_node, hna_recv_buff, hna_buff_len, old_router);
    }
    // Sanity check
    if (!isInMacLayer())
    {
    	Uint128 next = omnet_exist_rte (orig_node->orig);
        if(orig_node->router)
        {
            if (next!=orig_node->router->addr)
                add_del_route(orig_node->orig, 32, orig_node->router->addr,
            		 orig_node->router->if_incoming->if_index, orig_node->router->if_incoming->dev, BATMAN_RT_TABLE_HOSTS, ROUTE_TYPE_UNICAST, ROUTE_ADD);
        }
        else
        {
            if (next.getIPAddress()!=IPAddress::ALLONES_ADDRESS)
                add_del_route(orig_node->orig, 32, next, orig_node->batmanIf->if_index,
                      orig_node->batmanIf->dev, BATMAN_RT_TABLE_HOSTS, ROUTE_TYPE_UNICAST, ROUTE_DEL);
        }
    }
}


void Batman::update_gw_list(OrigNode *orig_node, uint8_t new_gwflags, uint16_t gw_port)
{
    GwNode *gw_node;

    for (unsigned int i = 0; i<gw_list.size();i++ )
    {
        gw_node = gw_list[i];
        if ( gw_node->orig_node == orig_node )
        {
            if ( new_gwflags == 0 ) {
                gw_node->deleted = getTime();
                gw_node->orig_node->gwflags = new_gwflags;
                if (gw_node == curr_gateway)
                    choose_gw();

            }
            else
            {
                gw_node->deleted = 0;
                gw_node->orig_node->gwflags = new_gwflags;
            }
            return;
        }
    }

    gw_node = new GwNode();
    memset(gw_node, 0, sizeof(GwNode));

    gw_node->orig_node = orig_node;
    gw_node->gw_port = gw_port;
    gw_node->last_failure = getTime();
    gw_list.push_back(gw_node);
}




/* returns the up and downspeeds in kbit, calculated from the class */
void Batman::get_gw_speeds(unsigned char gw_class, int *down, int *up)
{
    char sbit = (gw_class & 0x80) >> 7;
    char dpart = (gw_class & 0x7C) >> 3;
    char upart = (gw_class & 0x07);

    *down = 32 * (sbit + 2) * (1 << dpart);
    *up = ((upart + 1) * (*down)) / 8;
}



/* calculates the gateway class from kbit */
unsigned char Batman::get_gw_class(int down, int up)
{
    int mdown = 0, tdown, tup, difference = 0x0FFFFFFF;
    unsigned char gw_class = 0, sbit, part;


    /* test all downspeeds */
    for (sbit = 0; sbit < 2; sbit++) {

        for (part = 0; part < 16; part++) {

            tdown = 32 * (sbit + 2) * (1 << part);

            if ( abs(tdown - down) < difference) {

                gw_class = (sbit << 7) + (part << 3);
                difference = abs(tdown - down);
                mdown = tdown;

            }

        }

    }

    /* test all upspeeds */
    difference = 0x0FFFFFFF;

    for (part = 0; part < 8; part++) {

        tup = ((part + 1) * (mdown)) / 8;

        if (abs(tup - up) < difference) {

            gw_class = (gw_class & 0xF8) | part;
            difference = abs(tup - up);

        }

    }

    return gw_class;
}



int Batman::isBidirectionalNeigh(OrigNode *orig_node, OrigNode *orig_neigh_node, BatmanPacket *in, const simtime_t &recv_time, BatmanIf *if_incoming)
{

    NeighNode *neigh_node = NULL, *tmp_neigh_node = NULL;
    uint8_t total_count;

    if ( orig_node == orig_neigh_node )
    {
        for (unsigned int i = 0; i< orig_node->neigh_list.size();i++ )
        {
            tmp_neigh_node = orig_node->neigh_list[i];
            if ( ( tmp_neigh_node->addr == orig_neigh_node->orig ) && ( tmp_neigh_node->if_incoming == if_incoming ) )
                neigh_node = tmp_neigh_node;
        }

        if ( neigh_node == NULL )
            neigh_node = create_neighbor(orig_node, orig_neigh_node, orig_neigh_node->orig, if_incoming);

        neigh_node->last_valid = recv_time;

    }
    else
    {

        /* find packet count of corresponding one hop neighbor */
        for (unsigned int i = 0; i< orig_neigh_node->neigh_list.size();i++)
        {
            tmp_neigh_node = orig_neigh_node->neigh_list[i];
            if ( ( tmp_neigh_node->addr == orig_neigh_node->orig ) && ( tmp_neigh_node->if_incoming == if_incoming ) )
                neigh_node = tmp_neigh_node;

        }

        if ( neigh_node == NULL )
            neigh_node = create_neighbor(orig_neigh_node, orig_neigh_node, orig_neigh_node->orig, if_incoming);
    }

    orig_node->last_valid = recv_time;

    /* pay attention to not get a value bigger than 100 % */
    total_count = ( orig_neigh_node->bcast_own_sum[if_incoming->if_num] > neigh_node->real_packet_count ? neigh_node->real_packet_count : orig_neigh_node->bcast_own_sum[if_incoming->if_num] );

    /* if we have too few packets (too less data) we set tq_own to zero */
    /* if we receive too few packets it is not considered bidirectional */
    if ( ( total_count < minimum_send ) || ( neigh_node->real_packet_count < minimum_recv ) )
    {
        orig_neigh_node->tq_own = 0;

    }
    else
    {

        /* neigh_node->real_packet_count is never zero as we only purge old information when getting new information */
        orig_neigh_node->tq_own = (TQ_MAX_VALUE * total_count) / neigh_node->real_packet_count;

    }

    /* 1 - ((1-x)** 3), normalized to TQ_MAX_VALUE */
    /* this does affect the nearly-symmetric links only a little,
     * but punishes asymetric links more. */
    /* this will give a value between 0 and TQ_MAX_VALUE */
    orig_neigh_node->tq_asym_penalty = TQ_MAX_VALUE - (TQ_MAX_VALUE *
            (local_win_size - neigh_node->real_packet_count) *
            (local_win_size - neigh_node->real_packet_count) *
            (local_win_size - neigh_node->real_packet_count)) /
            (local_win_size * local_win_size * local_win_size);

    in->setTq ((in->getTq() * orig_neigh_node->tq_own * orig_neigh_node->tq_asym_penalty) / (TQ_MAX_VALUE *  TQ_MAX_VALUE));
/*
    addr_to_string( orig_node->orig, orig_str, ADDR_STR_LEN );
    addr_to_string( orig_neigh_node->orig, neigh_str, ADDR_STR_LEN );

    debug_output(4, "bidirectional: orig = %-15s neigh = %-15s => own_bcast = %2i, real recv = %2i, local tq: %3i, asym_penalty: %3i, total tq: %3i \n",
              orig_str, neigh_str, total_count, neigh_node->real_packet_count, orig_neigh_node->tq_own, orig_neigh_node->tq_asym_penalty, in->tq);
*/
    /* if link has the minimum required transmission quality consider it bidirectional */
    if (in->getTq() >= TQ_TOTAL_BIDRECT_LIMIT)
        return 1;

    return 0;
}
#if 0
static void generate_vis_packet(void)
{
    struct hash_it_t *hashit = NULL;
    struct orig_node *orig_node;
    struct vis_data *vis_data;
    struct list_head *list_pos;
    struct batman_if *batman_if;

    if (vis_packet != NULL) {
        debugFree(vis_packet, 1102);
        vis_packet = NULL;
        vis_packet_size = 0;
    }

    vis_packet_size = sizeof(struct vis_packet);
    vis_packet = debugMalloc(vis_packet_size, 104);

    memcpy(&((struct vis_packet *)vis_packet)->sender_ip, (unsigned char *)&(((struct batman_if *)if_list.next)->addr.sin_addr.s_addr), 4);

    ((struct vis_packet *)vis_packet)->version = VIS_COMPAT_VERSION;
    ((struct vis_packet *)vis_packet)->gw_class = gateway_class;
    ((struct vis_packet *)vis_packet)->tq_max = TQ_MAX_VALUE;

    /* neighbor list */
    while (NULL != (hashit = hash_iterate(orig_hash, hashit))) {

        orig_node = hashit->bucket->data;

        /* we interested in 1 hop neighbours only */
        if ((orig_node->router != NULL) && (orig_node->orig == orig_node->router->addr) &&
            (orig_node->router->tq_avg > 0)) {

            vis_packet_size += sizeof(struct vis_data);

            vis_packet = debugRealloc(vis_packet, vis_packet_size, 105);

            vis_data = (struct vis_data *)(vis_packet + vis_packet_size - sizeof(struct vis_data));

            memcpy(&vis_data->ip, (unsigned char *)&orig_node->orig, 4);

            vis_data->data = orig_node->router->tq_avg;
            vis_data->type = DATA_TYPE_NEIGH;

        }

    }

    /* secondary interfaces */
    if (found_ifs > 1) {
        list_for_each(list_pos, &if_list) {
            batman_if = list_entry(list_pos, struct batman_if, list);

            if (((struct vis_packet *)vis_packet)->sender_ip == batman_if->addr.sin_addr.s_addr)
                continue;

            vis_packet_size += sizeof(struct vis_data);

            vis_packet = debugRealloc(vis_packet, vis_packet_size, 106);

            vis_data = (struct vis_data *)(vis_packet + vis_packet_size - sizeof(struct vis_data));

            memcpy(&vis_data->ip, (unsigned char *)&batman_if->addr.sin_addr.s_addr, 4);

            vis_data->data = 0;
            vis_data->type = DATA_TYPE_SEC_IF;
        }
    }

    /* hna announcements */
    vis_packet = hna_local_update_vis_packet(vis_packet, &vis_packet_size);

    if (vis_packet_size == sizeof(struct vis_packet)) {
        debugFree(vis_packet, 1107);
        vis_packet = NULL;
        vis_packet_size = 0;
    }
}

static void send_vis_packet(void)
{
    generate_vis_packet();

    if ( vis_packet != NULL )
        send_udp_packet(vis_packet, vis_packet_size, vis_if.addr, vis_if.sock, NULL);
}
#endif
uint8_t Batman::count_real_packets(BatmanPacket *in, const Uint128 &neigh, BatmanIf *if_incoming)
{
    OrigNode *orig_node;
    NeighNode *tmp_neigh_node;
    uint8_t is_duplicate = 0;

    orig_node = getOrigNode(in->getOrig());

    /*char orig_str[ADDR_STR_LEN], neigh_str[ADDR_STR_LEN];

    addr_to_string( in->orig, orig_str, ADDR_STR_LEN );
    addr_to_string( neigh, neigh_str, ADDR_STR_LEN );

    debug_output( 3, "count_real_packets: orig = %s, neigh = %s, seq = %i, last seq = %i\n", orig_str, neigh_str, in->seqno, orig_node->last_real_seqno );*/

    for (unsigned int i= 0;i<orig_node->neigh_list.size();i++)
    {
        tmp_neigh_node = orig_node->neigh_list[i];
        if ( !is_duplicate )
            is_duplicate = get_bit_status(tmp_neigh_node->real_bits, orig_node->last_real_seqno, in->getSeqNumber() );

        if ( ( tmp_neigh_node->addr == neigh ) && ( tmp_neigh_node->if_incoming == if_incoming ) ) {

            bit_get_packet(tmp_neigh_node->real_bits, in->getSeqNumber() - orig_node->last_real_seqno, 1 );
            /*debug_output( 3, "count_real_packets (yes): neigh = %s, is_new = %s, seq = %i, last seq = %i\n", neigh_str, ( is_new_seqno ? "YES" : "NO" ), in->seqno, orig_node->last_real_seqno );*/

        }
        else
        {
            bit_get_packet(tmp_neigh_node->real_bits, in->getSeqNumber() - orig_node->last_real_seqno, 0 );
            /*debug_output( 3, "count_real_packets (no): neigh = %s, is_new = %s, seq = %i, last seq = %i\n", neigh_str, ( is_new_seqno ? "YES" : "NO" ), in->seqno, orig_node->last_real_seqno );*/
        }

        tmp_neigh_node->real_packet_count = bit_packet_count(tmp_neigh_node->real_bits);
    }

    if ( !is_duplicate )
    {

        EV << "updating last_seqno: old" << orig_node->last_real_seqno <<" new "<< in->getSeqNumber() << "\n" ;
        orig_node->last_real_seqno = in->getSeqNumber();

    }
    return is_duplicate;
}

void Batman::schedule_own_packet(BatmanIf *batman_if)
{
    ForwNode *forw_node_new=NULL;
    OrigNode *orig_node;

    forw_node_new = new ForwNode;

    do
    {
       forw_node_new->send_time = getTime() + originator_interval + par("jitter2");
    } while (forw_node_new->send_time < simTime());// avoid schedule in the past

    EV << "Send own packet in "<< forw_node_new->send_time <<endl;

    forw_node_new->if_incoming = batman_if;
    forw_node_new->own = 1;
    forw_node_new->num_packets = 0;
    forw_node_new->direct_link_flags = 0;
    forw_node_new->pack_buff = buildDefaultBatmanPkt(batman_if);
    forw_node_new->pack_buff->setHnaLen(0);

    /* non-primary interfaces do not send hna information */
    if ((hna_list.size() > 0) && (batman_if->if_num == 0))
    {
        forw_node_new->pack_buff->setHnaMsgArraySize(hna_buff_local.size());
        forw_node_new->pack_buff->setHnaLen(hna_buff_local.size());
        forw_node_new->pack_buff->setByteLength(forw_node_new->pack_buff->getByteLength()+(hna_buff_local.size() * 5));
        forw_node_new->pack_buff->setHnaLen(hna_buff_local.size());
        for (unsigned int i = 0;i<hna_buff_local.size();i++)
        {
            BatmanHnaMsg aux;
            aux.addr=hna_buff_local[i].addr;
            aux.netmask=hna_buff_local[i].netmask;

            forw_node_new->pack_buff->setHnaMsg(i,aux);

        }
    }

    /* change sequence number to network order */

    Forwlist::iterator forwListIt;
    for (forwListIt = forw_list.begin();forwListIt != forw_list.end();forwListIt++)
    {
        ForwNode *forw_packet_tmp = *forwListIt;
        if (forw_packet_tmp->send_time > forw_node_new->send_time)
        {
            forw_list.insert(forwListIt,forw_node_new);
            break;
        }
    }

    if (forwListIt == forw_list.end())
    {
          forw_list.push_back(forw_node_new);
    }

    batman_if->seqno++;
    for (OrigMap::iterator it = origMap.begin();it!=origMap.end();it++)
    {
        orig_node = it->second;
        //debug_output( 4, "count own bcast (schedule_own_packet): old = %i, ", orig_node->bcast_own_sum[batman_if->if_num] );

        std::vector<TYPE_OF_WORD>vectorAux;
        for (unsigned int i=0;i<num_words;i++)
        {
        	vectorAux.push_back(orig_node->bcast_own[(batman_if->if_num * num_words)+i]);
        }
        bit_get_packet(vectorAux, 1, 0 );
        orig_node->bcast_own_sum[batman_if->if_num] = bit_packet_count(vectorAux);
        for (unsigned int i=0;i<num_words;i++)
        {
        	orig_node->bcast_own[(batman_if->if_num * num_words)+i] = vectorAux[i];
        }
        vectorAux.clear();
        //debug_output( 4, "new = %i \n", orig_node->bcast_own_sum[batman_if->if_num] );
    }
}



void Batman::schedule_forward_packet(OrigNode *orig_node, BatmanPacket *in, const Uint128 &neigh, uint8_t directlink, int16_t hna_buff_len, BatmanIf *if_incoming, const simtime_t &curr_time)
{
    ForwNode *forw_node_new = NULL, *forw_node_aggregate = NULL, *forw_node_pos = NULL;
    //struct list_head *list_pos = forw_list.next, *prev_list_head = (struct list_head *)&forw_list;
    BatmanPacket *bat_packet;
    uint8_t tq_avg = 0;
    simtime_t send_time;
    //prof_start(PROF_schedule_forward_packet);

    //debug_output(4, "schedule_forward_packet():  \n");

    if (in->getTtl() <= 1)
    {
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


    Forwlist::iterator  it;
    for (it=forw_list.begin();it!=forw_list.end();it++)
    {
        forw_node_pos = *it;
        if (aggregation_enabled) {

            /* don't save aggregation position if aggregation is disabled */
            forw_node_aggregate = forw_node_pos;

            /**
             * we can aggregate the current packet to this packet if:
             * - the send time is within our MAX_AGGREGATION_MS time
             * - the resulting packet wont be bigger than MAX_AGGREGATION_BYTES
             */
            if ((forw_node_pos->send_time < send_time)  &&
                (forw_node_pos->pack_buff->getByteLength() + in->getByteLength()  <= MAX_AGGREGATION_BYTES)) {

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

    if (forw_node_aggregate == NULL)
    {

        forw_node_new = new  ForwNode;
        forw_node_new->pack_buff = in;
        forw_node_new->own = 0;
        forw_node_new->if_incoming = if_incoming;
        forw_node_new->num_packets = 0;
        forw_node_new->direct_link_flags = 0;
        forw_node_new->send_time = send_time;

    }
    else
    {
        // It's necessary decapsulate and recapsulate the packets
        appendPacket(forw_node_aggregate->pack_buff,in);
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

            bat_packet->setTq (orig_node->router->tq_avg);
            bat_packet->setTtl (orig_node->router->last_ttl - 1);
            bat_packet->setHops(orig_node->router->num_hops+1);
        }

        tq_avg = orig_node->router->tq_avg;

    }

    /* apply hop penalty */
    bat_packet->setTq (bat_packet->getTq() * (TQ_MAX_VALUE - hop_penalty) / (TQ_MAX_VALUE));

    //debug_output(4, "forwarding: tq_orig: %i, tq_avg: %i, tq_forw: %i, ttl_orig: %i, ttl_forw: %i \n", in->tq, tq_avg, bat_packet->tq, in->ttl - 1, bat_packet->ttl);

    if (directlink)
        bat_packet->setFlags (bat_packet->getFlags()|DIRECTLINK);
    else
        bat_packet->setFlags (bat_packet->getFlags()&(~DIRECTLINK));

    /* if the packet was not aggregated */
    if (forw_node_aggregate == NULL) {

        /* if the packet should go somewhere in the queue */
        if (it!=forw_list.end())
            forw_list.insert(it,forw_node_new);
        /* if the packet is the last packet in the queue */
        else
            forw_list.push_back(forw_node_new);
    }
    EV << "Fordward packet " << bat_packet << "at :" <<forw_node_new->send_time<< endl;
}

void Batman::appendPacket(cPacket *oldPacket,cPacket * packetToAppend)
{
    if (oldPacket->getEncapsulatedPacket()==NULL)
    {
        oldPacket->encapsulate(packetToAppend);
        return;
    }
    std::vector<cPacket*> vectorPacket;
    cPacket * pkt = oldPacket;
    while (pkt->getEncapsulatedPacket())
    {
        vectorPacket.push_back(pkt->decapsulate());
        pkt = vectorPacket.back();
    }
    pkt = packetToAppend;
    while (!vectorPacket.empty())
    {
          cPacket *pktAux=vectorPacket.back();
          pktAux->encapsulate(pkt);
          pkt=pktAux;
          vectorPacket.pop_back();
    }
    oldPacket->encapsulate(pkt);
}

void Batman::send_outstanding_packets(const simtime_t &curr_time)
{
    ForwNode *forw_node= NULL;
    BatmanIf *batman_if;
    BatmanPacket *bat_packet;
    uint8_t directlink, curr_packet_num;
    int16_t curr_packet_len;

    //prof_start(PROF_send_outstanding_packets);


    while  (true)
    {
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
            ((forw_node->own) && (forw_node->if_incoming->if_num > 0)))
        {

            //debug_output(4, "%s packet (originator %s, seqno %d, TTL %d) on interface %s\n", (forw_node->own ? "Sending own" : "Forwarding"), orig_str, ntohs(bat_packet->seqno), bat_packet->ttl, forw_node->if_incoming->dev);

            if (send_udp_packet(forw_node->pack_buff->dup(), forw_node->pack_buff_len, forw_node->if_incoming->broad, BATMAN_PORT, forw_node->if_incoming) < 0)
                    deactivate_interface(forw_node->if_incoming);
        }
        else
        {
           for (unsigned int i =0;i<if_list.size();i++)
           {
               batman_if =if_list[i];
               curr_packet_num = curr_packet_len = 0;
               bat_packet = forw_node->pack_buff->dup();
               BatmanPacket *bat_packetAux=bat_packet;
               while (bat_packetAux!=NULL)
               {
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
                       bat_packetAux->setTq ((bat_packetAux->getTq() * (TQ_MAX_VALUE - (2 * hop_penalty))) / (TQ_MAX_VALUE));

//                   debug_output(4, "%s %spacket (originator %s, seqno %d, TQ %d, TTL %d, IDF %s) on interface %s\n", (curr_packet_num > 0 ? "Forwarding" : (forw_node->own ? "Sending own" : "Forwarding")), (curr_packet_num > 0 ? "aggregated " : ""), orig_str, ntohs(bat_packet->seqno), bat_packet->tq, bat_packet->ttl, (bat_packet->flags & DIRECTLINK ? "on" : "off"), batman_if->dev);

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

int8_t Batman::send_udp_packet(cPacket *packet_buff, int32_t packet_buff_len, const Uint128 & destAdd, int32_t send_sock, BatmanIf *batman_if)
{
    if ((batman_if != NULL) && (!batman_if->if_active))
    {
        delete packet_buff;
        return 0;
    }
    if (batman_if)
        sendToIp (packet_buff, BATMAN_PORT,destAdd,BATMAN_PORT,1,par("broadCastDelay"),batman_if->dev->ipv4Data()->getIPAddress());
    else
        sendToIp (packet_buff,BATMAN_PORT,destAdd,BATMAN_PORT,1,par("broadCastDelay"),(Uint128)0);
    return 0;
}


BatmanIf * Batman::is_batman_if(InterfaceEntry * dev)
{

    for (unsigned int i=0;i<if_list.size();i++)
    {
        if(if_list[i]->dev==dev)
            return if_list[i];
    }
    return NULL;
}
//
//
// modification routing tables methods
//
//
void Batman::add_del_route(const Uint128  & dest, uint8_t netmask, const Uint128  & router, int32_t ifi, InterfaceEntry* dev, uint8_t rt_table, int8_t route_type, int8_t route_action)
{

    if (route_type != ROUTE_TYPE_UNICAST)
        return;
    if (route_action==ROUTE_DEL)
    {
       setRoute(dest,0,0,0,0);
       return;
    }
    int index=-1;
    for (int i=0;i<getNumInterfaces();i++)
    {
        if (dev == this->getInterfaceEntry(i))
        {
            index=i;
            break;
        }
    }
    if (index>=0)
       setRoute(dest,router,index,-1,Uint128::UINT128_MAX);
}

int Batman::add_del_interface_rules(int8_t rule_action)
{

    if (isInMacLayer())
        return 1;
    int if_count=1;
    for (int i=0;i<getNumInterfaces();i++)
    {
        InterfaceEntry *ifr = getInterfaceEntry (i);

        if (ifr->ipv4Data ()==NULL) // no ipv4
            continue;

            continue;
        if (ifr->isDown())
            continue;

        Uint128 addr = ifr->ipv4Data()->getIPAddress().getInt();
        Uint128 netmask = ifr->ipv4Data()->getNetmask().getInt();
        uint8_t mask=0;
        for (unsigned int i=0;i<128;i++)
        {
           if (netmask.bit(i))
               mask++;
           else
               break;
        }


        Uint128 netaddr =  addr&netmask;
        BatmanIf *batman_if;


        Uint128 ZERO;
        add_del_route(netaddr, mask, ZERO, 0, ifr, BATMAN_RT_TABLE_TUNNEL, ROUTE_TYPE_THROW, rule_action);

        if ((batman_if = is_batman_if(ifr))==NULL)
            continue;

        add_del_rule(netaddr, netmask, BATMAN_RT_TABLE_TUNNEL, (rule_action == RULE_DEL ? 0 : BATMAN_RT_PRIO_TUNNEL + if_count), 0, RULE_TYPE_SRC, rule_action);

        if (ifr->isLoopback())
            add_del_rule(0, 0, BATMAN_RT_TABLE_TUNNEL, BATMAN_RT_PRIO_TUNNEL, ifr, RULE_TYPE_IIF, rule_action);
        if_count++;
    }

    return 1;
}

void Batman::add_del_rule(uint32_t network, uint8_t netmask, int8_t rt_table, uint32_t prio, InterfaceEntry *iif, int8_t rule_type, int8_t rule_action)
{
    return;
}


// Bits methods
/* clear the bits */
void Batman::bit_init( std::vector<TYPE_OF_WORD> &seq_bits )
{
    for (int i = 0 ; i < (int)num_words; i++)
    {
        seq_bits[i]= 0;
    }
}

/* returns true if corresponding bit in given seq_bits indicates so and curr_seqno is within range of last_seqno */
uint8_t Batman::get_bit_status( std::vector<TYPE_OF_WORD> &seq_bits, uint16_t last_seqno, uint16_t curr_seqno ) {
    int16_t diff, word_offset, word_num;
    diff= last_seqno- curr_seqno;
    if (diff < 0 || diff >= local_win_size)
        return 0;
    else
    {
        word_offset= ( last_seqno - curr_seqno ) % WORD_BIT_SIZE;    /* which position in the selected word */
        word_num   = ( last_seqno - curr_seqno ) / WORD_BIT_SIZE;    /* which word */

        if ( seq_bits[word_num] & 1<<word_offset )   /* get position status */
            return 1;
        else
            return 0;
    }
}

/* turn corresponding bit on, so we can remember that we got the packet */
void Batman::bit_mark( std::vector<TYPE_OF_WORD> &seq_bits, int32_t n )
{
    int32_t word_offset,word_num;
    if (n<0 || n >= local_win_size) {            /* if too old, just drop it */
        return;
    }

    word_offset= n%WORD_BIT_SIZE;    /* which position in the selected word */
    word_num   = n/WORD_BIT_SIZE;    /* which word */

    seq_bits[word_num]|= 1<<word_offset;    /* turn the position on */
}

/* shift the packet array p by n places. */
void Batman::bit_shift( std::vector<TYPE_OF_WORD> &seq_bits, int32_t n ) {
    int32_t word_offset, word_num;
    int32_t i;

/*    bit_print( seq_bits );*/
    if( n<=0 )
        return;

    word_offset= n%WORD_BIT_SIZE;    /* shift how much inside each word */
    word_num   = n/WORD_BIT_SIZE;    /* shift over how much (full) words */

    for (i=num_words-1; i>word_num; i-- )
    {
        /* going from old to new, so we can't overwrite the data we copy from. *
          * left is high, right is low: FEDC BA98 7654 3210
         *                                      ^^ ^^
         *                             vvvv
         * ^^^^ = from, vvvvv =to, we'd have word_num==1 and
         * word_offset==WORD_BIT_SIZE/2 ????? in this example. (=24 bits)
         *
         * our desired output would be: 9876 5432 1000 0000
         * */
    	seq_bits[i]=
    			(seq_bits[i - word_num] << word_offset) +
    					/* take the lower port from the left half, shift it left to its final position */
    			(seq_bits[i - word_num - 1] >>	(WORD_BIT_SIZE-word_offset));

        /* for our example that would be: word[0] = 9800 + 0076 = 9876 */
    }
    /* now for our last word, i==word_num, we only have the it's "left" half. that's the 1000 word in
     * our example.*/

    seq_bits[i]= (seq_bits[i - word_num] << word_offset);

    /* pad the rest with 0, if there is anything */
    i--;
	for (; i>=0; i--)
		seq_bits[i]= 0;
/*    bit_print( seq_bits ); */
}


/* receive and process one packet, returns 1 if received seq_num is considered new, 0 if old  */
char Batman::bit_get_packet( std::vector<TYPE_OF_WORD> &seq_bits, int16_t seq_num_diff, int8_t set_mark )
{
    int i;
    /* we already got a sequence number higher than this one, so we just mark it. this should wrap around the integer just fine */
    if ((seq_num_diff < 0) && (seq_num_diff >= -local_win_size)) {

        if ( set_mark )
            bit_mark( seq_bits, -seq_num_diff );
        return 0;
    }

    if ((seq_num_diff > local_win_size) || (seq_num_diff < -local_win_size))
    {        /* it seems we missed a lot of packets or the other host restarted */

//        if (seq_num_diff > local_win_size)
//            debug_output(4, "It seems we missed a lot of packets (%i) !\n",  seq_num_diff-1);

//        if (-seq_num_diff > local_win_size)
//            debug_output(4, "Other host probably restarted !\n");

        for (i=0; i<num_words; i++)
            seq_bits[i]= 0;

        if ( set_mark )
            seq_bits[0] = 1;  /* we only have the latest packet */

    }
    else
    {
        bit_shift(seq_bits, seq_num_diff);
        if ( set_mark )
            bit_mark(seq_bits, 0);

    }
    return 1;
}

/* count the hamming weight, how many good packets did we receive? just count the 1's ... */
int Batman::bit_packet_count( std::vector<TYPE_OF_WORD> &seq_bits )
{
    int i, hamming = 0;
    TYPE_OF_WORD word;
    for (i=0; i<num_words; i++)
    {
        word = seq_bits[i];
        while (word) {
            word &= word-1;   /* see http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan */
            hamming++;
        }
    }
    return(hamming);
}

uint8_t Batman::bit_count( int32_t to_count )
{

    uint8_t hamming = 0;
    while ( to_count )
    {
        to_count &= to_count-1;   /* see http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan */
        hamming++;
    }
    return(hamming);
}

void Batman::deactivate_interface(BatmanIf *iface)
{
    iface->if_active=false;
    //iface->dev->setDown(true);
}

void Batman::activate_interface(BatmanIf *iface)
{
    iface->if_active=true;

    //iface->dev->setDown(true);
}

void Batman::check_active_inactive_interfaces(void)
{
    /* all available interfaces are deactive */
    for (unsigned int i=0;i<if_list.size();i++){
        BatmanIf* batman_if = if_list[i];
        if ((batman_if->if_active) && (batman_if->dev->isDown()))
        {
            deactivate_interface(batman_if);
            active_ifs--;
        }
        else if ((!batman_if->if_active) && (!batman_if->dev->isDown()))
        {
            activate_interface(batman_if);
            active_ifs++;
        }
    }
}


void Batman::check_inactive_interfaces(void)
{
    /* all available interfaces are active */
    if (found_ifs == active_ifs)
        return;

    for (unsigned int i=0;i<if_list.size();i++){
        BatmanIf* batman_if = if_list[i];

        if ((!batman_if->if_active) && (!batman_if->dev->isDown()))
        {
            activate_interface(batman_if);
            active_ifs++;
        }
    }
}

void Batman::check_active_interfaces(void)
{
    /* all available interfaces are deactive */
    if (active_ifs == 0)
        return;
    for (unsigned int i=0;i<if_list.size();i++)
    {
        BatmanIf* batman_if = if_list[i];
        if ((batman_if->if_active) && (batman_if->dev->isDown()))
        {
            deactivate_interface(batman_if);
            active_ifs--;
        }
    }
}

BatmanPacket *Batman::buildDefaultBatmanPkt(const BatmanIf *batman_if)
{
	std::string str="BatmanPkt:"+batman_if->address.getIPAddress().str();
	BatmanPacket * pkt=new BatmanPacket(str.c_str());

	pkt->setVersion (0);
	pkt->setFlags (0x00);
	pkt->setTtl ((batman_if->if_num > 0 ? 2 : TTL));
	pkt->setGatewayFlags (batman_if->if_num > 0 ? 0 : gateway_class);
	pkt->setSeqNumber (batman_if->seqno);
	pkt->setGatewayPort (GW_PORT);
	pkt->setTq (TQ_MAX_VALUE);
    if (batman_if->if_active)
    {
	   pkt->setOrig(batman_if->address);
	   pkt->setPrevSender(batman_if->address);
    }
	return pkt;
}


simtime_t Batman::getTime()
{
    return simTime()+par("desynchronized");
}

