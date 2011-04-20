/***************************************************************************
 *   Copyright (C) 2004 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *                                                                         *
 *   Modified by Weverton Cordeiro                                         *
 *   (C) 2007 wevertoncordeiro@gmail.com                                   *
 *   Adapted for omnetpp                                                   *
 *   2008 Alfonso Ariza Quintana aarizaq@uma.es                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

///
/// \file   OLSR_ETX.cc
/// \brief  Implementation of OLSR agent and related classes.
///
/// This is the main file of this software because %OLSR's behaviour is
/// implemented here.
///


#include <math.h>
#include <limits.h>

#include <omnetpp.h>
#include "UDPPacket.h"
#include "IPControlInfo.h"
#include "IPv4InterfaceData.h"
#include "IPv6ControlInfo.h"

#include "NotifierConsts.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"

#include "OLSRpkt_m.h"
#include "OLSR_ETX.h"
#include "OLSR_ETX_dijkstra.h"


/// Length (in bytes) of UDP header.
#define UDP_HDR_LEN 8
/// Port Number
#define RT_PORT 698
#define IP_DEF_TTL 32


#define state_      (*state_etx_ptr)

///
/// \brief Function called by MAC layer when cannot deliver a packet.
///
/// \param p Packet which couldn't be delivered.
/// \param arg OLSR agent passed for a callback.
///


Define_Module(OLSR_ETX);

/********** Timers **********/

void OLSR_ETX_LinkQualityTimer::expire()
{
    OLSR_ETX *agentaux = check_and_cast<OLSR_ETX *>(agent_);
    agentaux->OLSR_ETX::link_quality();
    // agentaux->scheduleAt(simTime()+agentaux->hello_ival_,this);
    agentaux->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+agentaux->hello_ival_,this));
}


/********** OLSR_ETX class **********/
///
///
void
OLSR_ETX::initialize(int stage)
{
    if (stage==4)
    {

        //
        // Do some initializations
        willingness_=par("Willingness");
        hello_ival_=par("Hello_ival");
        tc_ival_=par("Tc_ival");
        mid_ival_=par("Mid_ival");
        use_mac_=par("use_mac");



        if ( par("Fish_eye"))
            parameter_.fish_eye() = 1;
        else
            parameter_.fish_eye() = 0;
        parameter_.mpr_algorithm() = par("Mpr_algorithm");
        parameter_.routing_algorithm() = par("routing_algorithm");
        parameter_.link_quality() = par("Link_quality");

        parameter_.tc_redundancy() = par("Tc_redundancy");

        /// Link delay extension
        if (par("Link_delay"))
            parameter_.link_delay() = 1;
        else
            parameter_.link_delay() = 0;
        parameter_.c_alpha() = par("C_alpha");

        // Fish Eye Routing Algorithm for TC message dispatching...
        tc_msg_ttl_index_ = 0;

        tc_msg_ttl_ [0] = 255;
        tc_msg_ttl_ [1] = 3;
        tc_msg_ttl_ [2] = 2;
        tc_msg_ttl_ [3] = 1;
        tc_msg_ttl_ [4] = 2;
        tc_msg_ttl_ [5] = 1;
        tc_msg_ttl_ [6] = 1;
        tc_msg_ttl_ [7] = 3;
        tc_msg_ttl_ [8] = 2;
        tc_msg_ttl_ [9] = 1;
        tc_msg_ttl_ [10] = 2;
        tc_msg_ttl_ [11] = 1;
        tc_msg_ttl_ [12] = 1;

        /// Link delay extension
        cap_sn_ = 0;
        pkt_seq_    = OLSR_MAX_SEQ_NUM;
        msg_seq_    = OLSR_MAX_SEQ_NUM;
        ansn_       = OLSR_MAX_SEQ_NUM;

        registerRoutingModule();
        ra_addr_ = getAddress();

        timerMessage = new cMessage();
        timerQueuePtr = new TimerQueue;

        // Starts all timers

        helloTimer = new OLSR_HelloTimer(); ///< Timer for sending HELLO messages.
        tcTimer = new OLSR_TcTimer();   ///< Timer for sending TC messages.
        midTimer = new OLSR_MidTimer(); ///< Timer for sending MID messages.
        linkQualityTimer = new OLSR_ETX_LinkQualityTimer();

        hello_timer_.resched(hello_ival_);
        tc_timer_.resched(hello_ival_);
        mid_timer_.resched(hello_ival_);
        link_quality_timer_.resched(0.0);

        useIndex = false;

        if (use_mac())
        {
            linkLayerFeeback();
        }
        state_ptr=state_etx_ptr=new OLSR_ETX_state();
        scheduleNextEvent();

    }
}


///
/// \brief  This function is called whenever a event  is received. It identifies
///     the type of the received event and process it accordingly.
///
/// If it is an %OLSR packet then it is processed. In other case, if it is a data packet
/// then it is forwarded.
///
/// \param  p the received packet.
/// \param  h a handler (not used).
///

///
/// \brief Processes an incoming %OLSR packet following RFC 3626 specification.
/// \param p received packet.
///
void
OLSR_ETX::recv_olsr(cMessage* msg)
{

    OLSR_pkt* op;
    nsaddr_t src_addr;
    int index;

    // All routing messages are sent from and to port RT_PORT,
    // so we check it.

// Extract information and delete the cantainer without more use
    op = check_packet(PK(msg),src_addr,index);
    if (op==NULL)
        return;


    // If the packet contains no messages must be silently discarded.
    // There could exist a message with an empty body, so the size of
    // the packet would be pkt-hdr-size + msg-hdr-size.

    if (op->getByteLength() < OLSR_PKT_HDR_SIZE + OLSR_MSG_HDR_SIZE)
    {
        delete op;
        return;
    }

// Process Olsr information
    assert(op->msgArraySize() >= 0 && op->msgArraySize() <= OLSR_MAX_MSGS);
    for (int i = 0; i < (int) op->msgArraySize(); i++)
    {
        OLSR_ETX_msg& msg = op->msg(i);

        // If ttl is less than or equal to zero, or
        // the receiver is the same as the originator,
        // the message must be silently dropped
        // if (msg.ttl() <= 0 || msg.orig_addr() == ra_addr())
        if (msg.ttl() <= 0 || isLocalAddress (msg.orig_addr()))
            continue;

        // If the message has been processed it must not be
        // processed again
        bool do_forwarding = true;
        OLSR_ETX_dup_tuple* duplicated = state_.find_dup_tuple(msg.orig_addr(), msg.msg_seq_num());
        if (duplicated == NULL)
        {
            // Process the message according to its type
            if (msg.msg_type() == OLSR_HELLO_MSG)
                process_hello(msg, ra_addr(), src_addr, op->pkt_seq_num(),index);
            else if (msg.msg_type() == OLSR_TC_MSG)
                process_tc(msg, src_addr,index);
            else if (msg.msg_type() == OLSR_MID_MSG)
                process_mid(msg, src_addr,index);
            else
            {
                debug("%f: Node %d can not process OLSR packet because does not "
                      "implement OLSR type (%x)\n",
                      CURRENT_TIME,
                      OLSR::node_id(ra_addr()),
                      msg.msg_type());
            }
        }
        else
        {
            // If the message has been considered for forwarding, it should
            // not be retransmitted again
            for (addr_list_t::iterator it = duplicated->iface_list().begin();
                    it != duplicated->iface_list().end();
                    it++)
            {
                if (*it == ra_addr())
                {
                    do_forwarding = false;
                    break;
                }
            }
        }

        if (do_forwarding)
        {
            // HELLO messages are never forwarded.
            // TC and MID messages are forwarded using the default algorithm.
            // Remaining messages are also forwarded using the default algorithm.
            if (msg.msg_type() != OLSR_HELLO_MSG)
                forward_default(msg, duplicated, ra_addr(),src_addr);
        }

    }
// Link delay extension
    if (parameter_.link_delay() && op->sn() > 0)
    {
        OLSR_ETX_link_tuple *link_tuple = NULL;
        OLSR_link_tuple *link_aux = state_.find_link_tuple (src_addr);
        if (link_aux)
        {
            link_tuple = dynamic_cast<OLSR_ETX_link_tuple*> (link_aux);
            if (!link_tuple)
                opp_error("\n Error conversion link tuple to link ETX tuple");
        }
        if (link_tuple)
            link_tuple->link_delay_computation(op);
    }

    // After processing all OLSR_ETX messages, we must recompute routing table
    switch (parameter_.routing_algorithm())
    {
    case OLSR_ETX_DIJKSTRA_ALGORITHM:
        rtable_dijkstra_computation();
        break;

    default:
    case OLSR_ETX_DEFAULT_ALGORITHM:
        rtable_default_computation();
        break;
    }
    delete op;

}

///
/// \brief Computates MPR set of a node following RFC 3626 hints.
///
// Hinerit
//void
//OLSR::mpr_computation()
void
OLSR_ETX::olsr_mpr_computation()
{
    mpr_computation();
}



///
/// \brief Computates MPR set of a node.
///
void
OLSR_ETX::olsr_r1_mpr_computation()
{
    // For further details please refer to paper
    // Quality of Service Routing in Ad Hoc Networks Using OLSR

    bool increment;
    state_.clear_mprset();

    nbset_t N; nb2hopset_t N2;
    // N is the subset of neighbors of the node, which are
    // neighbor "of the interface I" and have willigness different
    // from OLSR_ETX_WILL_NEVER
    for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
        if ((*it)->getStatus() == OLSR_ETX_STATUS_SYM) // I think that we need this check
            N.push_back(*it);

    // N2 is the set of 2-hop neighbors reachable from "the interface
    // I", excluding:
    // (i)   the nodes only reachable by members of N with willingness WILL_NEVER
    // (ii)  the node performing the computation
    // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
    //       link to this node on some interface.
    for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++)
    {
        OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>( *it);


        if (!nb2hop_tuple)
            opp_error("\n Error conversion nd2hop tuple");


        bool ok = true;
        OLSR_ETX_nb_tuple* nb_tuple =state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_addr());
        if (nb_tuple == NULL)
            ok = false;
        else
        {
            nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_addr(), OLSR_ETX_WILL_NEVER);
            if (nb_tuple != NULL)
                ok = false;
            else
            {
                nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb2hop_addr());
                if (nb_tuple != NULL)
                    ok = false;
            }
        }

        if (ok)
            N2.push_back(nb2hop_tuple);
    }

    // Start with an MPR set made of all members of N with
    // N_willingness equal to WILL_ALWAYS
    for (nbset_t::iterator it = N.begin(); it != N.end(); it++)
    {
        OLSR_ETX_nb_tuple* nb_tuple = *it;
        if (nb_tuple->willingness() == OLSR_ETX_WILL_ALWAYS)
            state_.insert_mpr_addr(nb_tuple->nb_main_addr());
    }

    // Add to Mi the nodes in N which are the only nodes to provide reachability
    // to a node in N2. Remove the nodes from N2 which are now covered by
    // a node in the MPR set.
    mprset_t foundset;
    std::set<nsaddr_t> deleted_addrs;
    // iterate through all 2 hop neighbors we have
    for (nb2hopset_t::iterator it = N2.begin(); it != N2.end();)
    {
        OLSR_ETX_nb2hop_tuple* nb2hop_tuple1 = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it);
        if (!nb2hop_tuple1)
            opp_error("\n Error conversion nd2hop tuple");

        increment = true;

        // check if this two hop neighbor has more that one hop neighbor in N
        // it would mean that there is more than one node in N that reaches
        // the current 2 hop node
        mprset_t::iterator pos = foundset.find(nb2hop_tuple1->nb2hop_addr());
        if (pos != foundset.end())
        {
            it++;
            continue;
        }

        bool found = false;
        // find the one hop neighbor that provides reachability to the
        // current two hop neighbor.
        for (nbset_t::iterator it2 = N.begin(); it2 != N.end(); it2++)
        {
            if ((*it2)->nb_main_addr() == nb2hop_tuple1->nb_main_addr())
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            it++;
            continue;
        }

        found = false;
        // check if there is another one hop neighbor able to provide
        // reachability to the current 2 hop neighbor
        for (nb2hopset_t::iterator it2 = it + 1; it2 != N2.end(); it2++)
        {
            OLSR_ETX_nb2hop_tuple* nb2hop_tuple2 = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it2);
            if (!nb2hop_tuple2)
                opp_error("\n Error conversion nd2hop tuple");

            if (nb2hop_tuple1->nb2hop_addr() == nb2hop_tuple2->nb2hop_addr())
            {
                foundset.insert(nb2hop_tuple1->nb2hop_addr());
                found = true;
                break;
            }
        }
        // if there is only one node, add our one hop neighbor to the MPR set
        if (!found)
        {
            state_.insert_mpr_addr(nb2hop_tuple1->nb_main_addr());

            // erase all 2 hop neighbor nodes that are now reached through this
            // newly added MPR
            for (nb2hopset_t::iterator it2 = it + 1; it2 != N2.end();)
            {
                OLSR_ETX_nb2hop_tuple* nb2hop_tuple2 = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it2);
                if (!nb2hop_tuple2)
                    opp_error("\n Error conversion nd2hop tuple");

                if (nb2hop_tuple1->nb_main_addr() == nb2hop_tuple2->nb_main_addr())
                {
                    deleted_addrs.insert(nb2hop_tuple2->nb2hop_addr());
                    it2 = N2.erase(it2);
                }
                else
                    it2++;
            }
            it = N2.erase(it);
            increment = false;
        }

        // erase all 2 hop neighbor nodes that are now reached through this
        // newly added MPR. We are now looking for the backup links
        for (std::set<nsaddr_t>::iterator it2 = deleted_addrs.begin();
                it2 != deleted_addrs.end(); it2++)
        {
            for (nb2hopset_t::iterator it3 = N2.begin(); it3 != N2.end();)
            {
                if ((*it3)->nb2hop_addr() == *it2)
                {
                    it3 = N2.erase(it3);
                    // I have to reset the external iterator because it
                    // may have been invalidated by the latter deletion
                    it = N2.begin();
                    increment = false;
                }
                else
                    it3++;
            }
        }
        deleted_addrs.clear();
        if (increment)
            it++;
    }

    // While there exist nodes in N2 which are not covered by at
    // least one node in the MPR set:
    while (N2.begin() != N2.end())
    {
        // For each node in N, calculate the reachability, i.e., the
        // number of nodes in N2 that it can reach
        std::map<int, std::vector<OLSR_ETX_nb_tuple*> > reachability;
        std::set<int> rs;
        for (nbset_t::iterator it = N.begin(); it != N.end(); it++)
        {
            OLSR_ETX_nb_tuple* nb_tuple = *it;
            int r = 0;
            for (nb2hopset_t::iterator it2 = N2.begin(); it2 != N2.end(); it2++)
            {
                OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it2);
                if (!nb2hop_tuple)
                    opp_error("\n Error conversion nd2hop tuple");


                if (nb_tuple->nb_main_addr() == nb2hop_tuple->nb_main_addr())
                    r++;
            }
            rs.insert(r);
            reachability[r].push_back(nb_tuple);
        }

        // Select as a MPR the node with highest N_willingness among
        // the nodes in N with non-zero reachability. In case of
        // multiple choice select the node which provides
        // reachability to the maximum number of nodes in N2. In
        // case of multiple choices select the node with best conectivity
        // to the current node. Remove the nodes from N2 which are now covered
        // by a node in the MPR set.
        OLSR_ETX_nb_tuple* max = NULL;
        int max_r = 0;
        for (std::set<int>::iterator it = rs.begin(); it != rs.end(); it++)
        {
            int r = *it;
            if (r > 0)
            {
                for (std::vector<OLSR_ETX_nb_tuple*>::iterator it2 = reachability[r].begin();
                        it2 != reachability[r].end(); it2++)
                {
                    OLSR_ETX_nb_tuple* nb_tuple = *it2;
                    if (max == NULL || nb_tuple->willingness() > max->willingness())
                    {
                        max = nb_tuple;
                        max_r = r;
                    }
                    else if (nb_tuple->willingness() == max->willingness())
                    {
                        if (r > max_r)
                        {
                            max = nb_tuple;
                            max_r = r;
                        }
                        else if (r == max_r)
                        {
                            OLSR_ETX_link_tuple *nb_link_tuple=NULL, *max_link_tuple=NULL;
                            OLSR_link_tuple * link_tuple_aux;
                            double now = CURRENT_TIME;
                            link_tuple_aux = state_.find_sym_link_tuple (nb_tuple->nb_main_addr(), now);
                            if (link_tuple_aux)
                            {
                                nb_link_tuple = dynamic_cast<OLSR_ETX_link_tuple *>(link_tuple_aux);
                                if (!nb_link_tuple)
                                    opp_error("\n Error conversion link tuple");
                            }
                            link_tuple_aux = state_.find_sym_link_tuple (max->nb_main_addr(), now);
                            if (link_tuple_aux)
                            {
                                max_link_tuple = dynamic_cast<OLSR_ETX_link_tuple *>(link_tuple_aux);
                                if (!max_link_tuple)
                                    opp_error("\n Error conversion link tuple");
                            }
                            if (nb_link_tuple || max_link_tuple)
                                continue;
                            if (parameter_.link_delay())
                            {
                                if (nb_link_tuple->link_delay() < max_link_tuple->link_delay())
                                {
                                    max = nb_tuple;
                                    max_r = r;
                                }
                            }
                            else
                            {
                                switch (parameter_.link_quality())
                                {
                                case OLSR_ETX_BEHAVIOR_ETX:
                                    if (nb_link_tuple->etx() < max_link_tuple->etx())
                                    {
                                        max = nb_tuple;
                                        max_r = r;
                                    }
                                    break;
                                case OLSR_ETX_BEHAVIOR_ML:
                                    if (nb_link_tuple->etx() > max_link_tuple->etx())
                                    {
                                        max = nb_tuple;
                                        max_r = r;
                                    }
                                    break;
                                case OLSR_ETX_BEHAVIOR_NONE:
                                default:
                                    // max = nb_tuple;
                                    // max_r = r;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (max != NULL)
        {
            state_.insert_mpr_addr(max->nb_main_addr());
            std::set<nsaddr_t> nb2hop_addrs;
            for (nb2hopset_t::iterator it = N2.begin(); it != N2.end();)
            {
                OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it);
                if (!nb2hop_tuple)
                    opp_error("\n Error conversion nd2hop tuple");


                if (nb2hop_tuple->nb_main_addr() == max->nb_main_addr())
                {
                    nb2hop_addrs.insert(nb2hop_tuple->nb2hop_addr());
                    it = N2.erase(it);
                }
                else
                    it++;
            }
            for (nb2hopset_t::iterator it = N2.begin(); it != N2.end();)
            {
                OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it);
                if (!nb2hop_tuple)
                    opp_error("\n Error conversion nd2hop tuple");


                std::set<nsaddr_t>::iterator it2 =
                    nb2hop_addrs.find(nb2hop_tuple->nb2hop_addr());
                if (it2 != nb2hop_addrs.end())
                {
                    it = N2.erase(it);
                }
                else
                    it++;

            }
        }
    }
}

///
/// \brief Computates MPR set of a node.
///
void
OLSR_ETX::olsr_r2_mpr_computation()
{
    // For further details please refer to paper
    // Quality of Service Routing in Ad Hoc Networks Using OLSR

    state_.clear_mprset();

    nbset_t N; nb2hopset_t N2;

    // N is the subset of neighbors of the node, which are
    // neighbor "of the interface I"
    for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
        if ((*it)->getStatus() == OLSR_ETX_STATUS_SYM &&
                (*it)->willingness() != OLSR_ETX_WILL_NEVER) // I think that we need this check
            N.push_back(*it);

    // N2 is the set of 2-hop neighbors reachable from "the interface
    // I", excluding:
    // (i)   the nodes only reachable by members of N with willingness WILL_NEVER
    // (ii)  the node performing the computation
    // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
    //       link to this node on some interface.
    for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++)
    {
        OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it);
        if (!nb2hop_tuple)
            opp_error("\n Error conversion nd2hop tuple");


        bool ok = true;
        OLSR_ETX_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_addr());

        if (nb_tuple == NULL)
            ok = false;
        else
        {
            nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_addr(), OLSR_ETX_WILL_NEVER);
            if (nb_tuple != NULL)
                ok = false;
            else
            {
                nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb2hop_addr());
                if (nb_tuple != NULL)
                    ok = false;
            }
        }
        if (ok)
            N2.push_back(nb2hop_tuple);
    }
    // While there exist nodes in N2 which are not covered by at
    // least one node in the MPR set:
    while (N2.begin() != N2.end())
    {
        // For each node in N, calculate the reachability, i.e., the
        // number of nodes in N2 that it can reach
        std::map<int, std::vector<OLSR_ETX_nb_tuple*> > reachability;
        std::set<int> rs;
        for (nbset_t::iterator it = N.begin(); it != N.end(); it++)
        {
            OLSR_ETX_nb_tuple* nb_tuple = *it;
            int r = 0;
            for (nb2hopset_t::iterator it2 = N2.begin(); it2 != N2.end(); it2++)
            {
                OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it2);
                if (!nb2hop_tuple)
                    opp_error("\n Error conversion nd2hop tuple");


                if (nb_tuple->nb_main_addr() == nb2hop_tuple->nb_main_addr())
                    r++;
            }
            rs.insert(r);
            reachability[r].push_back(nb_tuple);
        }

        // Add to Mi the node in N that has the best link to the current
        // node. In case of tie, select tin N2. Remove the nodes from N2
        // which are now covered by a node in the MPR set.
        OLSR_ETX_nb_tuple* max = NULL;
        int max_r = 0;
        for (std::set<int>::iterator it = rs.begin(); it != rs.end(); it++)
        {
            int r = *it;
            if (r > 0)
            {
                for (std::vector<OLSR_ETX_nb_tuple*>::iterator it2 = reachability[r].begin();
                        it2 != reachability[r].end(); it2++)
                {
                    OLSR_ETX_nb_tuple* nb_tuple = *it2;
                    if (max == NULL)
                    {
                        max = nb_tuple;
                        max_r = r;
                    }
                    else
                    {
                        OLSR_ETX_link_tuple *nb_link_tuple=NULL, *max_link_tuple=NULL;
                        OLSR_link_tuple *link_tuple_aux;
                        double now = CURRENT_TIME;

                        link_tuple_aux = state_.find_sym_link_tuple (nb_tuple->nb_main_addr(), now);
                        if (link_tuple_aux)
                        {
                            nb_link_tuple = dynamic_cast<OLSR_ETX_link_tuple *>(link_tuple_aux);
                            if (!nb_link_tuple)
                                opp_error("\n Error conversion link tuple");
                        }
                        link_tuple_aux = state_.find_sym_link_tuple (max->nb_main_addr(), now);
                        if (link_tuple_aux)
                        {
                            max_link_tuple = dynamic_cast<OLSR_ETX_link_tuple *>(link_tuple_aux);
                            if (!max_link_tuple)
                                opp_error("\n Error conversion link tuple");
                        }
                        if (nb_link_tuple || max_link_tuple)
                            continue;
                        switch (parameter_.link_quality())
                        {
                        case OLSR_ETX_BEHAVIOR_ETX:
                            if (nb_link_tuple->etx() < max_link_tuple->etx())
                            {
                                max = nb_tuple;
                                max_r = r;
                            }
                            else if (nb_link_tuple->etx() == max_link_tuple->etx())
                            {
                                if (r > max_r)
                                {
                                    max = nb_tuple;
                                    max_r = r;
                                }
                                else if (r == max_r && degree(nb_tuple) > degree (max))
                                {
                                    max = nb_tuple;
                                    max_r = r;
                                }
                            }
                            break;
                        case OLSR_ETX_BEHAVIOR_ML:
                            if (nb_link_tuple->etx() > max_link_tuple->etx())
                            {
                                max = nb_tuple;
                                max_r = r;
                            }
                            else if (nb_link_tuple->etx() == max_link_tuple->etx())
                            {
                                if (r > max_r)
                                {
                                    max = nb_tuple;
                                    max_r = r;
                                }
                                else if (r == max_r && degree(nb_tuple) > degree (max))
                                {
                                    max = nb_tuple;
                                    max_r = r;
                                }
                            }
                            break;
                        case OLSR_ETX_BEHAVIOR_NONE:
                        default:
                            if (r > max_r)
                            {
                                max = nb_tuple;
                                max_r = r;
                            }
                            else if (r == max_r && degree(nb_tuple) > degree (max))
                            {
                                max = nb_tuple;
                                max_r = r;
                            }
                            break;
                        }
                    }
                }
            }
        }
        if (max != NULL)
        {
            state_.insert_mpr_addr(max->nb_main_addr());
            std::set<nsaddr_t> nb2hop_addrs;
            for (nb2hopset_t::iterator it = N2.begin(); it != N2.end();)
            {
                OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it);
                if (!nb2hop_tuple)
                    opp_error("\n Error conversion nd2hop tuple");

                if (nb2hop_tuple->nb_main_addr() == max->nb_main_addr())
                {
                    nb2hop_addrs.insert(nb2hop_tuple->nb2hop_addr());
                    it = N2.erase(it);
                }
                else
                    it++;

            }
            for (nb2hopset_t::iterator it = N2.begin(); it != N2.end();)
            {
                OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it);
                if (!nb2hop_tuple)
                    opp_error("\n Error conversion nd2hop tuple");

                std::set<nsaddr_t>::iterator it2 =
                    nb2hop_addrs.find(nb2hop_tuple->nb2hop_addr());
                if (it2 != nb2hop_addrs.end())
                {
                    it = N2.erase(it);
                }
                else
                    it++;

            }
        }
    }
}

///
/// \brief Computates MPR set of a node.
///
void
OLSR_ETX::qolsr_mpr_computation()
{
    // For further details please refer to paper
    // QoS Routing in OLSR with Several Classes of Services

    state_.clear_mprset();

    nbset_t N; nb2hopset_t N2;
    // N is the subset of neighbors of the node, which are
    // neighbor "of the interface I"
    for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
        if ((*it)->getStatus() == OLSR_ETX_STATUS_SYM &&
                (*it)->willingness() != OLSR_ETX_WILL_NEVER) // I think that we need this check
            N.push_back(*it);

    // N2 is the set of 2-hop neighbors reachable from "the interface
    // I", excluding:
    // (i)   the nodes only reachable by members of N with willingness WILL_NEVER
    // (ii)  the node performing the computation
    // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
    //       link to this node on some interface.
    for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++)
    {
        OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it);
        if (!nb2hop_tuple)
            opp_error("\n Error conversion nd2hop tuple");

        bool ok = true;
        OLSR_ETX_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_addr());
        if (nb_tuple == NULL)
            ok = false;
        else
        {
            nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_addr(), OLSR_ETX_WILL_NEVER);
            if (nb_tuple != NULL)
                ok = false;
            else
            {
                nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb2hop_addr());
                if (nb_tuple != NULL)
                    ok = false;
            }
        }
        if (ok)
            N2.push_back(nb2hop_tuple);
    }
    // While there exist nodes in N2 which are not covered by at
    // least one node in the MPR set:
    while (N2.begin() != N2.end())
    {
        // For each node in N, calculate the reachability, i.e., the
        // number of nodes in N2 that it can reach
        std::map<int, std::vector<OLSR_ETX_nb_tuple*> > reachability;
        std::set<int> rs;
        for (nbset_t::iterator it = N.begin(); it != N.end(); it++)
        {
            OLSR_ETX_nb_tuple* nb_tuple = *it;
            int r = 0;
            for (nb2hopset_t::iterator it2 = N2.begin(); it2 != N2.end(); it2++)
            {
                OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it2);
                if (!nb2hop_tuple)
                    opp_error("\n Error conversion nd2hop tuple");

                if (nb_tuple->nb_main_addr() == nb2hop_tuple->nb_main_addr())
                    r++;
            }
            rs.insert(r);
            reachability[r].push_back(nb_tuple);
        }
        // Select a node z from N2
        OLSR_nb2hop_tuple* t_aux = *(N2.begin());
        OLSR_ETX_nb2hop_tuple* z =NULL;
        if (t_aux)
        {
            z = dynamic_cast<OLSR_ETX_nb2hop_tuple*> (t_aux);
            if (!z)
                opp_error("\n Error conversion nd2hop tuple");
        }

        // Add to Mi, if not yet present, the node in N that provides the
        // shortest-widest path to reach z. In case of tie, select the node
        // that reaches the maximum number of nodes in N2. Remove the nodes from N2
        // which are now covered by a node in the MPR set.
        OLSR_ETX_nb_tuple* max = NULL;
        int max_r = 0;

        // Iterate through all links in nb2hop_set that has the same two hop
        // neighbor as the second point of the link
        for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++)
        {
            OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it);
            if (!nb2hop_tuple)
                opp_error("\n Error conversion nd2hop tuple");


            // If the two hop neighbor is not the one we have selected, skip
            if (nb2hop_tuple->nb2hop_addr() != z->nb2hop_addr())
                continue;
            // Compare the one hop neighbor that reaches the two hop neighbor z with
            for (std::set<int>::iterator it2 = rs.begin(); it2 != rs.end(); it2++)
            {
                int r = *it2;
                if (r > 0)
                {
                    for (std::vector<OLSR_ETX_nb_tuple*>::iterator it3 = reachability[r].begin();
                            it3 != reachability[r].end(); it3++)
                    {
                        OLSR_ETX_nb_tuple* nb_tuple = *it3;
                        if (nb2hop_tuple->nb_main_addr() != nb_tuple->nb_main_addr())
                            continue;
                        if (max == NULL)
                        {
                            max = nb_tuple;
                            max_r = r;
                        }
                        else
                        {
                            OLSR_ETX_link_tuple *nb_link_tuple=NULL, *max_link_tuple=NULL;
                            double now = CURRENT_TIME;


                            OLSR_link_tuple *link_tuple_aux = state_.find_sym_link_tuple (nb_tuple->nb_main_addr(), now); /* bug */
                            if (link_tuple_aux)
                            {
                                nb_link_tuple = dynamic_cast<OLSR_ETX_link_tuple *>(link_tuple_aux);
                                if (!nb_link_tuple)
                                    opp_error("\n Error conversion link tuple");
                            }
                            link_tuple_aux = state_.find_sym_link_tuple (max->nb_main_addr(), now); /* bug */
                            if (link_tuple_aux)
                            {
                                max_link_tuple = dynamic_cast<OLSR_ETX_link_tuple *>(link_tuple_aux);
                                if (!max_link_tuple)
                                    opp_error("\n Error conversion link tuple");
                            }

                            double current_total_etx, max_total_etx;

                            switch (parameter_.link_quality())
                            {
                            case OLSR_ETX_BEHAVIOR_ETX:
                                current_total_etx = nb_link_tuple->etx() + nb2hop_tuple->etx();
                                max_total_etx = max_link_tuple->etx() + nb2hop_tuple->etx();
                                if (current_total_etx < max_total_etx)
                                {
                                    max = nb_tuple;
                                    max_r = r;
                                }
                                else if (current_total_etx == max_total_etx)
                                {
                                    if (r > max_r)
                                    {
                                        max = nb_tuple;
                                        max_r = r;
                                    }
                                    else if (r == max_r && degree(nb_tuple) > degree (max))
                                    {
                                        max = nb_tuple;
                                        max_r = r;
                                    }
                                }
                                break;

                            case OLSR_ETX_BEHAVIOR_ML:
                                current_total_etx = nb_link_tuple->etx() * nb2hop_tuple->etx();
                                max_total_etx = max_link_tuple->etx() * nb2hop_tuple->etx();
                                if (current_total_etx > max_total_etx)
                                {
                                    max = nb_tuple;
                                    max_r = r;
                                }
                                else if (current_total_etx == max_total_etx)
                                {
                                    if (r > max_r)
                                    {
                                        max = nb_tuple;
                                        max_r = r;
                                    }
                                    else if (r == max_r && degree(nb_tuple) > degree (max))
                                    {
                                        max = nb_tuple;
                                        max_r = r;
                                    }
                                }
                                break;
                            case OLSR_ETX_BEHAVIOR_NONE:
                            default:
                                if (r > max_r)
                                {
                                    max = nb_tuple;
                                    max_r = r;
                                }
                                else if (r == max_r && degree(nb_tuple) > degree (max))
                                {
                                    max = nb_tuple;
                                    max_r = r;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (max != NULL)
        {
            state_.insert_mpr_addr(max->nb_main_addr());
            std::set<nsaddr_t> nb2hop_addrs;
            for (nb2hopset_t::iterator it = N2.begin(); it != N2.end();)
            {
                OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it);
                if (!nb2hop_tuple)
                    opp_error("\n Error conversion nd2hop tuple");

                if (nb2hop_tuple->nb_main_addr() == max->nb_main_addr())
                {
                    nb2hop_addrs.insert(nb2hop_tuple->nb2hop_addr());
                    it = N2.erase(it);
                }
                else
                    it++;

            }
            for (nb2hopset_t::iterator it = N2.begin(); it != N2.end();)
            {
                OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it);
                if (!nb2hop_tuple)
                    opp_error("\n Error conversion nd2hop tuple");


                std::set<nsaddr_t>::iterator it2 =
                    nb2hop_addrs.find(nb2hop_tuple->nb2hop_addr());
                if (it2 != nb2hop_addrs.end())
                {
                    it = N2.erase(it);
                }
                else
                    it++;
            }
        }
    }
}

///
/// \brief Computates MPR set of a node.
///
void
OLSR_ETX::olsrd_mpr_computation()
{
    // MPR computation algorithm according to olsrd project: all nodes will be selected
    // as MPRs, since they have WILLIGNESS different of WILL_NEVER

    state_.clear_mprset();

    // Build a MPR set made of all members of N with
    // N_willingness different of WILL_NEVER
    for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
    {
        OLSR_ETX_nb_tuple* nb_tuple = *it;
        if (nb_tuple->willingness() != OLSR_ETX_WILL_NEVER &&
                nb_tuple->getStatus() == OLSR_ETX_STATUS_SYM)
            state_.insert_mpr_addr(nb_tuple->nb_main_addr());
    }
}


///
/// \brief Creates the routing table of the node following RFC 3626 hints.
///
void
OLSR_ETX::rtable_default_computation()
{
    rtable_computation();
    // 1. All the entries from the routing table are removed.
}




///
/// \brief Creates the routing table of the node using dijkstra algorithm
///
void
OLSR_ETX::rtable_dijkstra_computation()
{
    // Declare a class that will run the dijkstra algorithm
    Dijkstra *dijkstra=new Dijkstra ();

    // All the entries from the routing table are removed.
    rtable_.clear();
    omnet_clean_rte();

    debug ("Current node %d:\n", (uint32_t)ra_addr());
    // Iterate through all out 1 hop neighbors
    for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
    {
        OLSR_ETX_nb_tuple* nb_tuple = *it;

        // Get the best link we have to the current neighbor..
        OLSR_ETX_link_tuple* best_link =
            state_.find_best_sym_link_tuple (nb_tuple->nb_main_addr(), CURRENT_TIME);
        // Add this edge to the graph we are building
        if (best_link)
        {
            debug ("nb_tuple: %d (local) ==> %d , delay %lf, quality %lf\n", (uint32_t)best_link->local_iface_addr(),
                   (uint32_t)nb_tuple->nb_main_addr(), best_link->nb_link_delay(), best_link->etx());
            dijkstra->add_edge (nb_tuple->nb_main_addr(), best_link->local_iface_addr(),
                                best_link->nb_link_delay(), best_link->etx(), true);
        }
    }

    // N (set of 1-hop neighbors) is the set of nodes reachable through a symmetric
    // link with willingness different of WILL_NEVER. The vector at each position
    // is a list of the best links connecting the one hop neighbor to a 2 hop neighbor
    // Note: we are not our own two hop neighbor
    std::map<nsaddr_t, std::vector<OLSR_ETX_nb2hop_tuple*> > N;
    std::set<nsaddr_t> N_index;
    for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++)
    {
        OLSR_ETX_nb2hop_tuple* nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(*it);
        if (!nb2hop_tuple)
            opp_error("\n Error conversion nd2hop tuple");

        nsaddr_t nb2hop_main_addr = nb2hop_tuple->nb2hop_addr();
        nsaddr_t nb_main_addr = nb2hop_tuple->nb_main_addr();

        if (nb2hop_main_addr == ra_addr())
            continue;
        // do we have a symmetric link to the one hop neighbor?
        OLSR_ETX_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb_main_addr);
        if (nb_tuple == NULL)
            continue;
        // one hop neighbor has willingness different from OLSR_ETX_WILL_NEVER?
        nb_tuple = state_.find_nb_tuple(nb_main_addr, OLSR_ETX_WILL_NEVER);
        if (nb_tuple != NULL)
            continue;
        // Retrieve the link that connect us to this 2 hop neighbor
        OLSR_ETX_nb2hop_tuple* best_link = NULL;
        OLSR_nb2hop_tuple * nb2hop_tuple_aux = state_.find_nb2hop_tuple(nb_main_addr, nb2hop_main_addr);
        if (nb2hop_tuple_aux)
        {
            best_link = dynamic_cast<OLSR_ETX_nb2hop_tuple *>(nb2hop_tuple_aux);
            if (!best_link)
                opp_error("\n Error conversion nb2tuple tuple");
        }

        bool found = false;
        for (std::vector<OLSR_ETX_nb2hop_tuple*>::iterator it2 = N[nb_main_addr].begin();
                it2 != N[nb_main_addr].end(); it2++)
        {
            OLSR_ETX_nb2hop_tuple* current_link = *it2;
            if (current_link->nb_main_addr() == nb_main_addr &&
                    current_link->nb2hop_addr() == nb2hop_main_addr)
            {
                found = true;
                break;
            }
        }
        if (!found)
            N[nb_main_addr].push_back(best_link);
        N_index.insert(nb_main_addr);
    }
    // we now have the best link to all of our 2 hop neighbors. Add this information
    // for each 2 hop neighbor to the edge vector...
    for (std::set<nsaddr_t>::iterator it = N_index.begin(); it != N_index.end(); it++)
    {
        nsaddr_t nb_main_addr = *it;

        for (std::vector<OLSR_ETX_nb2hop_tuple*>::iterator it2 = N[nb_main_addr].begin();
                it2 != N[nb_main_addr].end(); it2++)
        {
            OLSR_ETX_nb2hop_tuple* nb2hop_tuple = *it2;
            // Add this edge to the graph we are building. The last hop is our 1 hop
            // neighbor that has the best link to the current two hop neighbor. And
            // nb2hop_addr is not directly connected to this node
            debug ("nb2hop_tuple: %d (local) ==> %d , delay %lf, quality %lf\n", (uint32_t)nb_main_addr,
                   (uint32_t)nb2hop_tuple->nb2hop_addr(), nb2hop_tuple->nb_link_delay(), nb2hop_tuple->etx());
            dijkstra->add_edge (nb2hop_tuple->nb2hop_addr(), nb_main_addr,
                                nb2hop_tuple->nb_link_delay(), nb2hop_tuple->etx(), false);
        }
    }

    // here we rely on the fact that in TC messages only the best links are published
    for (topologyset_t::iterator it = topologyset().begin();
            it != topologyset().end(); it++)
    {
        OLSR_ETX_topology_tuple* topology_tuple = dynamic_cast<OLSR_ETX_topology_tuple*>(*it);
        if (!topology_tuple)
            opp_error("\n Error conversion topology tuple");



        if (topology_tuple->dest_addr() == ra_addr())
            continue;
        // Add this edge to the graph we are building. The last hop is our 1 hop
        // neighbor that has the best link to the current two hop. And dest_addr
        // is not directly connected to this node
        debug ("topology_tuple: %d (local) ==> %d , delay %lf, quality %lf\n", (uint32_t)topology_tuple->last_addr(),
               (uint32_t)topology_tuple->dest_addr(), topology_tuple->nb_link_delay(), topology_tuple->etx());
        dijkstra->add_edge (topology_tuple->dest_addr(), topology_tuple->last_addr(),
                            topology_tuple->nb_link_delay(), topology_tuple->etx(), false);
    }

    // Run the dijkstra algorithm
    dijkstra->run();

    // Now all we have to do is inserting routes according to hop count
#if 1
    std::multimap<int,nsaddr_t> processed_nodes;
    for (Dijkstra::DijkstraMap::iterator it = dijkstra->dijkstraMap.begin();it != dijkstra->dijkstraMap.end(); it++)
    {
    	// store the nodes in hop order, the multimap order the values in function of number of hops
        processed_nodes.insert(std::pair<int,nsaddr_t>(it->second.hop_count(),it->first));
    }
    while (!processed_nodes.empty())
    {

    	std::multimap<int,nsaddr_t>::iterator it=processed_nodes.begin();
    	Dijkstra::DijkstraMap::iterator itDij = dijkstra->dijkstraMap.find(it->second);
    	if (itDij==dijkstra->dijkstraMap.end())
    	    opp_error("node not found in DijkstraMap");
    	int hopCount =it->first;
        if (hopCount == 1)
        {
            // add route...
            rtable_.add_entry(it->second, it->second, itDij->second.link().last_node(), 1,-1);
            omnet_chg_rte (it->second,it->second,0,hopCount,false,itDij->second.link().last_node());
        }
        else if (it->first > 1)
        {
            // add route...
            OLSR_ETX_rt_entry* entry = rtable_.lookup(itDij->second.link().last_node());
            if (entry==NULL)
                opp_error("entry not found");
            rtable_.add_entry(it->second, entry->next_addr(), entry->iface_addr(), hopCount,entry->local_iface_index());
            omnet_chg_rte (it->second, entry->next_addr(),0,hopCount,false,entry->iface_addr());
        }
        processed_nodes.erase(processed_nodes.begin());
        dijkstra->dijkstraMap.erase(itDij);
    }
    dijkstra->dijkstraMap.clear();
#else
    std::set<nsaddr_t> processed_nodes;
    for (std::set<nsaddr_t>::iterator it = dijkstra->all_nodes()->begin();it != dijkstra->all_nodes()->end(); it++)
    {
        if (dijkstra->D(*it).hop_count() == 1)
        {
            // add route...
            rtable_.add_entry(*it, *it, dijkstra->D(*it).link().last_node(), 1,-1);
            omnet_chg_rte (*it, *it,0,1,false,dijkstra->D(*it).link().last_node());
            processed_nodes.insert(*it);
        }
    }
    for (std::set<nsaddr_t>::iterator it = processed_nodes.begin(); it != processed_nodes.end(); it++)
        dijkstra->all_nodes()->erase(*it);
    processed_nodes.clear();
    for (std::set<nsaddr_t>::iterator it = dijkstra->all_nodes()->begin();it != dijkstra->all_nodes()->end(); it++)
    {
        if (dijkstra->D(*it).hop_count() == 2)
        {
            // add route...
            OLSR_ETX_rt_entry* entry = rtable_.lookup(dijkstra->D(*it).link().last_node());
            assert(entry != NULL);
            rtable_.add_entry(*it, dijkstra->D(*it).link().last_node(), entry->iface_addr(), 2,entry->local_iface_index());
            omnet_chg_rte (*it, dijkstra->D(*it).link().last_node(),0,2,false,entry->iface_addr());
            processed_nodes.insert(*it);
        }
    }
    for (std::set<nsaddr_t>::iterator it = processed_nodes.begin(); it != processed_nodes.end(); it++)
        dijkstra->all_nodes()->erase(*it);
    processed_nodes.clear();
    for (int i = 3; i <= dijkstra->highest_hop(); i++)
    {
        for (std::set<nsaddr_t>::iterator it = dijkstra->all_nodes()->begin();it != dijkstra->all_nodes()->end(); it++)
        {
            if (dijkstra->D(*it).hop_count() == i)
            {
                // add route...
                OLSR_ETX_rt_entry* entry = rtable_.lookup(dijkstra->D(*it).link().last_node());
                assert(entry != NULL);
                rtable_.add_entry(*it, entry->next_addr(), entry->iface_addr(), i,entry->local_iface_index());
                omnet_chg_rte (*it, entry->next_addr(),0,i,false,entry->iface_addr());
                processed_nodes.insert(*it);
            }
        }
        for (std::set<nsaddr_t>::iterator it = processed_nodes.begin(); it != processed_nodes.end(); it++)
            dijkstra->all_nodes()->erase(*it);
        processed_nodes.clear();
    }
#endif
    // 5. For each entry in the multiple interface association base
    // where there exists a routing entry such that:
    //  R_dest_addr  == I_main_addr  (of the multiple interface association entry)
    // AND there is no routing entry such that:
    //  R_dest_addr  == I_iface_addr
    // then a route entry is created in the routing table
    for (ifaceassocset_t::iterator it = ifaceassocset().begin(); it != ifaceassocset().end(); it++)
    {
        OLSR_ETX_iface_assoc_tuple* tuple = *it;
        OLSR_ETX_rt_entry* entry1 = rtable_.lookup(tuple->main_addr());
        OLSR_ETX_rt_entry* entry2 = rtable_.lookup(tuple->iface_addr());
        if (entry1 != NULL && entry2 == NULL)
        {
            rtable_.add_entry(tuple->iface_addr(),
                              entry1->next_addr(), entry1->iface_addr(), entry1->dist(),entry1->local_iface_index());
            omnet_chg_rte (tuple->iface_addr(),entry1->next_addr(),0,entry1->dist(),false,entry1->iface_addr());

        }
    }
    // rtable_.print_debug(this);
    // destroy the dijkstra class we've created
    // dijkstra->clear ();
    delete dijkstra;
}

///
/// \brief Processes a HELLO message following RFC 3626 specification.
///
/// Link sensing and population of the Neighbor Set, 2-hop Neighbor Set and MPR
/// Selector Set are performed.
///
/// \param msg the %OLSR message which contains the HELLO message.
/// \param receiver_iface the address of the interface where the message was received from.
/// \param sender_iface the address of the interface where the message was sent from.
///
void
OLSR_ETX::process_hello(OLSR_msg& msg, const nsaddr_t &receiver_iface, const nsaddr_t &sender_iface,uint16_t pkt_seq_num,const int &index)
{
    assert(msg.msg_type() == OLSR_ETX_HELLO_MSG);
    link_sensing(msg, receiver_iface, sender_iface, pkt_seq_num,index);
    populate_nbset(msg);
    populate_nb2hopset(msg);
    switch (parameter_.mpr_algorithm())
    {
    case OLSR_ETX_MPR_R1:
        olsr_r1_mpr_computation();
        break;
    case OLSR_ETX_MPR_R2:
        olsr_r2_mpr_computation();
        break;
    case OLSR_ETX_MPR_QOLSR:
        qolsr_mpr_computation();
        break;
    case OLSR_ETX_MPR_OLSRD:
        olsrd_mpr_computation();
        break;
    case OLSR_ETX_DEFAULT_MPR:
    default:
        olsr_mpr_computation();
        break;
    }
    populate_mprselset(msg);
}

///
/// \brief Processes a TC message following RFC 3626 specification.
///
/// The Topology Set is updated (if needed) with the information of
/// the received TC message.
///
/// \param msg the %OLSR message which contains the TC message.
/// \param sender_iface the address of the interface where the message was sent from.
///
void
OLSR_ETX::process_tc(OLSR_msg& msg, const nsaddr_t &sender_iface,const int &index)
{
    assert(msg.msg_type() == OLSR_ETX_TC_MSG);
    double now = CURRENT_TIME;
    OLSR_tc& tc = msg.tc();

    // 1. If the sender interface of this message is not in the symmetric
    // 1-hop neighborhood of this node, the message MUST be discarded.
    OLSR_ETX_link_tuple* link_tuple = NULL;
    OLSR_link_tuple *tuple_aux = state_.find_sym_link_tuple(sender_iface, now);
    if (tuple_aux)
    {
        link_tuple = dynamic_cast<OLSR_ETX_link_tuple*> (tuple_aux);
        if (!link_tuple)
            opp_error("\n Error conversion link tuple");
    }

    if (link_tuple == NULL)
        return;
    // 2. If there exist some tuple in the topology set where:
    //   T_last_addr == originator address AND
    //   T_seq       >  ANSN,
    // then further processing of this TC message MUST NOT be
    // performed. This might be a message received out of order.
    OLSR_ETX_topology_tuple* topology_tuple = NULL;
    OLSR_topology_tuple* topology_tuple_aux = state_.find_newer_topology_tuple(msg.orig_addr(), tc.ansn());
    if (topology_tuple_aux)
    {
        topology_tuple = dynamic_cast<OLSR_ETX_topology_tuple*> (topology_tuple_aux);
        if (!topology_tuple)
            opp_error("\n error conversion Topology tuple");
    }

    if (topology_tuple != NULL)
        return;

    // 3. All tuples in the topology set where:
    //  T_last_addr == originator address AND
    //  T_seq       <  ANSN
    // MUST be removed from the topology set.
    state_.erase_older_topology_tuples(msg.orig_addr(), tc.ansn());

    // 4. For each of the advertised neighbor main address received in
    // the TC message:
    for (int i = 0; i < tc.count; i++)
    {
        assert(i >= 0 && i < OLSR_ETX_MAX_ADDRS);
        nsaddr_t addr = tc.nb_etx_main_addr(i).iface_address();
        // 4.1. If there exist some tuple in the topology set where:
        //   T_dest_addr == advertised neighbor main address, AND
        //   T_last_addr == originator address,
        // then the holding time of that tuple MUST be set to:
        //   T_time      =  current time + validity time.
        OLSR_ETX_topology_tuple* topology_tuple = NULL;
        OLSR_topology_tuple* topology_tuple_aux = state_.find_topology_tuple(addr, msg.orig_addr());
        if (topology_tuple_aux)
        {
            topology_tuple = dynamic_cast<OLSR_ETX_topology_tuple*> (topology_tuple_aux);
            if (!topology_tuple)
                opp_error("\n error conversion Topology tuple");
        }


        if (topology_tuple != NULL)
            topology_tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
        // 4.2. Otherwise, a new tuple MUST be recorded in the topology
        // set where:
        //  T_dest_addr = advertised neighbor main address,
        //  T_last_addr = originator address,
        //  T_seq       = ANSN,
        //  T_time      = current time + validity time.
        else
        {
            topology_tuple = new OLSR_ETX_topology_tuple;
            topology_tuple->dest_addr() = addr;
            topology_tuple->last_addr() = msg.orig_addr();
            topology_tuple->seq() = tc.ansn();
            topology_tuple->set_qos_behaviour(parameter_.link_quality());
            topology_tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
            add_topology_tuple(topology_tuple);
            // Schedules topology tuple deletion
            OLSR_TopologyTupleTimer* topology_timer =
                new OLSR_TopologyTupleTimer(this, topology_tuple);
            topology_timer->resched(DELAY(topology_tuple->time()));
        }
        // Update link quality and link delay information

        topology_tuple->update_link_quality(tc.nb_etx_main_addr(i).link_quality(),
                                            tc.nb_etx_main_addr(i).nb_link_quality());

        topology_tuple->update_link_delay(tc.nb_etx_main_addr(i).link_delay(),
                                          tc.nb_etx_main_addr(i).nb_link_delay());
    }
}

///
/// \brief OLSR's default forwarding algorithm.
///
/// See RFC 3626 for details.
///
/// \param p the %OLSR packet which has been received.
/// \param msg the %OLSR message which must be forwarded.
/// \param dup_tuple NULL if the message has never been considered for forwarding,
/// or a duplicate tuple in other case.
/// \param local_iface the address of the interface where the message was received from.
///
// void OLSR::forward_default from OLSR

///
/// \brief Forwards a data packet to the appropiate next hop indicated by the routing table.
///
/// \param p the packet which must be forwarded.
///
#if 0
void
OLSR::forward_data(Packet* p)
{
    struct hdr_cmn* ch  = HDR_CMN(p);
    struct hdr_ip* ih   = HDR_IP(p);

    if (ch->direction() == hdr_cmn::UP &&
            ((uint32_t)ih->daddr() == IP_BROADCAST || ih->daddr() == ra_addr()))
    {
        dmux_->recv(p, 0);
        return;
    }
    else
    {
        ch->direction() = hdr_cmn::DOWN;
        ch->addr_type() = NS_AF_INET;
        if ((uint32_t)ih->daddr() == IP_BROADCAST)
            ch->next_hop()  = IP_BROADCAST;
        else
        {
            OLSR_rt_entry* entry = rtable_.lookup(ih->daddr());
            if (entry == NULL)
            {
                debug("%f: Node %d can not forward a packet destined to %d\n",
                      CURRENT_TIME,
                      OLSR::node_id(ra_addr()),
                      OLSR::node_id(ih->daddr()));
                drop(p, DROP_RTR_NO_ROUTE);
                return;
            }
            else
            {
                entry = rtable_.find_send_entry(entry);
                assert(entry != NULL);
                ch->next_hop() = entry->next_addr();
                if (use_mac())
                {
                    ch->xmit_failure_   = olsr_mac_failed_callback;
                    ch->xmit_failure_data_  = (void*)this;
                }
            }
        }

        Scheduler::getInstance().schedule(target_, p, 0.0);
    }
}
#endif
///
/// \brief Enques an %OLSR message which will be sent with a delay of (0, delay].
///
/// This buffering system is used in order to piggyback several %OLSR messages in
/// a same %OLSR packet.
///
/// \param msg the %OLSR message which must be sent.
/// \param delay maximum delay the %OLSR message is going to be buffered.
///
// void OLSR::enque_msg(OLSR_msg& msg, double delay) from OLSR

///
/// \brief Creates as many %OLSR packets as needed in order to send all buffered
/// %OLSR messages.
///
/// Maximum number of messages which can be contained in an %OLSR packet is
/// dictated by OLSR_MAX_MSGS constant.
///
void
OLSR_ETX::send_pkt()
{
    int num_msgs = msgs_.size();
    if (num_msgs == 0)
        return;

    Uint128 destAdd;
    destAdd = IPAddress::ALLONES_ADDRESS;
    // Calculates the number of needed packets
    int num_pkts = (num_msgs % OLSR_ETX_MAX_MSGS == 0) ? num_msgs / OLSR_ETX_MAX_MSGS :
                   (num_msgs / OLSR_ETX_MAX_MSGS + 1);

    for (int i = 0; i < num_pkts; i++)
    {
        /// Link delay extension
        if (parameter_.link_delay())
        {
            // We are going to use link delay extension
            // to define routes to be selected
            OLSR_pkt* op1   = new OLSR_pkt;
            op1->setName("OLSR_ETX pkt");
            // Duplicated packet...
            OLSR_pkt* op2;
            op1->setPkt_seq_num( pkt_seq());
            if (i == 0)
            {
                op1->setByteLength(OLSR_ETX_CAPPROBE_PACKET_SIZE);
                op1->setSn(cap_sn());

                // Allocate room for a duplicated packet...
                op2 = new OLSR_pkt;
                op2->setName("OLSR_ETX pkt");
                op2->setByteLength(OLSR_ETX_CAPPROBE_PACKET_SIZE);
                // duplicated packet sequence no ...
                op2->setPkt_seq_num(op1->pkt_seq_num());
                // but different cap sequence no
                op2->setSn(cap_sn());
            }
            else
            {
                op1->setByteLength(OLSR_ETX_PKT_HDR_SIZE);
                op1->setSn(0);
            }

            int j = 0;
            for (std::vector<OLSR_ETX_msg>::iterator it = msgs_.begin(); it != msgs_.end();)
            {
                if (j == OLSR_ETX_MAX_MSGS)
                    break;
                op1->setMsgArraySize(j+1);
                op1->setMsg(j++,*it);

                if (i != 0)
                    op1->setByteLength(op1->getByteLength()+(*it).size());
                else /* if (i == 0) */
                {
                    op2->setMsgArraySize(j+1);
                    op2->setMsg(j++,*it);
                }
                it = msgs_.erase(it);
            }


            // Marking packet timestamp
            op1->setSend_time(CURRENT_TIME);
            if (i == 0)
                op2->setSend_time(op1->send_time());
            // Sending packet pair
            sendToIp (op1, RT_PORT,destAdd, RT_PORT,IP_DEF_TTL,(nsaddr_t)0);
            if (i == 0)
                sendToIp (op2, RT_PORT,destAdd, RT_PORT,IP_DEF_TTL,(nsaddr_t)0);
        }
        else
        {
            OLSR_pkt* op    = new OLSR_pkt;

            op->setByteLength( OLSR_ETX_PKT_HDR_SIZE );
            op->setPkt_seq_num( pkt_seq());
            int j = 0;
            for (std::vector<OLSR_msg>::iterator it = msgs_.begin(); it != msgs_.end();)
            {
                if (j == OLSR_MAX_MSGS)
                    break;

                op->setMsgArraySize(j+1);
                op->setMsg(j++,*it);
                op->setByteLength(op->getByteLength()+(*it).size());

                it = msgs_.erase(it);
            }
            sendToIp (op, RT_PORT,destAdd, RT_PORT,IP_DEF_TTL,(nsaddr_t)0);

        }
    }
}

///
/// \brief Creates a new %OLSR HELLO message which is buffered to be sent later on.
///
void
OLSR_ETX::send_hello()
{
    OLSR_msg msg;
    double now      = CURRENT_TIME;
    msg.msg_type()      = OLSR_HELLO_MSG;
    msg.vtime()     = OLSR::seconds_to_emf(OLSR_NEIGHB_HOLD_TIME);
    msg.orig_addr()     = ra_addr();
    msg.ttl()       = 1;
    msg.hop_count()     = 0;
    msg.msg_seq_num()   = msg_seq();

    msg.hello().reserved()      = 0;
    msg.hello().htime()     = OLSR::seconds_to_emf(hello_ival());
    msg.hello().willingness()   = willingness();
    msg.hello().count       = 0;


    std::map<uint8_t, int> linkcodes_count;
    for (linkset_t::iterator it = linkset().begin(); it != linkset().end(); it++)
    {
        OLSR_ETX_link_tuple* link_tuple = dynamic_cast<OLSR_ETX_link_tuple*>(*it);
        if (!link_tuple)
            opp_error("\n Error conversion link tuple");


        if (link_tuple->local_iface_addr() == ra_addr() && link_tuple->time() >= now)
        {
            uint8_t link_type, nb_type, link_code;

            // Establishes link type
            if (use_mac() && link_tuple->lost_time() >= now)
                link_type = OLSR_LOST_LINK;
            else if (link_tuple->sym_time() >= now)
                link_type = OLSR_SYM_LINK;
            else if (link_tuple->asym_time() >= now)
                link_type = OLSR_ASYM_LINK;
            else
                link_type = OLSR_LOST_LINK;
            // Establishes neighbor type.
            if (state_.find_mpr_addr(get_main_addr(link_tuple->nb_iface_addr())))
                nb_type = OLSR_MPR_NEIGH;
            else
            {
                bool ok = false;
                for (nbset_t::iterator nb_it = nbset().begin();
                        nb_it != nbset().end();
                        nb_it++)
                {
                    OLSR_nb_tuple* nb_tuple = *nb_it;
                    if (nb_tuple->nb_main_addr() == link_tuple->nb_iface_addr())
                    {
                        if (nb_tuple->getStatus() == OLSR_STATUS_SYM)
                            nb_type = OLSR_SYM_NEIGH;
                        else if (nb_tuple->getStatus() == OLSR_STATUS_NOT_SYM)
                            nb_type = OLSR_NOT_NEIGH;
                        else
                        {
                            fprintf(stderr, "There is a neighbor tuple"
                                    " with an unknown status!\n");
                            exit(1);
                        }
                        ok = true;
                        break;
                    }
                }
                if (!ok)
                {
                    fprintf(stderr, "Link tuple has no corresponding"
                            " Neighbor tuple\n");
                    exit(1);
                }
            }

            int count = msg.hello().count;

            link_code = (link_type & 0x03) | ((nb_type << 2) & 0x0f);
            std::map<uint8_t, int>::iterator pos = linkcodes_count.find(link_code);
            if (pos == linkcodes_count.end())
            {
                linkcodes_count[link_code] = count;
                assert(count >= 0 && count < OLSR_MAX_HELLOS);
                msg.hello().hello_msg(count).count = 0;
                msg.hello().hello_msg(count).link_code() = link_code;
                msg.hello().hello_msg(count).reserved() = 0;
                msg.hello().hello_msg(count).set_qos_behaviour(parameter_.link_quality());
                msg.hello().count++;
            }
            else
                count = (*pos).second;

            int i = msg.hello().hello_msg(count).count;
            assert(count >= 0 && count < OLSR_MAX_HELLOS);
            assert(i >= 0 && i < OLSR_MAX_ADDRS);

            msg.hello().hello_msg(count).nb_iface_addr(i) =
                link_tuple->nb_iface_addr();


//////////////////////////////
/// Link Quality extensions
//////////////////////////////
            // publish link quality information we have found out and the one we
            // have received from our neighbors
            msg.hello().hello_msg(count).nb_etx_iface_addr(i).link_quality() =
                link_tuple->link_quality();
            msg.hello().hello_msg(count).nb_etx_iface_addr(i).nb_link_quality() =
                link_tuple->nb_link_quality();
            /// Link delay extension
            msg.hello().hello_msg(count).nb_etx_iface_addr(i).link_delay() =
                link_tuple->link_delay();
            msg.hello().hello_msg(count).nb_etx_iface_addr(i).nb_link_delay() =
                link_tuple->nb_link_delay();
// End link QoS

            msg.hello().hello_msg(count).count++;
            msg.hello().hello_msg(count).link_msg_size() =
                msg.hello().hello_msg(count).size();



        }
    }

    msg.msg_size() = msg.size();

    enque_msg(msg, JITTER);
}

///
/// \brief Creates a new %OLSR_ETX TC message which is buffered to be sent later on.
///
void
OLSR_ETX::send_tc()
{
    OLSR_msg msg;
    msg.msg_type() = OLSR_ETX_TC_MSG;
    msg.vtime() = OLSR::seconds_to_emf(OLSR_ETX_TOP_HOLD_TIME);
    msg.orig_addr() = ra_addr();
    if (parameter_.fish_eye())
    {
        msg.ttl() = tc_msg_ttl_ [tc_msg_ttl_index_];
        tc_msg_ttl_index_ = (tc_msg_ttl_index_ + 1) % (MAX_TC_MSG_TTL);
    }
    else
    {
        msg.ttl() = 255;
    }
    msg.hop_count() = 0;
    msg.msg_seq_num()  = msg_seq();

    msg.tc().ansn() = ansn_;
    msg.tc().reserved()  = 0;
    msg.tc().count = 0;
    msg.tc().set_qos_behaviour(parameter_.link_quality());

    //switch (parameter_.mpr_algorithm()) {
    //case OLSR_ETX_MPR_OLSRD:
    // Reported by Mohamed Belhassen
    switch (parameter_.tc_redundancy())
    {
    case OLSR_ETX_TC_ALL_NEIGHBOR_SET_REDUNDANCY:

        // Report all 1 hop neighbors we have
        for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
        {
            OLSR_ETX_nb_tuple* nb_tuple = *it;
            int count = msg.tc().count;
            OLSR_ETX_link_tuple *link_tuple;

            if (nb_tuple->getStatus() == OLSR_ETX_STATUS_SYM)
            {
                assert(count >= 0 && count < OLSR_ETX_MAX_ADDRS);
                // Report link quality and neighbor link quality of the best link that we have
                // to this node.
                link_tuple = state_.find_best_sym_link_tuple (nb_tuple->nb_main_addr(), CURRENT_TIME);
                if (link_tuple != NULL)
                {
                    msg.tc().nb_etx_main_addr(count).iface_address() = nb_tuple->nb_main_addr();
                    msg.tc().nb_etx_main_addr(count).link_quality() = link_tuple->link_quality();
                    msg.tc().nb_etx_main_addr(count).nb_link_quality() =
                        link_tuple->nb_link_quality();
                    /// Link delay extension
                    msg.tc().nb_etx_main_addr(count).link_delay() = link_tuple->link_delay();
                    msg.tc().nb_etx_main_addr(count).nb_link_delay() =
                        link_tuple->nb_link_delay();
                    msg.tc().count++;
                }
            }
        }
        break;

    default:
        //if (parameter_.tc_redundancy() & OLSR_ETX_TC_REDUNDANCY_MPR_SEL_SET) {
        // Reported by Mohamed Belhassen
        if (parameter_.tc_redundancy()==OLSR_ETX_TC_MPR_PLUS_MPR_SEL_SET_REDUNDANCY)
        {
            // Report our MPR selector set
            for (mprselset_t::iterator it = mprselset().begin(); it != mprselset().end(); it++)
            {
                OLSR_ETX_mprsel_tuple* mprsel_tuple = *it;
                int count = msg.tc().count;
                OLSR_ETX_link_tuple *link_tuple;

                assert(count >= 0 && count < OLSR_ETX_MAX_ADDRS);
                // Report link quality and neighbor link quality of the best link that we have
                // to this node.
                link_tuple = state_.find_best_sym_link_tuple (mprsel_tuple->main_addr(), CURRENT_TIME);
                if (link_tuple != NULL)
                {
                    msg.tc().nb_etx_main_addr(count).iface_address() = mprsel_tuple->main_addr();
                    msg.tc().nb_etx_main_addr(count).link_quality() = link_tuple->link_quality();
                    msg.tc().nb_etx_main_addr(count).nb_link_quality() =
                        link_tuple->nb_link_quality();
                    /// Link delay extension
                    msg.tc().nb_etx_main_addr(count).link_delay() = link_tuple->link_delay();
                    msg.tc().nb_etx_main_addr(count).nb_link_delay() =
                        link_tuple->nb_link_delay();
                    msg.tc().count++;
                }
            }
        }

        //  if (parameter_.tc_redundancy() & OLSR_ETX_TC_REDUNDANCY_MPR_SET) {
        // Reported by Mohamed Belhassen
        if ((parameter_.tc_redundancy()==OLSR_ETX_TC_MPR_PLUS_MPR_SEL_SET_REDUNDANCY)||(parameter_.tc_redundancy()==OLSR_ETX_TC_MPR_SEL_SET_REDUNDANCY))
        {
            // Also report our MPR set
            for (mprset_t::iterator it = mprset().begin(); it != mprset().end(); it++)
            {
                nsaddr_t mpr_addr = *it;
                int count = msg.tc().count;
                OLSR_ETX_link_tuple *link_tuple;

                assert(count >= 0 && count < OLSR_ETX_MAX_ADDRS);
                // Report link quality and neighbor link quality of the best link that we have
                // to this node.
                link_tuple = state_.find_best_sym_link_tuple (mpr_addr, CURRENT_TIME);
                if (link_tuple != NULL)
                {
                    msg.tc().nb_etx_main_addr(count).iface_address() = mpr_addr;
                    msg.tc().nb_etx_main_addr(count).link_quality() = link_tuple->link_quality();
                    msg.tc().nb_etx_main_addr(count).nb_link_quality() =
                        link_tuple->nb_link_quality();
                    /// Link delay extension
                    msg.tc().nb_etx_main_addr(count).link_delay() = link_tuple->link_delay();
                    msg.tc().nb_etx_main_addr(count).nb_link_delay() =
                        link_tuple->nb_link_delay();
                    msg.tc().count++;
                }
            }
        }
        break;
    }
    msg.msg_size() = msg.size();
    enque_msg(msg, JITTER);
}


///
/// \brief Creates a new %OLSR MID message which is buffered to be sent later on.
/// \warning This message is never invoked because there is no support for multiple interfaces.
///
//void OLSR::send_mid()  from OLSR

///
/// \brief  Updates Link Set according to a new received HELLO message (following RFC 3626
///     specification). Neighbor Set is also updated if needed.
///
/// \param msg the OLSR message which contains the HELLO message.
/// \param receiver_iface the address of the interface where the message was received from.
/// \param sender_iface the address of the interface where the message was sent from.
///
void
OLSR_ETX::link_sensing
(OLSR_msg& msg, const nsaddr_t &receiver_iface, const nsaddr_t &sender_iface,
 uint16_t pkt_seq_num,const int & index)
{
    OLSR_hello& hello  = msg.hello();
    double now = CURRENT_TIME;
    bool updated = false;
    bool created = false;

    // 1 Upon receiving a HELLO message, if there exists no link tuple
    //   with L_neighbor_iface_addr == Source Address a new tuple is created with
    //           L_neighbor_iface_addr = Source Address
    //           L_local_iface_addr    = Address of the interface
    //                                   which received the
    //                                   HELLO message
    //           L_SYM_time            = current time - 1 (expired)
    //           L_time                = current time + validity time
    OLSR_ETX_link_tuple* link_tuple = NULL;
    OLSR_link_tuple* link_tuple_aux = state_.find_link_tuple(sender_iface);
    if (link_tuple_aux)
    {
        link_tuple = dynamic_cast<OLSR_ETX_link_tuple*>(link_tuple_aux);
        if (link_tuple== NULL)
            opp_error("\n Error conversion link tuple");
    }
    if (link_tuple == NULL)
    {
        // We have to create a new tuple
        link_tuple = new OLSR_ETX_link_tuple;
// Omnet
        link_tuple->set_qos_behaviour(parameter_);
        link_tuple->set_owner(this);
//
        link_tuple->nb_iface_addr()  = sender_iface;
        link_tuple->local_iface_addr()  = receiver_iface;
        link_tuple->sym_time() = now - 1;
        link_tuple->lost_time() = 0.0;
        link_tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
        // Init link quality information struct for this link tuple
        link_tuple->link_quality_init (pkt_seq_num, DEFAULT_LOSS_WINDOW_SIZE);
        /// Link delay extension
        link_tuple->link_delay_init ();
        // This call will be also in charge of creating a new tuple in
        // the neighbor set
        add_link_tuple(link_tuple, hello.willingness());
        created = true;
    }
    else
        updated = true;

    // Account link quality information for this link
    link_tuple->receive (pkt_seq_num, OLSR::emf_to_seconds(hello.htime()));

    // 2    The tuple (existing or new) with:
    //           L_neighbor_iface_addr == Source Address
    //      is then modified as follows:
    //      2.1  L_ASYM_time = current time + validity time;

    link_tuple->asym_time() = now + OLSR::emf_to_seconds(msg.vtime());
    assert(hello.count >= 0 && hello.count <= OLSR_ETX_MAX_HELLOS);
    for (int i = 0; i < hello.count; i++)
    {
        OLSR_ETX_hello_msg& hello_msg = hello.hello_msg(i);
        int lt = hello_msg.link_code() & 0x03;
        int nt = hello_msg.link_code() >> 2;

        // We must not process invalid advertised links
        if ((lt == OLSR_ETX_SYM_LINK && nt == OLSR_ETX_NOT_NEIGH) ||
                (nt != OLSR_ETX_SYM_NEIGH && nt != OLSR_ETX_MPR_NEIGH
                 && nt != OLSR_ETX_NOT_NEIGH))
            continue;

        assert(hello_msg.count >= 0 && hello_msg.count <= OLSR_ETX_MAX_ADDRS);
        for (int j = 0; j < hello_msg.count; j++)
        {
            //      2.2  if the node finds the address of the interface which
            //           received the HELLO message among the addresses listed in
            //           the link message then the tuple is modified as follows:
            if (hello_msg.nb_etx_iface_addr(j).iface_address() == receiver_iface)
            {
                //           2.2.1 if Link Type is equal to LOST_LINK then
                //                     L_SYM_time = current time - 1 (i.e., expired)
                if (lt == OLSR_ETX_LOST_LINK)
                {
                    link_tuple->sym_time() = now - 1;
                    updated = true;
                }
                //           2.2.2 else if Link Type is equal to SYM_LINK or ASYM_LINK
                //                then
                //                     L_SYM_time = current time + validity time,
                //                     L_time     = L_SYM_time + NEIGHB_HOLD_TIME
                else if (lt == OLSR_ETX_SYM_LINK || lt == OLSR_ETX_ASYM_LINK)
                {
                    link_tuple->sym_time() =
                        now + OLSR::emf_to_seconds(msg.vtime());
                    link_tuple->time() =
                        link_tuple->sym_time() + OLSR_ETX_NEIGHB_HOLD_TIME;
                    link_tuple->lost_time() = 0.0;
                    updated = true;
                }
                // Update our neighbor's idea of link quality and link delay
                link_tuple->update_link_quality (hello_msg.nb_etx_iface_addr(j).link_quality());
                link_tuple->update_link_delay (hello_msg.nb_etx_iface_addr(j).link_delay());

                break;
            }
        }

    }

    //      2.3  L_time = max(L_time, L_ASYM_time)
    link_tuple->time() = MAX(link_tuple->time(), link_tuple->asym_time());

    if (updated)
        updated_link_tuple(link_tuple);
    // Schedules link tuple deletion
    if (created && link_tuple != NULL)
    {
        OLSR_LinkTupleTimer* link_timer =
            new OLSR_LinkTupleTimer(this, link_tuple);
        link_timer->resched(DELAY(MIN(link_tuple->time(), link_tuple->sym_time())));
    }
}

///
/// \brief  Updates the Neighbor Set according to the information contained in a new received
///     HELLO message (following RFC 3626).
///
/// \param msg the %OLSR message which contains the HELLO message.
///
// void OLSR::populate_nbset(OLSR_msg& msg) from OLSR


///
/// \brief  Updates the 2-hop Neighbor Set according to the information contained in a new
///     received HELLO message (following RFC 3626).
///
/// \param msg the %OLSR message which contains the HELLO message.
///
void
OLSR_ETX::populate_nb2hopset(OLSR_msg& msg)
{
    OLSR_hello& hello  = msg.hello();
    double now = CURRENT_TIME;

    // Upon receiving a HELLO message, the "validity time" MUST be computed
    // from the Vtime field of the message header (see section 3.3.2).
    double validity_time = now + OLSR::emf_to_seconds(msg.vtime());

    //  If the Originator Address is the main address of a
    //  L_neighbor_iface_addr from a link tuple included in the Link Set with
    //         L_SYM_time >= current time (not expired)
    //  then the 2-hop Neighbor Set SHOULD be updated as follows:
    for (linkset_t::iterator it_lt = linkset().begin(); it_lt != linkset().end(); it_lt++)
    {
        OLSR_ETX_link_tuple* link_tuple = dynamic_cast<OLSR_ETX_link_tuple*>(*it_lt);
        if (!link_tuple)
            opp_error("\n Error conversion link tuple");



        if (get_main_addr(link_tuple->nb_iface_addr()) == msg.orig_addr() &&
                link_tuple->sym_time() >= now)
        {
            assert(hello.count >= 0 && hello.count <= OLSR_ETX_MAX_HELLOS);

            for (int i = 0; i < hello.count; i++)
            {
                OLSR_ETX_hello_msg& hello_msg = hello.hello_msg(i);
                int nt = hello_msg.link_code() >> 2;
                assert(hello_msg.count >= 0 && hello_msg.count <= OLSR_ETX_MAX_ADDRS);

                // 1    for each address (henceforth: 2-hop neighbor address), listed
                //      in the HELLO message with Neighbor Type equal to SYM_NEIGH or
                //      MPR_NEIGH:
                if (nt == OLSR_ETX_SYM_NEIGH || nt == OLSR_ETX_MPR_NEIGH)
                {

                    for (int j = 0; j < hello_msg.count; j++)
                    {
                        // Weverton Cordeiro: was not verifying 2hop main addr
                        nsaddr_t nb2hop_addr = get_main_addr(hello_msg.nb_etx_iface_addr(j).iface_address());

                        // 1.1  if the main address of the 2-hop neighbor address = main
                        // address of the receiving node: silently discard the 2-hop neighbor address.
                        if (nb2hop_addr == ra_addr())
                            continue;
                        // 1.2  Otherwise, a 2-hop tuple is created with:
                        // N_neighbor_main_addr =  Originator Address;
                        // N_2hop_addr          =  main address of the 2-hop neighbor;
                        // N_time               =  current time + validity time.
                        OLSR_ETX_nb2hop_tuple* nb2hop_tuple = NULL;
                        OLSR_nb2hop_tuple* nb2hop_tuple_aux =
                            state_.find_nb2hop_tuple(msg.orig_addr(), nb2hop_addr);
                        if (nb2hop_tuple_aux)
                        {
                            nb2hop_tuple = dynamic_cast<OLSR_ETX_nb2hop_tuple*>(nb2hop_tuple_aux);
                            if (!nb2hop_tuple)
                                opp_error("\n Error conversion nd2hop tuple");
                        }

                        if (nb2hop_tuple == NULL)
                        {
                            nb2hop_tuple = new OLSR_ETX_nb2hop_tuple;
                            nb2hop_tuple->nb_main_addr() = msg.orig_addr();
                            nb2hop_tuple->nb2hop_addr() = nb2hop_addr;
                            nb2hop_tuple->set_qos_behaviour(parameter_.link_quality());

                            // Init link quality and link delay information
                            nb2hop_tuple->update_link_quality(0.0, 0.0);
                            nb2hop_tuple->update_link_delay(1.0, 1.0);

                            add_nb2hop_tuple(nb2hop_tuple);
                            nb2hop_tuple->time() = validity_time;
                            // Schedules nb2hop tuple deletion
                            OLSR_Nb2hopTupleTimer* nb2hop_timer =
                                new OLSR_Nb2hopTupleTimer(this, nb2hop_tuple);
                            nb2hop_timer->resched(DELAY(nb2hop_tuple->time()));
                        }
                        else
                            // This tuple may replace an older similar tuple with same
                            // N_neighbor_main_addr and N_2hop_addr values.
                            nb2hop_tuple->time() = validity_time;

                        // Update Link Quality information. Note: we only want information about the best link
                        switch (parameter_.link_quality())
                        {
                        case OLSR_ETX_BEHAVIOR_ETX:
                            if (hello_msg.nb_etx_iface_addr(j).etx() < nb2hop_tuple->etx())
                            {
                                nb2hop_tuple->update_link_quality(
                                    hello_msg.nb_etx_iface_addr(j).link_quality(),
                                    hello_msg.nb_etx_iface_addr(j).nb_link_quality());

                            }
                            break;

                        case OLSR_ETX_BEHAVIOR_ML:
                            if (hello_msg.nb_etx_iface_addr(j).etx() > nb2hop_tuple->etx())
                            {
                                nb2hop_tuple->update_link_quality(
                                    hello_msg.nb_etx_iface_addr(j).link_quality(),
                                    hello_msg.nb_etx_iface_addr(j).nb_link_quality());
                            }
                            break;

                        case OLSR_ETX_BEHAVIOR_NONE:
                        default:
                            //
                            break;
                        }
                        /// Link delay extension
                        if (hello_msg.nb_etx_iface_addr(j).nb_link_delay() < nb2hop_tuple->nb_link_delay())
                        {
                            nb2hop_tuple->update_link_delay(
                                hello_msg.nb_etx_iface_addr(j).link_delay(),
                                hello_msg.nb_etx_iface_addr(j).nb_link_delay());
                        }
                    }
                }
                // 2 For each 2-hop node listed in the HELLO message with Neighbor
                //   Type equal to NOT_NEIGH, all 2-hop tuples where:
                else if (nt == OLSR_ETX_NOT_NEIGH)
                {

                    for (int j = 0; j < hello_msg.count; j++)
                    {
                        nsaddr_t nb2hop_addr = get_main_addr(hello_msg.nb_etx_iface_addr(j).iface_address());

                        state_.erase_nb2hop_tuples(msg.orig_addr(), nb2hop_addr);
                    }
                }
            }
            // this hello message was already processed, and processing it for another symmetric
            // link we find in our link set will not make any new changes
            break;
        }
    }
}
///
/// \brief  Updates the MPR Selector Set according to the information contained in a new
///     received HELLO message (following RFC 3626).
///
/// \param msg the %OLSR message which contains the HELLO message.
///
//void OLSR::populate_mprselset(OLSR_msg& msg) from OLSR

///
/// \brief  Drops a given packet because it couldn't be delivered to the corresponding
///     destination by the MAC layer. This may cause a neighbor loss, and appropiate
///     actions are then taken.
///
/// \param p the packet which couldn't be delivered by the MAC layer.
///
// void OLSR::mac_failed(IPDatagram* p) from OLSR

///
/// \brief Schedule the timer used for sending HELLO messages.
///
// void OLSR::set_hello_timer() { from OLSR


///
/// \brief Schedule the timer used for sending TC messages.
///
// void OLSR::set_tc_timer() { From OLSR


///
/// \brief Schedule the timer used for sending MID messages.
///
// void OLSR::set_mid_timer() { from OLSR

///
/// \brief Performs all actions needed when a neighbor loss occurs.
///
/// Neighbor Set, 2-hop Neighbor Set, MPR Set and MPR Selector Set are updated.
///
/// \param tuple link tuple with the information of the link to the neighbor which has been lost.
///
void
OLSR_ETX::nb_loss(OLSR_link_tuple* tuple)
{
    debug("%f: Node %d detects neighbor %d loss\n", CURRENT_TIME,
          OLSR::node_id(ra_addr()), OLSR::node_id(tuple->nb_iface_addr()));

    updated_link_tuple(tuple);
    state_.erase_nb2hop_tuples(get_main_addr(tuple->nb_iface_addr()));
    state_.erase_mprsel_tuples(get_main_addr(tuple->nb_iface_addr()));

    switch (parameter_.mpr_algorithm())
    {
    case OLSR_ETX_MPR_R1:
        olsr_r1_mpr_computation();
        break;

    case OLSR_ETX_MPR_R2:
        olsr_r2_mpr_computation();
        break;

    case OLSR_ETX_MPR_QOLSR:
        qolsr_mpr_computation();
        break;

    case OLSR_ETX_MPR_OLSRD:
        olsrd_mpr_computation();
        break;

    case OLSR_ETX_DEFAULT_MPR:
    default:
        olsr_mpr_computation();
        break;
    }
    switch (parameter_.routing_algorithm())
    {
    case OLSR_ETX_DIJKSTRA_ALGORITHM:
        rtable_dijkstra_computation();
        break;

    default:
    case OLSR_ETX_DEFAULT_ALGORITHM:
        rtable_default_computation();
        break;
    }
}


///
/// \brief Adds a duplicate tuple to the Duplicate Set.
///
/// \param tuple the duplicate tuple to be added.
///
// void OLSR::add_dup_tuple(OLSR_dup_tuple* tuple) { from OLSR

///
/// \brief Removes a duplicate tuple from the Duplicate Set.
///
/// \param tuple the duplicate tuple to be removed.
///
// void OLSR::rm_dup_tuple(OLSR_dup_tuple* tuple) { From OLSR


///
/// \brief Adds a link tuple to the Link Set (and an associated neighbor tuple to the Neighbor Set).
///
/// \param tuple the link tuple to be added.
/// \param willingness willingness of the node which is going to be inserted in the Neighbor Set.
///
// void OLSR::add_link_tuple(OLSR_link_tuple* tuple, uint8_t  willingness) { from OLSR

///
/// \brief Removes a link tuple from the Link Set.
///
/// \param tuple the link tuple to be removed.
///
// void OLSR::rm_link_tuple(OLSR_link_tuple* tuple) { from OLSR

///
/// \brief  This function is invoked when a link tuple is updated. Its aim is to
///     also update the corresponding neighbor tuple if it is needed.
///
/// \param tuple the link tuple which has been updated.
///
// void OLSR::updated_link_tuple(OLSR_link_tuple* tuple) { from OLSR


///
/// \brief Adds a neighbor tuple to the Neighbor Set.
///
/// \param tuple the neighbor tuple to be added.
///
// void OLSR::add_nb_tuple(OLSR_nb_tuple* tuple) { from OLSR

///
/// \brief Removes a neighbor tuple from the Neighbor Set.
///
/// \param tuple the neighbor tuple to be removed.
///
// void OLSR::rm_nb_tuple(OLSR_nb_tuple* tuple) { from OLSR


///
/// \brief Adds a 2-hop neighbor tuple to the 2-hop Neighbor Set.
///
/// \param tuple the 2-hop neighbor tuple to be added.
///
// void OLSR::add_nb2hop_tuple(OLSR_nb2hop_tuple* tuple) { from OLSR

///
/// \brief Removes a 2-hop neighbor tuple from the 2-hop Neighbor Set.
///
/// \param tuple the 2-hop neighbor tuple to be removed.
///
// void OLSR::rm_nb2hop_tuple(OLSR_nb2hop_tuple* tuple) { from OLSR

///
/// \brief Adds an MPR selector tuple to the MPR Selector Set.
///
/// Advertised Neighbor Sequence Number (ANSN) is also updated.
///
/// \param tuple the MPR selector tuple to be added.
///
// void OLSR::add_mprsel_tuple(OLSR_mprsel_tuple* tuple) { from OLSR

///
/// \brief Removes an MPR selector tuple from the MPR Selector Set.
///
/// Advertised Neighbor Sequence Number (ANSN) is also updated.
///
/// \param tuple the MPR selector tuple to be removed.
///
// void OLSR::rm_mprsel_tuple(OLSR_mprsel_tuple* tuple) { from OLSR

///
/// \brief Adds a topology tuple to the Topology Set.
///
/// \param tuple the topology tuple to be added.
///
// void OLSR::add_topology_tuple(OLSR_topology_tuple* tuple) { from OSLR

///
/// \brief Removes a topology tuple from the Topology Set.
///
/// \param tuple the topology tuple to be removed.
///
// void OLSR::rm_topology_tuple(OLSR_topology_tuple* tuple) {

///
/// \brief Adds an interface association tuple to the Interface Association Set.
///
/// \param tuple the interface association tuple to be added.
///
// void OLSR::add_ifaceassoc_tuple(OLSR_iface_assoc_tuple* tuple) {

///
/// \brief Removes an interface association tuple from the Interface Association Set.
///
/// \param tuple the interface association tuple to be removed.
///
//void OLSR::rm_ifaceassoc_tuple(OLSR_iface_assoc_tuple* tuple) {

///
/// \brief Gets the main address associated with a given interface address.
///
/// \param iface_addr the interface address.
/// \return the corresponding main address.
///
// nsaddr_t OLSR::get_main_addr(nsaddr_t iface_addr) {


void OLSR_ETX::finish()
{
    rtable_.clear();
    msgs_.clear();
    delete state_etx_ptr;
    state_ptr = state_etx_ptr=NULL;
    //cancelAndDelete(&hello_timer_);
    //cancelAndDelete(&tc_timer_);
    //cancelAndDelete(&mid_timer_);
    //cancelAndDelete(&link_quality_timer_);
    helloTimer= NULL;   ///< Timer for sending HELLO messages.
    tcTimer= NULL;  ///< Timer for sending TC messages.
    midTimer = NULL;    ///< Timer for sending MID messages.
    linkQualityTimer=NULL;
}

OLSR_ETX::~OLSR_ETX()
{

    rtable_.clear();
    msgs_.clear();
    if (state_etx_ptr)
    {
        delete state_etx_ptr;
        state_ptr=state_etx_ptr = NULL;
    }

    /*
        mprset().clear();
        mprselset().clear();
        linkset().clear();
        nbset().clear();
        nb2hopset().clear();
        topologyset().clear();
        dupset().clear();
        ifaceassocset().clear();
    */
    /*
        if (&hello_timer_!=NULL)
            cancelAndDelete(&hello_timer_);
        if (&tc_timer_!=NULL)
            cancelAndDelete(&tc_timer_);
        if (&mid_timer_!=NULL)
            cancelAndDelete(&mid_timer_);
        if (&link_quality_timer_!=NULL)
            cancelAndDelete(&link_quality_timer_);
        */
    if (timerMessage)
    {
        cancelAndDelete(timerMessage);
        timerMessage=NULL;
    }

    while (timerQueuePtr && timerQueuePtr->size()>0)
    {
        OLSR_Timer * timer = timerQueuePtr->begin()->second;
        timerQueuePtr->erase(timerQueuePtr->begin());
        timer->setTuple(NULL);
        if (helloTimer==timer)
            helloTimer=NULL;
        else if (tcTimer==timer)
            tcTimer=NULL;
        else if (midTimer==timer)
            midTimer=NULL;
        else if (linkQualityTimer==timer)
            linkQualityTimer=NULL;
        delete timer;
    }

    if (helloTimer)
    {
        delete helloTimer;
        helloTimer=NULL;
    }
    if (tcTimer)
    {
        delete tcTimer;
        tcTimer=NULL;
    }
    if (midTimer)
    {
        delete midTimer;
        midTimer=NULL;
    }
    if (linkQualityTimer)
    {
        delete linkQualityTimer;
        linkQualityTimer=NULL;
    }
    if (timerQueuePtr)
    {
        delete timerQueuePtr;
        timerQueuePtr = NULL;
    }
}


///
/// \brief Verify if a link tuple has reached a timeout in the expected time to receive a new packet
///
void
OLSR_ETX::link_quality ()
{
    double now = CURRENT_TIME;
    for (linkset_t::iterator it = state_.linkset_.begin(); it != state_.linkset_.end(); it++)
    {
        OLSR_ETX_link_tuple* tuple = dynamic_cast<OLSR_ETX_link_tuple*>(*it);
        if (tuple->next_hello() < now)
            tuple->packet_timeout();
    }
}
