/***************************************************************************
 *   Copyright (C) 2004 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *   Adapted for omnetpp                                                   *
 *   2008 Alfonso Ariza Quintana aarizaq@uma.es                            *
 *                                                                         *
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
/// \file   OLSR.cc
/// \brief  Implementation of OLSR agent and related classes.
///
/// This is the main file of this software because %OLSR's behaviour is
/// implemented here.
///


#include <math.h>
#include <limits.h>

#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

#include "inet/routing/extras/olsr/OLSRpkt_m.h"
#include "inet/routing/extras/olsr/OLSR.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"

namespace inet {

namespace inetmanet {

/// Length (in bytes) of UDP header.
#define UDP_HDR_LEN 8
/// Port Number
#define RT_PORT 698
#define IP_DEF_TTL 32

#define MULTIPLE_IFACES_SUPPORT
#define state_      (*state_ptr)

///
/// \brief Function called by MAC layer when cannot deliver a packet.
///
/// \param p Packet which couldn't be delivered.
/// \param arg OLSR agent passed for a callback.
///

Define_Module(OLSR);


std::ostream& operator<<(std::ostream& os, const OLSR_rt_entry& e)
{
    os << "dest:"<< e.dest_addr_.str() << " ";
    os << "gw:" << e.next_addr_.str() << " ";
    os << "iface:" << e.iface_addr_.str() << " ";
    os << "dist:" << e.dist_ << " ";
    return os;
};


uint32 OlsrAddressSize::ADDR_SIZE = ADDR_SIZE_DEFAULT;

/********** Timers **********/

void OLSR_Timer::removeTimer()
{
    removeQueueTimer();
}

OLSR_Timer::OLSR_Timer(OLSR* agent) : cOwnedObject("OLSR_Timer")
{
    agent_ = agent;
    tuple_ = NULL;
}

OLSR_Timer::~OLSR_Timer()
{
    removeTimer();
}

OLSR_Timer::OLSR_Timer() : cOwnedObject("OLSR_Timer")
{
    agent_ = dynamic_cast <OLSR*> (this->getOwner());
    if (agent_==NULL)
        throw cRuntimeError("timer ower is bad");
    tuple_ = NULL;
}

void OLSR_Timer::removeQueueTimer()
{
    TimerQueue::iterator it;
    for (it=agent_->timerQueuePtr->begin(); it != agent_->timerQueuePtr->end(); it++ )
    {
        if (it->second==this)
        {
            agent_->timerQueuePtr->erase(it);
            return;
        }
    }
}

void OLSR_Timer::resched(double time)
{
    removeQueueTimer();
    agent_->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+time, this));
    //if (this->isScheduled())
    //  agent_->cancelEvent(this);
    // agent_->scheduleAt (simTime()+time,this);
}


///
/// \brief Sends a HELLO message and reschedules the HELLO timer.
/// \param e The event which has expired.
///
void
OLSR_HelloTimer::expire()
{
    agent_->send_hello();
    // agent_->scheduleAt(simTime()+agent_->hello_ival_- JITTER,this);
    agent_->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+agent_->hello_ival_- agent_->jitter(), this));
}

///
/// \brief Sends a TC message (if there exists any MPR selector) and reschedules the TC timer.
/// \param e The event which has expired.
///
void
OLSR_TcTimer::expire()
{
    if (agent_->mprselset().size() > 0)
        agent_->send_tc();
    // agent_->scheduleAt(simTime()+agent_->tc_ival_- JITTER,this);
    agent_->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+agent_->tc_ival_- agent_->jitter(), this));

}

///
/// \brief Sends a MID message (if the node has more than one interface) and resets the MID timer.
/// \warning Currently it does nothing because there is no support for multiple interfaces.
/// \param e The event which has expired.
///
void
OLSR_MidTimer::expire()
{
#ifdef MULTIPLE_IFACES_SUPPORT
    if (agent_->isInMacLayer())
        return; // not multi-interface support
    agent_->send_mid();
//  agent_->scheduleAt(simTime()+agent_->mid_ival_- JITTER,this);
    agent_->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+agent_->mid_ival_- agent_->jitter(), this));
#endif
}

///
/// \brief Removes tuple_ if expired. Else timer is rescheduled to expire at tuple_->time().
///
/// The task of actually removing the tuple is left to the OLSR agent.
///
/// \param e The event which has expired.
///


void
OLSR_DupTupleTimer::expire()
{
    OLSR_dup_tuple* tuple = dynamic_cast<OLSR_dup_tuple*> (tuple_);
    double time = tuple->time();
    if (time < SIMTIME_DBL(simTime()))
    {
        removeTimer();
        delete this;
    }
    else
    {
        // agent_->scheduleAt (simTime()+DELAY_T(time),this);
        agent_->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+DELAY_T(time), this));
    }
}

OLSR_DupTupleTimer::~OLSR_DupTupleTimer()
{
    removeTimer();
    if (!tuple_)
        return;
    OLSR_dup_tuple* tuple = dynamic_cast<OLSR_dup_tuple*> (tuple_);
    tuple->asocTimer = NULL;
    if (agent_->state_ptr==NULL)
        return;
    agent_->rm_dup_tuple(tuple);
    delete tuple_;
}

///
/// \brief Removes tuple_ if expired. Else if symmetric time
/// has expired then it is assumed a neighbor loss and agent_->nb_loss()
/// is called. In this case the timer is rescheduled to expire at
/// tuple_->time(). Otherwise the timer is rescheduled to expire at
/// the minimum between tuple_->time() and tuple_->sym_time().
///
/// The task of actually removing the tuple is left to the OLSR agent.
///
/// \param e The event which has expired.
///

OLSR_LinkTupleTimer::OLSR_LinkTupleTimer(OLSR* agent, OLSR_link_tuple* tuple) : OLSR_Timer(agent)
{
    tuple_ = tuple;
    tuple->asocTimer = this;
    first_time_ = true;
}

void
OLSR_LinkTupleTimer::expire()
{
    double now;
    now = SIMTIME_DBL(simTime());
    OLSR_link_tuple* tuple = dynamic_cast<OLSR_link_tuple*> (tuple_);
    if (tuple->time() < now)
    {
        removeTimer();
        delete this;
    }
    else if (tuple->sym_time() < now)
    {
        if (first_time_)
            first_time_ = false;
        else
            agent_->nb_loss(tuple);
        // agent_->scheduleAt (simTime()+DELAY_T(tuple_->time()),this);
        agent_->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+DELAY_T(tuple->time()), this));
    }
    else
    {
        // agent_->scheduleAt (simTime()+DELAY_T(MIN(tuple_->time(), tuple_->sym_time())),this);
        agent_->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+DELAY_T(MIN(tuple->time(), tuple->sym_time())), this));
    }
}

OLSR_LinkTupleTimer::~OLSR_LinkTupleTimer()
{
    removeTimer();
    if (!tuple_)
        return;
    OLSR_link_tuple* tuple = dynamic_cast<OLSR_link_tuple*> (tuple_);
    tuple->asocTimer = NULL;
    if (agent_->state_ptr==NULL)
        return;
    agent_->rm_link_tuple(tuple);
    agent_->setTopologyChanged(true);
    delete tuple_;
}

///
/// \brief Removes tuple_ if expired. Else the timer is rescheduled to expire at tuple_->time().
///
/// The task of actually removing the tuple is left to the OLSR agent.
///
/// \param e The event which has expired.
///

void
OLSR_Nb2hopTupleTimer::expire()
{
    OLSR_nb2hop_tuple* tuple = dynamic_cast<OLSR_nb2hop_tuple*> (tuple_);
    double time = tuple->time();
    if (time < SIMTIME_DBL(simTime()))
    {
        removeTimer();
        delete this;
    }
    else
    {
        // agent_->scheduleAt (simTime()+DELAY_T(time),this);
        agent_->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+DELAY_T(time), this));
    }
}

OLSR_Nb2hopTupleTimer::~OLSR_Nb2hopTupleTimer()
{
    removeTimer();
    if (!tuple_)
        return;
    OLSR_nb2hop_tuple* tuple = dynamic_cast<OLSR_nb2hop_tuple*> (tuple_);
    tuple->asocTimer = NULL;
    if (agent_->state_ptr==NULL)
        return;
    agent_->rm_nb2hop_tuple(tuple);
    agent_->setTopologyChanged(true);
    delete tuple_;
}


///
/// \brief Removes tuple_ if expired. Else the timer is rescheduled to expire at tuple_->time().
///
/// The task of actually removing the tuple is left to the OLSR agent.
///
/// \param e The event which has expired.
///


void
OLSR_MprSelTupleTimer::expire()
{
    OLSR_mprsel_tuple* tuple = dynamic_cast<OLSR_mprsel_tuple*> (tuple_);
    double time = tuple->time();
    if (time < SIMTIME_DBL(simTime()))
    {
        removeTimer();
        delete this;
    }
    else
    {
//      agent_->scheduleAt (simTime()+DELAY_T(time),this);
        agent_->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+DELAY_T(time), this));
    }
}

OLSR_MprSelTupleTimer::~OLSR_MprSelTupleTimer()
{
    removeTimer();
    if (!tuple_)
        return;
    OLSR_mprsel_tuple* tuple = dynamic_cast<OLSR_mprsel_tuple*> (tuple_);
    tuple->asocTimer = NULL;
    if (agent_->state_ptr==NULL)
        return;
    agent_->rm_mprsel_tuple(tuple);
    delete tuple_;
}



///
/// \brief Removes tuple_ if expired. Else the timer is rescheduled to expire at tuple_->time().
///
/// The task of actually removing the tuple is left to the OLSR agent.
///
/// \param e The event which has expired.
///

void
OLSR_TopologyTupleTimer::expire()
{
    OLSR_topology_tuple* tuple = dynamic_cast<OLSR_topology_tuple*> (tuple_);
    double time = tuple->time();
    if (time < SIMTIME_DBL(simTime()))
    {
        removeTimer();
        delete this;
    }
    else
    {
//      agent_->scheduleAt (simTime()+DELAY_T(time),this);
        agent_->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+DELAY_T(time), this));
    }
}

OLSR_TopologyTupleTimer::~OLSR_TopologyTupleTimer()
{
    removeTimer();
    if (!tuple_)
        return;
    OLSR_topology_tuple* tuple = dynamic_cast<OLSR_topology_tuple*> (tuple_);
    tuple->asocTimer = NULL;
    if (agent_->state_ptr==NULL)
        return;
    agent_->rm_topology_tuple(tuple);
    agent_->setTopologyChanged(true);
    delete tuple_;
}


///
/// \brief Removes tuple_ if expired. Else timer is rescheduled to expire at tuple_->time().
/// \warning Actually this is never invoked because there is no support for multiple interfaces.
/// \param e The event which has expired.
///

void OLSR_IfaceAssocTupleTimer::expire()
{
    OLSR_iface_assoc_tuple* tuple = dynamic_cast<OLSR_iface_assoc_tuple*> (tuple_);
    double time = tuple->time();
    if (time < SIMTIME_DBL(simTime()))
    {
        removeTimer();
        delete this;
    }
    else
    {
        //  agent_->scheduleAt (simTime()+DELAY_T(time),this);
        agent_->timerQueuePtr->insert(std::pair<simtime_t, OLSR_Timer *>(simTime()+DELAY_T(time), this));
    }
}

OLSR_IfaceAssocTupleTimer::~OLSR_IfaceAssocTupleTimer()
{
    removeTimer();
    if (!tuple_)
        return;
    OLSR_iface_assoc_tuple* tuple = dynamic_cast<OLSR_iface_assoc_tuple*> (tuple_);
    tuple->asocTimer = NULL;
    if (agent_->state_ptr==NULL)
        return;
    agent_->rm_ifaceassoc_tuple(tuple);
    agent_->setTopologyChanged(true);
    delete tuple_;
}


///
/// \brief Sends a control packet which must bear every message in the OLSR agent's buffer.
///
/// The task of actually sending the packet is left to the OLSR agent.
///
/// \param e The event which has expired.
///
void
OLSR_MsgTimer::expire()
{
    agent_->send_pkt();
    removeTimer();
    delete this;
}


/********** OLSR class **********/



///
///
void OLSR::initialize(int stage)
{
    ManetRoutingBase::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {

       if (isInMacLayer())
           OlsrAddressSize::ADDR_SIZE = 6;
    ///
    /// \brief Period at which a node must cite every link and every neighbor.
    ///
    /// We only use this value in order to define OLSR_NEIGHB_HOLD_TIME.
    ///
        OLSR_REFRESH_INTERVAL=par("OLSR_REFRESH_INTERVAL");

        //
        // Do some initializations
        willingness_ = par("Willingness");
        hello_ival_ = par("Hello_ival");
        tc_ival_ = par("Tc_ival");
        mid_ival_ = par("Mid_ival");
        use_mac_ = par("use_mac");

        OLSR_HELLO_INTERVAL = SIMTIME_DBL(tc_ival_);

     /// TC messages emission interval.
         OLSR_TC_INTERVAL = SIMTIME_DBL(tc_ival_);

     /// MID messages emission interval.
         OLSR_MID_INTERVAL = SIMTIME_DBL(mid_ival_);//   OLSR_TC_INTERVAL


        if (par("reduceFuncionality"))
            EV_TRACE << "reduceFuncionality true" << endl;
        else
            EV_TRACE << "reduceFuncionality false" << endl;

        pkt_seq_ = OLSR_MAX_SEQ_NUM;
        msg_seq_ = OLSR_MAX_SEQ_NUM;
        ansn_ = OLSR_MAX_SEQ_NUM;

        registerRoutingModule();
        ra_addr_ = getAddress();


        timerMessage = new cMessage();
        timerQueuePtr = new TimerQueue;


        useIndex = par("UseIndex");

        optimizedMid = par("optimizedMid");

        // Starts all timers

        helloTimer = new OLSR_HelloTimer(); ///< Timer for sending HELLO messages.
        tcTimer = new OLSR_TcTimer();   ///< Timer for sending TC messages.
        midTimer = new OLSR_MidTimer(); ///< Timer for sending MID messages.

        state_ptr = new OLSR_state();


        for (int i = 0; i< getNumWlanInterfaces(); i++)
        {
            // Create never expiring interface association tuple entries for our
            // own network interfaces, so that GetMainAddress () works to
            // translate the node's own interface addresses into the main address.
            OLSR_iface_assoc_tuple* tuple = new OLSR_iface_assoc_tuple;
            int index = getWlanInterfaceIndex(i);
            tuple->iface_addr() = getIfaceAddressFromIndex(index);
            tuple->main_addr() = ra_addr();
            tuple->time() = simtime_t::getMaxTime().dbl();
            tuple->local_iface_index() = index;
            add_ifaceassoc_tuple(tuple);
        }


        hello_timer_.resched(SIMTIME_DBL(hello_ival_));
        tc_timer_.resched(SIMTIME_DBL(hello_ival_));
        mid_timer_.resched(SIMTIME_DBL(hello_ival_));
        if (use_mac())
        {
            linkLayerFeeback();
        }
        scheduleNextEvent();

        WATCH_PTRMAP(rtable_.rt_);
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

void OLSR::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        //OLSR_Timer *timer=dynamic_cast<OLSR_Timer*>(msg);
        while (timerQueuePtr->begin()->first<=simTime())
        {
            OLSR_Timer *timer = timerQueuePtr->begin()->second;
            if (timer==NULL)
                throw cRuntimeError("timer ower is bad");
            else
            {
                timerQueuePtr->erase(timerQueuePtr->begin());
                timer->expire();
            }
        }
    }
    else
        recv_olsr(msg);

    scheduleNextEvent();
}

///
/// \brief Check if packet is OLSR
/// \param p received packet.
///

OLSR_pkt *
OLSR::check_packet(cPacket* msg, nsaddr_t &src_addr, int &index)
{
    cPacket *msg_aux = NULL;
    OLSR_pkt *op;
    index = getWlanInterfaceIndex(0);
    if (isInMacLayer())
    {
        if (!dynamic_cast<OLSR_pkt  *>(msg)) // Check if olsr packet
        {
            delete  msg;
            return NULL;
        }
        else
        {
            op = check_and_cast<OLSR_pkt  *>(msg);
            if (op->reduceFuncionality() && par("reduceFuncionality").boolValue())
            {
                delete msg;
                return NULL;
            }
            Ieee802Ctrl* ctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());
            src_addr = L3Address(ctrl->getSrc());
            delete ctrl;
            return dynamic_cast<OLSR_pkt  *>(msg);
        }

    }

    if (dynamic_cast<UDPPacket *>(msg)) // Check is Udp packet
    {
        UDPPacket * udpPacket = check_and_cast<UDPPacket*>(msg);
        if (udpPacket->getDestinationPort() != RT_PORT) // Check port
        {
            delete  msg;
            return NULL;
        }
        msg_aux = msg->decapsulate();
        if (!dynamic_cast<OLSR_pkt  *>(msg_aux)) // Check if olsr packet
        {
            delete  msg;
            delete msg_aux;
            return NULL;
        }

    }
    else
    {
        delete msg;
        return NULL;
    }
// Extract information and delete the cantainer without more use
    op = check_and_cast<OLSR_pkt  *>(msg_aux);
    if (op->reduceFuncionality() && par("reduceFuncionality"))
    {
        delete msg;
        delete op;
        return NULL;
    }
    INetworkProtocolControlInfo* controlInfo = check_and_cast<INetworkProtocolControlInfo*>(msg->removeControlInfo());
    src_addr = controlInfo->getSourceAddress();
    index = -1;
    InterfaceEntry * ie;

    for (int i=0; i<getNumWlanInterfaces(); i++)
    {
        ie = getWlanInterfaceEntry(i);
        if (ie->getInterfaceId() == controlInfo->getInterfaceId())
        {
            index = getWlanInterfaceIndex(i);
            break;
        }
    }

    delete controlInfo;
    delete msg;
    return op;
}
///
/// \brief Processes an incoming %OLSR packet following RFC 3626 specification.
/// \param p received packet.
///
void
OLSR::recv_olsr(cMessage* msg)
{

    OLSR_pkt* op;
    nsaddr_t src_addr;
    int index;

    // All routing messages are sent from and to port RT_PORT,
    // so we check it.

    op = check_packet(PK(msg), src_addr, index);
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
    nsaddr_t receiverIfaceAddr = getIfaceAddressFromIndex(index);
    for (int i = 0; i < (int) op->msgArraySize(); i++)
    {
        OLSR_msg& msg = op->msg(i);

        // If ttl is less than or equal to zero, or
        // the receiver is the same as the originator,
        // the message must be silently dropped
        // if (msg.ttl() <= 0 || msg.orig_addr() == ra_addr())
        if (msg.ttl() <= 0 || isLocalAddress(msg.orig_addr()))
            continue;

        // If the message has been processed it must not be
        // processed again
        bool do_forwarding = true;

        OLSR_dup_tuple* duplicated = state_.find_dup_tuple(msg.orig_addr(), msg.msg_seq_num());
        if (duplicated == NULL)
        {
            // Process the message according to its type
            if (msg.msg_type() == OLSR_HELLO_MSG)
                process_hello(msg, receiverIfaceAddr, src_addr, index);
            else if (msg.msg_type() == OLSR_TC_MSG)
                process_tc(msg, src_addr, index);
            else if (msg.msg_type() == OLSR_MID_MSG)
                process_mid(msg, src_addr, index);
            else
            {
                debug("%f: Node %s can not process OLSR packet because does not "
                      "implement OLSR type (%x)\n",
                      CURRENT_TIME,
                      getNodeId(ra_addr()),
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
                if (*it == receiverIfaceAddr)
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
                forward_default(msg, duplicated, receiverIfaceAddr, src_addr);
        }

    }
    delete op;

    // After processing all OLSR messages, we must recompute routing table
    rtable_computation();
}


 void CoverTwoHopNeighbors(const nsaddr_t &neighborMainAddr, nb2hopset_t & N2)
{ // first gather all 2-hop neighbors to be removed
    std::set<nsaddr_t> toRemove;
    for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++)
    {
        OLSR_nb2hop_tuple* twoHopNeigh = *it;
        if (twoHopNeigh->nb_main_addr() == neighborMainAddr)
        {
            toRemove.insert(twoHopNeigh->nb2hop_addr());
        }
    }
    // Now remove all matching records from N2
    for (nb2hopset_t::iterator it = N2.begin(); it != N2.end();)
    {
        OLSR_nb2hop_tuple* twoHopNeigh = *it;
        if (toRemove.find(twoHopNeigh->nb2hop_addr()) != toRemove.end())
            it = N2.erase(it);
        else
            it++;
    }
}

///
/// \brief Computates MPR set of a node following RFC 3626 hints.
///
#if 0
void
OLSR::mpr_computation()
{
    // MPR computation should be done for each interface. See section 8.3.1
    // (RFC 3626) for details.
    bool increment;

    state_.clear_mprset();

    nbset_t N; nb2hopset_t N2;
    // N is the subset of neighbors of the node, which are
    // neighbor "of the interface I"
    for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
        if ((*it)->getStatus() == OLSR_STATUS_SYM) // I think that we need this check
            N.push_back(*it);

    // N2 is the set of 2-hop neighbors reachable from "the interface
    // I", excluding:
    // (i)   the nodes only reachable by members of N with willingness WILL_NEVER
    // (ii)  the node performing the computation
    // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
    //       link to this node on some interface.
    for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++)
    {
        OLSR_nb2hop_tuple* nb2hop_tuple = *it;
        bool ok = true;

        OLSR_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_addr());
        if (nb_tuple == NULL)
            ok = false;
        else
        {
            nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_addr(), OLSR_WILL_NEVER);
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

    // 1. Start with an MPR set made of all members of N with
    // N_willingness equal to WILL_ALWAYS
    for (nbset_t::iterator it = N.begin(); it != N.end(); it++)
    {
        OLSR_nb_tuple* nb_tuple = *it;
        if (nb_tuple->willingness() == OLSR_WILL_ALWAYS)
            state_.insert_mpr_addr(nb_tuple->nb_main_addr());
    }

    // 2. Calculate D(y), where y is a member of N, for all nodes in N.
    // We will do this later.

    // 3. Add to the MPR set those nodes in N, which are the *only*
    // nodes to provide reachability to a node in N2. Remove the
    // nodes from N2 which are now covered by a node in the MPR set.
    mprset_t foundset;
    std::set<nsaddr_t> deleted_addrs;
    for (nb2hopset_t::iterator it = N2.begin(); it != N2.end();)
    {
        increment = true;
        OLSR_nb2hop_tuple* nb2hop_tuple1 = *it;

        mprset_t::iterator pos = foundset.find(nb2hop_tuple1->nb2hop_addr());
        if (pos != foundset.end())
        {
            it++;
            continue;
        }

        bool found = false;
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

        for (nb2hopset_t::iterator it2 = it + 1; it2 != N2.end(); it2++)
        {
            OLSR_nb2hop_tuple* nb2hop_tuple2 = *it2;
            if (nb2hop_tuple1->nb2hop_addr() == nb2hop_tuple2->nb2hop_addr())
            {
                foundset.insert(nb2hop_tuple1->nb2hop_addr());
                found = true;
                break;
            }
        }

        if (!found)
        {
            state_.insert_mpr_addr(nb2hop_tuple1->nb_main_addr());

            for (nb2hopset_t::iterator it2 = it + 1; it2 != N2.end();)
            {
                OLSR_nb2hop_tuple* nb2hop_tuple2 = *it2;
                if (nb2hop_tuple1->nb_main_addr() == nb2hop_tuple2->nb_main_addr())
                {
                    deleted_addrs.insert(nb2hop_tuple2->nb2hop_addr());
                    it2 = N2.erase(it2);
                }
                else
                    it2++;

            }

            int distanceFromEnd = std::distance(it, N2.end());
            int distance = std::distance(N2.begin(), it);
            int i = 0;
            for (nb2hopset_t::iterator it2 = N2.begin(); i < distance; i++) // check now the first section
            {

                OLSR_nb2hop_tuple* nb2hop_tuple2 = *it2;
                if (nb2hop_tuple1->nb_main_addr() == nb2hop_tuple2->nb_main_addr())
                {
                    deleted_addrs.insert(nb2hop_tuple2->nb2hop_addr());
                    it2 = N2.erase(it2);
                }
                else
                    it2++;

            }
            it = N2.end() - distanceFromEnd; // the standard doesn't guarantee that the iterator is valid if we have delete something in the vector, reload the iterator.

            it = N2.erase(it);
            increment = false;
        }

        for (std::set<nsaddr_t>::iterator it2 = deleted_addrs.begin();
                it2 != deleted_addrs.end();
                it2++)
        {
            for (nb2hopset_t::iterator it3 = N2.begin();
                    it3 != N2.end();)
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

    // 4. While there exist nodes in N2 which are not covered by at
    // least one node in the MPR set:
    while (N2.begin() != N2.end())
    {
        // 4.1. For each node in N, calculate the reachability, i.e., the
        // number of nodes in N2 which are not yet covered by at
        // least one node in the MPR set, and which are reachable
        // through this 1-hop neighbor
        std::map<int, std::vector<OLSR_nb_tuple*> > reachability;
        std::set<int> rs;
        for (nbset_t::iterator it = N.begin(); it != N.end(); it++)
        {
            OLSR_nb_tuple* nb_tuple = *it;
            int r = 0;
            for (nb2hopset_t::iterator it2 = N2.begin(); it2 != N2.end(); it2++)
            {
                OLSR_nb2hop_tuple* nb2hop_tuple = *it2;
                if (nb_tuple->nb_main_addr() == nb2hop_tuple->nb_main_addr())
                    r++;
            }
            rs.insert(r);
            reachability[r].push_back(nb_tuple);
        }

        // 4.2. Select as a MPR the node with highest N_willingness among
        // the nodes in N with non-zero reachability. In case of
        // multiple choice select the node which provides
        // reachability to the maximum number of nodes in N2. In
        // case of multiple nodes providing the same amount of
        // reachability, select the node as MPR whose D(y) is
        // greater. Remove the nodes from N2 which are now covered
        // by a node in the MPR set.
        OLSR_nb_tuple* max = NULL;
        int max_r = 0;
        for (std::set<int>::iterator it = rs.begin(); it != rs.end(); it++)
        {
            int r = *it;
            if (r > 0)
            {
                for (std::vector<OLSR_nb_tuple*>::iterator it2 = reachability[r].begin();
                        it2 != reachability[r].end();
                        it2++)
                {
                    OLSR_nb_tuple* nb_tuple = *it2;
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
                            if (degree(nb_tuple) > degree(max))
                            {
                                max = nb_tuple;
                                max_r = r;
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
            for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); )
            {
                OLSR_nb2hop_tuple* nb2hop_tuple = *it;
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
                OLSR_nb2hop_tuple* nb2hop_tuple = *it;
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


#else
void
OLSR::mpr_computation()
{
    // MPR computation should be done for each interface. See section 8.3.1
    // (RFC 3626) for details.
    state_.clear_mprset();

    nbset_t N; nb2hopset_t N2;
    // N is the subset of neighbors of the node, which are
    // neighbor "of the interface I"
    for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
        if ((*it)->getStatus() == OLSR_STATUS_SYM) // I think that we need this check
            N.push_back(*it);

    // N2 is the set of 2-hop neighbors reachable from "the interface
    // I", excluding:
    // (i)   the nodes only reachable by members of N with willingness WILL_NEVER
    // (ii)  the node performing the computation
    // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
    //       link to this node on some interface.
    for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++)
    {
        OLSR_nb2hop_tuple* nb2hop_tuple = *it;
        // (ii)  the node performing the computation
        if (isLocalAddress(nb2hop_tuple->nb2hop_addr()))
        {
            continue;
        }
        // excluding:
        // (i) the nodes only reachable by members of N with willingness WILL_NEVER
        bool ok = false;
        for (nbset_t::const_iterator it2 = N.begin(); it2 != N.end(); it2++)
        {
            OLSR_nb_tuple* neigh = *it2;
            if (neigh->nb_main_addr() == nb2hop_tuple->nb_main_addr())
            {
                if (neigh->willingness() == OLSR_WILL_NEVER)
                {
                    ok = false;
                    break;
                }
                else
                {
                    ok = true;
                    break;
                }
            }
        }
        if (!ok)
        {
            continue;
        }

        // excluding:
        // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
        //       link to this node on some interface.
        for (nbset_t::iterator it2 = N.begin(); it2 != N.end(); it2++)
        {
            OLSR_nb_tuple* neigh = *it2;
            if (neigh->nb_main_addr() == nb2hop_tuple->nb2hop_addr())
            {
                ok = false;
                break;
            }
        }
        if (ok)
            N2.push_back(nb2hop_tuple);
    }

    // 1. Start with an MPR set made of all members of N with
    // N_willingness equal to WILL_ALWAYS
    for (nbset_t::iterator it = N.begin(); it != N.end(); it++)
    {
        OLSR_nb_tuple* nb_tuple = *it;
        if (nb_tuple->willingness() == OLSR_WILL_ALWAYS)
        {
            state_.insert_mpr_addr(nb_tuple->nb_main_addr());
            // (not in RFC but I think is needed: remove the 2-hop
            // neighbors reachable by the MPR from N2)
            CoverTwoHopNeighbors (nb_tuple->nb_main_addr(), N2);
        }
    }

    // 2. Calculate D(y), where y is a member of N, for all nodes in N.
    // We will do this later.

    // 3. Add to the MPR set those nodes in N, which are the *only*
    // nodes to provide reachability to a node in N2. Remove the
    // nodes from N2 which are now covered by a node in the MPR set.

    std::set<nsaddr_t> coveredTwoHopNeighbors;
    for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++)
    {
        OLSR_nb2hop_tuple* twoHopNeigh = *it;
        bool onlyOne = true;
        // try to find another neighbor that can reach twoHopNeigh->twoHopNeighborAddr
        for (nb2hopset_t::const_iterator it2 = N2.begin(); it2 != N2.end(); it2++)
        {
            OLSR_nb2hop_tuple* otherTwoHopNeigh = *it2;
            if (otherTwoHopNeigh->nb2hop_addr() == twoHopNeigh->nb2hop_addr()
                    && otherTwoHopNeigh->nb_main_addr() != twoHopNeigh->nb_main_addr())
            {
                onlyOne = false;
                break;
            }
        }
        if (onlyOne)
        {
            state_.insert_mpr_addr(twoHopNeigh->nb_main_addr());

            // take note of all the 2-hop neighbors reachable by the newly elected MPR
            for (nb2hopset_t::const_iterator it2 = N2.begin(); it2 != N2.end(); it2++)
            {
                OLSR_nb2hop_tuple* otherTwoHopNeigh = *it2;
                if (otherTwoHopNeigh->nb_main_addr() == twoHopNeigh->nb_main_addr())
                {
                    coveredTwoHopNeighbors.insert(otherTwoHopNeigh->nb2hop_addr());
                }
            }
        }
    }
    // Remove the nodes from N2 which are now covered by a node in the MPR set.
    for (nb2hopset_t::iterator it = N2.begin(); it != N2.end();)
    {
        OLSR_nb2hop_tuple* twoHopNeigh = *it;
        if (coveredTwoHopNeighbors.find(twoHopNeigh->nb2hop_addr()) != coveredTwoHopNeighbors.end())
        {
            // This works correctly only because it is known that twoHopNeigh is reachable by exactly one neighbor,
            // so only one record in N2 exists for each of them. This record is erased here.
            it = N2.erase(it);
        }
        else
        {
            it++;
        }
    }
    // 4. While there exist nodes in N2 which are not covered by at
    // least one node in the MPR set:

    while (N2.begin() != N2.end())
    {

        // 4.1. For each node in N, calculate the reachability, i.e., the
        // number of nodes in N2 which are not yet covered by at
        // least one node in the MPR set, and which are reachable
        // through this 1-hop neighbor
        std::map<int, std::vector<OLSR_nb_tuple*> > reachability;
        std::set<int> rs;
        for (nbset_t::iterator it = N.begin(); it != N.end(); it++)
        {
            OLSR_nb_tuple* nb_tuple = *it;
            int r = 0;
            for (nb2hopset_t::iterator it2 = N2.begin(); it2 != N2.end(); it2++)
            {
                OLSR_nb2hop_tuple* nb2hop_tuple = *it2;

                if (nb_tuple->nb_main_addr() == nb2hop_tuple->nb_main_addr())
                    r++;
            }
            rs.insert(r);
            reachability[r].push_back(nb_tuple);
        }

        // 4.2. Select as a MPR the node with highest N_willingness among
        // the nodes in N with non-zero reachability. In case of
        // multiple choice select the node which provides
        // reachability to the maximum number of nodes in N2. In
        // case of multiple nodes providing the same amount of
        // reachability, select the node as MPR whose D(y) is
        // greater. Remove the nodes from N2 which are now covered
        // by a node in the MPR set.
        OLSR_nb_tuple *max = NULL;
        int max_r = 0;
        for (std::set<int>::iterator it = rs.begin(); it != rs.end(); it++)
        {
            int r = *it;
            if (r == 0)
            {
                continue;
            }
            for (std::vector<OLSR_nb_tuple *>::iterator it2 = reachability[r].begin();
                    it2 != reachability[r].end(); it2++)
            {
                OLSR_nb_tuple *nb_tuple = *it2;
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
                        if (degree(nb_tuple) > degree(max))
                        {
                            max = nb_tuple;
                            max_r = r;
                        }
                    }
                }
            }
        }
        if (max != NULL)
        {
            state_.insert_mpr_addr(max->nb_main_addr());
            CoverTwoHopNeighbors(max->nb_main_addr(), N2);
            EV_DETAIL << N2.size () << " 2-hop neighbors left to cover! \n";
        }
    }
}
#endif
///
/// \brief Creates the routing table of the node following RFC 3626 hints.
///
void
OLSR::rtable_computation()
{
    nsaddr_t netmask(IPv4Address::ALLONES_ADDRESS);
    // 1. All the entries from the routing table are removed.
    //

    if (par("DelOnlyRtEntriesInrtable_").boolValue())
    {
        for (rtable_t::const_iterator itRtTable = rtable_.getInternalTable()->begin();itRtTable != rtable_.getInternalTable()->begin();++itRtTable)
        {
            nsaddr_t addr = itRtTable->first;
            omnet_chg_rte(addr, addr,netmask,1, true, addr);
        }
    }
    else
        omnet_clean_rte(); // clean IP tables

    rtable_.clear();


    // 2. The new routing entries are added starting with the
    // symmetric neighbors (h=1) as the destination nodes.

    for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
    {
        OLSR_nb_tuple* nb_tuple = *it;
        if (nb_tuple->getStatus() == OLSR_STATUS_SYM)
        {
            bool nb_main_addr = false;
            OLSR_link_tuple* lt = NULL;
            for (linkset_t::iterator it2 = linkset().begin(); it2 != linkset().end(); it2++)
            {
                OLSR_link_tuple* link_tuple = *it2;
                if (get_main_addr(link_tuple->nb_iface_addr()) == nb_tuple->nb_main_addr() && link_tuple->time() >= CURRENT_TIME)
                {
                    lt = link_tuple;
                    rtable_.add_entry(link_tuple->nb_iface_addr(),
                                      link_tuple->nb_iface_addr(),
                                      link_tuple->local_iface_addr(),
                                      1, link_tuple->local_iface_index());
                    if (!useIndex)
                        omnet_chg_rte(link_tuple->nb_iface_addr(),
                                       link_tuple->nb_iface_addr(),
                                       netmask,
                                       1, false, link_tuple->local_iface_addr());
                    else
                        omnet_chg_rte(link_tuple->nb_iface_addr(),
                                       link_tuple->nb_iface_addr(),
                                       netmask,
                                       1, false, link_tuple->local_iface_index());

                    if (link_tuple->nb_iface_addr() == nb_tuple->nb_main_addr())
                        nb_main_addr = true;
                }
            }
            if (!nb_main_addr && lt != NULL)
            {
                rtable_.add_entry(nb_tuple->nb_main_addr(),
                                  lt->nb_iface_addr(),
                                  lt->local_iface_addr(),
                                  1, lt->local_iface_index());

                if (!useIndex)
                    omnet_chg_rte(nb_tuple->nb_main_addr(),
                                   lt->nb_iface_addr(),
                                   netmask,// Default mask
                                   1, false, lt->local_iface_addr());

                else
                    omnet_chg_rte(nb_tuple->nb_main_addr(),
                                   lt->nb_iface_addr(),
                                   netmask,// Default mask
                                   1, false, lt->local_iface_index());
            }
        }
    }

    // N2 is the set of 2-hop neighbors reachable from this node, excluding:
    // (i)   the nodes only reachable by members of N with willingness WILL_NEVER
    // (ii)  the node performing the computation
    // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
    //       link to this node on some interface.
    for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++)
    {
        OLSR_nb2hop_tuple* nb2hop_tuple = *it;
        bool ok = true;
        OLSR_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_addr());
        if (nb_tuple == NULL)
            ok = false;
        else
        {
            nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_addr(), OLSR_WILL_NEVER);
            if (nb_tuple != NULL)
                ok = false;
            else
            {
                nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb2hop_addr());
                if (nb_tuple != NULL)
                    ok = false;
            }
        }

        // 3. For each node in N2 create a new entry in the routing table
        if (ok)
        {
            OLSR_rt_entry* entry = rtable_.lookup(nb2hop_tuple->nb_main_addr());
            assert(entry != NULL);
            rtable_.add_entry(nb2hop_tuple->nb2hop_addr(),
                              entry->next_addr(),
                              entry->iface_addr(),
                              2, entry->local_iface_index());
            if (!useIndex)
                omnet_chg_rte(nb2hop_tuple->nb2hop_addr(),
                               entry->next_addr(),
                               netmask,
                               2, false, entry->iface_addr());

            else
                omnet_chg_rte(nb2hop_tuple->nb2hop_addr(),
                               entry->next_addr(),
                               netmask,
                               2, false, entry->local_iface_index());

        }
    }

    for (uint32_t h = 2;; h++)
    {
        bool added = false;

        // 4.1. For each topology entry in the topology table, if its
        // T_dest_addr does not correspond to R_dest_addr of any
        // route entry in the routing table AND its T_last_addr
        // corresponds to R_dest_addr of a route entry whose R_dist
        // is equal to h, then a new route entry MUST be recorded in
        // the routing table (if it does not already exist)
        for (topologyset_t::iterator it = topologyset().begin();
                it != topologyset().end();
                it++)
        {
            OLSR_topology_tuple* topology_tuple = *it;
            OLSR_rt_entry* entry1 = rtable_.lookup(topology_tuple->dest_addr());
            OLSR_rt_entry* entry2 = rtable_.lookup(topology_tuple->last_addr());
            if (entry1 == NULL && entry2 != NULL && entry2->dist() == h)
            {
                rtable_.add_entry(topology_tuple->dest_addr(),
                                  entry2->next_addr(),
                                  entry2->iface_addr(),
                                  h+1, entry2->local_iface_index(), entry2);

                if (!useIndex)
                    omnet_chg_rte(topology_tuple->dest_addr(),
                                   entry2->next_addr(),
                                   netmask,
                                   h+1, false, entry2->iface_addr());

                else
                    omnet_chg_rte(topology_tuple->dest_addr(),
                                   entry2->next_addr(),
                                   netmask,
                                   h+1, false, entry2->local_iface_index());

                added = true;
            }
        }

        // 5. For each entry in the multiple interface association base
        // where there exists a routing entry such that:
        //  R_dest_addr  == I_main_addr  (of the multiple interface association entry)
        // AND there is no routing entry such that:
        //  R_dest_addr  == I_iface_addr
        // then a route entry is created in the routing table
        for (ifaceassocset_t::iterator it = ifaceassocset().begin();
                it != ifaceassocset().end();
                it++)
        {
            OLSR_iface_assoc_tuple* tuple = *it;
            OLSR_rt_entry* entry1 = rtable_.lookup(tuple->main_addr());
            OLSR_rt_entry* entry2 = rtable_.lookup(tuple->iface_addr());
            if (entry1 != NULL && entry2 == NULL)
            {
                rtable_.add_entry(tuple->iface_addr(),
                                  entry1->next_addr(),
                                  entry1->iface_addr(),
                                  entry1->dist(), entry1->local_iface_index(), entry1);

                if (!useIndex)
                    omnet_chg_rte(tuple->iface_addr(),
                                   entry1->next_addr(),
                                   netmask,
                                   entry1->dist(), false, entry1->iface_addr());

                else
                    omnet_chg_rte(tuple->iface_addr(),
                                   entry1->next_addr(),
                                   netmask,
                                   entry1->dist(), false, entry1->local_iface_index());
                added = true;
            }
        }

        if (!added)
            break;
    }
    setTopologyChanged(false);
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
bool
OLSR::process_hello(OLSR_msg& msg, const nsaddr_t &receiver_iface, const nsaddr_t &sender_iface, const int &index)
{
    assert(msg.msg_type() == OLSR_HELLO_MSG);

    link_sensing(msg, receiver_iface, sender_iface, index);
    populate_nbset(msg);
    populate_nb2hopset(msg);
    mpr_computation();
    populate_mprselset(msg);
    return false;
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
bool
OLSR::process_tc(OLSR_msg& msg, const nsaddr_t &sender_iface, const int &index)
{
    assert(msg.msg_type() == OLSR_TC_MSG);
    double now = CURRENT_TIME;
    OLSR_tc& tc = msg.tc();

    // 1. If the sender interface of this message is not in the symmetric
    // 1-hop neighborhood of this node, the message MUST be discarded.
    OLSR_link_tuple* link_tuple = state_.find_sym_link_tuple(sender_iface, now);
    if (link_tuple == NULL)
        return false;

    // 2. If there exist some tuple in the topology set where:
    //  T_last_addr == originator address AND
    //  T_seq       >  ANSN,
    // then further processing of this TC message MUST NOT be
    // performed.
    OLSR_topology_tuple* topology_tuple =
        state_.find_newer_topology_tuple(msg.orig_addr(), tc.ansn());
    if (topology_tuple != NULL)
        return false;

    // 3. All tuples in the topology set where:
    //  T_last_addr == originator address AND
    //  T_seq       <  ANSN
    // MUST be removed from the topology set.
    state_.erase_older_topology_tuples(msg.orig_addr(), tc.ansn());

    // 4. For each of the advertised neighbor main address received in
    // the TC message:
    for (int i = 0; i < tc.count; i++)
    {
        assert(i >= 0 && i < OLSR_MAX_ADDRS);
        nsaddr_t addr = tc.nb_main_addr(i);
        // 4.1. If there exist some tuple in the topology set where:
        //  T_dest_addr == advertised neighbor main address, AND
        //  T_last_addr == originator address,
        // then the holding time of that tuple MUST be set to:
        //  T_time      =  current time + validity time.
        OLSR_topology_tuple* topology_tuple =
            state_.find_topology_tuple(addr, msg.orig_addr());
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
            OLSR_topology_tuple* topology_tuple = new OLSR_topology_tuple;
            topology_tuple->dest_addr() = addr;
            topology_tuple->last_addr() = msg.orig_addr();
            topology_tuple->seq() = tc.ansn();
            topology_tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
            topology_tuple->local_iface_index() = index;
            add_topology_tuple(topology_tuple);
            // Schedules topology tuple deletion
            OLSR_TopologyTupleTimer* topology_timer =
                new OLSR_TopologyTupleTimer(this, topology_tuple);
            topology_timer->resched(DELAY(topology_tuple->time()));
        }
    }
    return false;
}

///
/// \brief Processes a MID message following RFC 3626 specification.
///
/// The Interface Association Set is updated (if needed) with the information
/// of the received MID message.
///
/// \param msg the %OLSR message which contains the MID message.
/// \param sender_iface the address of the interface where the message was sent from.
///
void
OLSR::process_mid(OLSR_msg& msg, const nsaddr_t &sender_iface, const int &index)
{
    assert(msg.msg_type() == OLSR_MID_MSG);
    double now = CURRENT_TIME;
    OLSR_mid& mid = msg.mid();

    // 1. If the sender interface of this message is not in the symmetric
    // 1-hop neighborhood of this node, the message MUST be discarded.
    OLSR_link_tuple* link_tuple = state_.find_sym_link_tuple(sender_iface, now);
    if (link_tuple == NULL)
        return;

    // 2. For each interface address listed in the MID message
    for (int i = 0; i < mid.count; i++)
    {
        bool updated = false;
        for (ifaceassocset_t::iterator it = ifaceassocset().begin();
                it != ifaceassocset().end();
                it++)
        {
            OLSR_iface_assoc_tuple* tuple = *it;
            if (tuple->iface_addr() == mid.iface_addr(i)
                    && tuple->main_addr() == msg.orig_addr())
            {
                tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
                updated = true;
            }
        }
        if (!updated)
        {
            OLSR_iface_assoc_tuple* tuple = new OLSR_iface_assoc_tuple;
            tuple->iface_addr() = msg.mid().iface_addr(i);
            tuple->main_addr() = msg.orig_addr();
            tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
            tuple->local_iface_index() = index;
            add_ifaceassoc_tuple(tuple);
            // Schedules iface association tuple deletion
            OLSR_IfaceAssocTupleTimer* ifaceassoc_timer =
                new OLSR_IfaceAssocTupleTimer(this, tuple);
            ifaceassoc_timer->resched(DELAY(tuple->time()));
        }
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
void
OLSR::forward_default(OLSR_msg& msg, OLSR_dup_tuple* dup_tuple, const nsaddr_t &local_iface, const nsaddr_t &src_addr)
{
    double now = CURRENT_TIME;

    // If the sender interface address is not in the symmetric
    // 1-hop neighborhood the message must not be forwarded


    OLSR_link_tuple* link_tuple = state_.find_sym_link_tuple(src_addr, now);
    if (link_tuple == NULL)
        return;

    // If the message has already been considered for forwarding,
    // it must not be retransmitted again
    if (dup_tuple != NULL && dup_tuple->retransmitted())
    {
        debug("%f: Node %s does not forward a message received"
              " from %s because it is duplicated\n",
              CURRENT_TIME,
              getNodeId(ra_addr()),
              getNodeId(dup_tuple->getAddr()));
        return;
    }

    // If the sender interface address is an interface address
    // of a MPR selector of this node and ttl is greater than 1,
    // the message must be retransmitted
    bool retransmitted = false;
    if (msg.ttl() > 1)
    {
        OLSR_mprsel_tuple* mprsel_tuple =
            state_.find_mprsel_tuple(get_main_addr(src_addr));
        if (mprsel_tuple != NULL)
        {
            OLSR_msg& new_msg = msg;
            new_msg.ttl()--;
            new_msg.hop_count()++;
            // We have to introduce a random delay to avoid
            // synchronization with neighbors.
            enque_msg(new_msg, JITTER);
            retransmitted = true;
        }
    }

    // Update duplicate tuple...
    if (dup_tuple != NULL)
    {
        dup_tuple->time() = now + OLSR_DUP_HOLD_TIME;
        dup_tuple->retransmitted() = retransmitted;
        dup_tuple->iface_list().push_back(local_iface);
    }
    // ...or create a new one
    else
    {
        OLSR_dup_tuple* new_dup = new OLSR_dup_tuple;
        new_dup->getAddr() = msg.orig_addr();
        new_dup->seq_num() = msg.msg_seq_num();
        new_dup->time() = now + OLSR_DUP_HOLD_TIME;
        new_dup->retransmitted() = retransmitted;
        new_dup->iface_list().push_back(local_iface);
        add_dup_tuple(new_dup);
        // Schedules dup tuple deletion
        OLSR_DupTupleTimer* dup_timer =
            new OLSR_DupTupleTimer(this, new_dup);
        dup_timer->resched(DELAY(new_dup->time()));
    }
}

///
/// \brief Forwards a data packet to the appropiate next hop indicated by the routing table.
///
/// \param p the packet which must be forwarded.
///
#if 0
void
OLSR::forward_data(cMessage* p, nsaddr_t addr)
{
    struct hdr_cmn* ch = HDR_CMN(p);
    struct hdr_ip* ih = HDR_IP(p);

    if (ch->direction() == hdr_cmn::UP &&
            ((uint32_t)ih->daddr() == IP_BROADCAST || ih->daddr() == ra_addr()))
    {
        dmux_->recv(p, 0);
        return;
    }
    else
    {
        if ((uint32_t)ih->daddr() != IP_BROADCAST)
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

            }
        }
        sendToIp();
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
void
OLSR::enque_msg(OLSR_msg& msg, double delay)
{
    assert(delay >= 0);

    msgs_.push_back(msg);
    OLSR_MsgTimer* timer = new OLSR_MsgTimer(this);
    timer->resched(delay);
}

///
/// \brief Creates as many %OLSR packets as needed in order to send all buffered
/// %OLSR messages.
///
/// Maximum number of messages which can be contained in an %OLSR packet is
/// dictated by OLSR_MAX_MSGS constant.
///
void
OLSR::send_pkt()
{
    int num_msgs = msgs_.size();
    if (num_msgs == 0)
        return;

    // Calculates the number of needed packets
    int num_pkts = (num_msgs%OLSR_MAX_MSGS == 0) ? num_msgs/OLSR_MAX_MSGS :
                   (num_msgs/OLSR_MAX_MSGS + 1);

    L3Address destAdd;
    if (!this->isInMacLayer())
        destAdd.set(IPv4Address::ALLONES_ADDRESS);
    else
        destAdd.set(MACAddress::BROADCAST_ADDRESS);

    for (int i = 0; i < num_pkts; i++)
    {
        OLSR_pkt* op = new OLSR_pkt;
        op->setName("OLSR Pkt");

        op->setByteLength( OLSR_PKT_HDR_SIZE );
        op->setPkt_seq_num( pkt_seq());
        op->setReduceFuncionality(par("reduceFuncionality").boolValue());

        int j = 0;
        for (std::vector<OLSR_msg>::iterator it = msgs_.begin(); it != msgs_.end();)
        {
            if (j == OLSR_MAX_MSGS)
                break;

            op->setMsgArraySize(j+1);
            op->setMsg(j++, *it);
            op->setByteLength(op->getByteLength()+(*it).size());

            it = msgs_.erase(it);
        }

        sendToIp(op, RT_PORT, destAdd, RT_PORT, IP_DEF_TTL, 0.0, L3Address());
    }
}

///
/// \brief Creates a new %OLSR HELLO message which is buffered to be sent later on.
///
void
OLSR::send_hello()
{
    OLSR_msg msg;
    double now = CURRENT_TIME;
    msg.msg_type() = OLSR_HELLO_MSG;
    msg.vtime() = OLSR::seconds_to_emf(OLSR_NEIGHB_HOLD_TIME);
    msg.orig_addr() = ra_addr();
    msg.ttl() = 1;
    msg.hop_count() = 0;
    msg.msg_seq_num() = msg_seq();

    msg.hello().reserved() = 0;
    msg.hello().htime() = OLSR::seconds_to_emf(SIMTIME_DBL(hello_ival()));
    msg.hello().willingness() = willingness();
    msg.hello().count = 0;

    std::map<uint8_t, int> linkcodes_count;
    for (linkset_t::iterator it = linkset().begin(); it != linkset().end(); it++)
    {
        OLSR_link_tuple* link_tuple = *it;
        if (get_main_addr(link_tuple->local_iface_addr()) == ra_addr() && link_tuple->time() >= now)
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
                    if (nb_tuple->nb_main_addr() == get_main_addr(link_tuple->nb_iface_addr()))
                    {
                        if (nb_tuple->getStatus() == OLSR_STATUS_SYM)
                            nb_type = OLSR_SYM_NEIGH;
                        else if (nb_tuple->getStatus() == OLSR_STATUS_NOT_SYM)
                            nb_type = OLSR_NOT_NEIGH;
                        else
                        {
                            throw cRuntimeError("There is a neighbor tuple with an unknown status!");
                        }
                        ok = true;
                        break;
                    }
                }
                if (!ok)
                {
                    EV_INFO << "I don't know the neighbor " << get_main_addr(link_tuple->nb_iface_addr()) << "!!! \n";
                    continue;
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
                msg.hello().count++;
            }
            else
                count = (*pos).second;

            int i = msg.hello().hello_msg(count).count;
            assert(count >= 0 && count < OLSR_MAX_HELLOS);
            assert(i >= 0 && i < OLSR_MAX_ADDRS);

            msg.hello().hello_msg(count).nb_iface_addr(i) =
                link_tuple->nb_iface_addr();
            msg.hello().hello_msg(count).count++;
            msg.hello().hello_msg(count).link_msg_size() =
                msg.hello().hello_msg(count).size();
        }
    }

    msg.msg_size() = msg.size();

    enque_msg(msg, JITTER);
}

///
/// \brief Creates a new %OLSR TC message which is buffered to be sent later on.
///
void
OLSR::send_tc()
{
    OLSR_msg msg;
    msg.msg_type() = OLSR_TC_MSG;
    msg.vtime() = OLSR::seconds_to_emf(OLSR_TOP_HOLD_TIME);
    msg.orig_addr() = ra_addr();
    msg.ttl() = 255;
    msg.hop_count() = 0;
    msg.msg_seq_num() = msg_seq();

    msg.tc().ansn() = ansn_;
    msg.tc().reserved() = 0;
    msg.tc().count = 0;

    for (mprselset_t::iterator it = mprselset().begin(); it != mprselset().end(); it++)
    {
        OLSR_mprsel_tuple* mprsel_tuple = *it;
        int count = msg.tc().count;

        assert(count >= 0 && count < OLSR_MAX_ADDRS);
        msg.tc().nb_main_addr(count) = mprsel_tuple->main_addr();
        msg.tc().count++;
    }

    msg.msg_size() = msg.size();

    enque_msg(msg, JITTER);
}

///
/// \brief Creates a new %OLSR MID message which is buffered to be sent later on.
/// \warning This message is never invoked because there is no support for multiple interfaces.
///
void
OLSR::send_mid()
{
    if (getNumWlanInterfaces() <= 1 && optimizedMid)
        return;
    OLSR_msg msg;
    msg.msg_type() = OLSR_MID_MSG;
    msg.vtime() = OLSR::seconds_to_emf(OLSR_MID_HOLD_TIME);
    msg.orig_addr() = ra_addr();
    msg.ttl() = 255;
    msg.hop_count() = 0;
    msg.msg_seq_num() = msg_seq();

    msg.mid().count = 0;
    for (int i = 0; i< getNumWlanInterfaces(); i++)
    {
        int index = getWlanInterfaceIndex(i);
        nsaddr_t addr = getIfaceAddressFromIndex(index);
        msg.mid().setIface_addr(i,addr);
        msg.mid().count++;
    }
    //foreach iface in this_node do
    //  msg.mid().iface_addr(i) = iface
    //  msg.mid().count++
    //done

    msg.msg_size() = msg.size();

    enque_msg(msg, JITTER);
}

///
/// \brief  Updates Link Set according to a new received HELLO message (following RFC 3626
///     specification). Neighbor Set is also updated if needed.
///
/// \param msg the OLSR message which contains the HELLO message.
/// \param receiver_iface the address of the interface where the message was received from.
/// \param sender_iface the address of the interface where the message was sent from.
///
bool
OLSR::link_sensing(OLSR_msg& msg, const nsaddr_t &receiver_iface, const nsaddr_t &sender_iface, const int &index)
{
    OLSR_hello& hello = msg.hello();
    double now = CURRENT_TIME;
    bool updated = false;
    bool created = false;

    OLSR_link_tuple* link_tuple = state_.find_link_tuple(sender_iface);
    if (link_tuple == NULL)
    {
        // We have to create a new tuple
        link_tuple = new OLSR_link_tuple;
        link_tuple->nb_iface_addr() = sender_iface;
        link_tuple->local_iface_addr() = receiver_iface;
        link_tuple->local_iface_index() = index;
        link_tuple->sym_time() = now - 1;
        link_tuple->lost_time() = 0.0;
        link_tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
        add_link_tuple(link_tuple, hello.willingness());
        created = true;
    }
    else
        updated = true;

    link_tuple->asym_time() = now + OLSR::emf_to_seconds(msg.vtime());
    assert(hello.count >= 0 && hello.count <= OLSR_MAX_HELLOS);
    for (int i = 0; i < hello.count; i++)
    {
        OLSR_hello_msg& hello_msg = hello.hello_msg(i);
        int lt = hello_msg.link_code() & 0x03;
        int nt = hello_msg.link_code() >> 2;

        // We must not process invalid advertised links
        if ((lt == OLSR_SYM_LINK && nt == OLSR_NOT_NEIGH) ||
                (nt != OLSR_SYM_NEIGH && nt != OLSR_MPR_NEIGH
                 && nt != OLSR_NOT_NEIGH))
            continue;

        assert(hello_msg.count >= 0 && hello_msg.count <= OLSR_MAX_ADDRS);
        for (int j = 0; j < hello_msg.count; j++)
        {
            if (hello_msg.nb_iface_addr(j) == receiver_iface)
            {
                if (lt == OLSR_LOST_LINK)
                {
                    link_tuple->sym_time() = now - 1;
                    updated = true;
                }
                else if (lt == OLSR_SYM_LINK || lt == OLSR_ASYM_LINK)
                {
                    link_tuple->sym_time() =
                        now + OLSR::emf_to_seconds(msg.vtime());
                    link_tuple->time() =
                        link_tuple->sym_time() + OLSR_NEIGHB_HOLD_TIME;
                    link_tuple->lost_time() = 0.0;
                    updated = true;
                }
                break;
            }
        }

    }
    link_tuple->time() = MAX(link_tuple->time(), link_tuple->asym_time());

    if (updated)
        updated_link_tuple(link_tuple, hello.willingness());

    // Schedules link tuple deletion
    if (created && link_tuple != NULL)
    {
        OLSR_LinkTupleTimer* link_timer =
            new OLSR_LinkTupleTimer(this, link_tuple);
        link_timer->resched(DELAY(MIN(link_tuple->time(), link_tuple->sym_time())));
    }
    return false;
}

///
/// \brief  Updates the Neighbor Set according to the information contained in a new received
///     HELLO message (following RFC 3626).
///
/// \param msg the %OLSR message which contains the HELLO message.
///
bool
OLSR::populate_nbset(OLSR_msg& msg)
{
    OLSR_hello& hello = msg.hello();

    OLSR_nb_tuple* nb_tuple = state_.find_nb_tuple(msg.orig_addr());
    if (nb_tuple != NULL)
        nb_tuple->willingness() = hello.willingness();
    return false;
}

///
/// \brief  Updates the 2-hop Neighbor Set according to the information contained in a new
///     received HELLO message (following RFC 3626).
///
/// \param msg the %OLSR message which contains the HELLO message.
///
bool
OLSR::populate_nb2hopset(OLSR_msg& msg)
{
    double now = CURRENT_TIME;
    OLSR_hello& hello = msg.hello();

    for (linkset_t::iterator it_lt = linkset().begin(); it_lt != linkset().end(); it_lt++)
    {
        OLSR_link_tuple* link_tuple = *it_lt;
        if (get_main_addr(link_tuple->nb_iface_addr()) == msg.orig_addr())
        {
            if (link_tuple->sym_time() >= now)
            {
                assert(hello.count >= 0 && hello.count <= OLSR_MAX_HELLOS);
                for (int i = 0; i < hello.count; i++)
                {
                    OLSR_hello_msg& hello_msg = hello.hello_msg(i);
                    int nt = hello_msg.link_code() >> 2;
                    assert(hello_msg.count >= 0 &&
                           hello_msg.count <= OLSR_MAX_ADDRS);

                    for (int j = 0; j < hello_msg.count; j++)
                    {
                        nsaddr_t nb2hop_addr = get_main_addr(hello_msg.nb_iface_addr(j));
                        if (nt == OLSR_SYM_NEIGH || nt == OLSR_MPR_NEIGH)
                        {
                            // if the main address of the 2-hop
                            // neighbor address = main address of
                            // the receiving node: silently
                            // discard the 2-hop neighbor address
                            if (nb2hop_addr != ra_addr())
                            {
                                // Otherwise, a 2-hop tuple is created
                                OLSR_nb2hop_tuple* nb2hop_tuple =
                                    state_.find_nb2hop_tuple(msg.orig_addr(), nb2hop_addr);
                                if (nb2hop_tuple == NULL)
                                {
                                    nb2hop_tuple =
                                        new OLSR_nb2hop_tuple;
                                    nb2hop_tuple->nb_main_addr() =
                                        msg.orig_addr();
                                    nb2hop_tuple->nb2hop_addr() =
                                        nb2hop_addr;
                                    add_nb2hop_tuple(nb2hop_tuple);
                                    nb2hop_tuple->time() =
                                        now + OLSR::emf_to_seconds(msg.vtime());
                                    // Schedules nb2hop tuple
                                    // deletion
                                    OLSR_Nb2hopTupleTimer* nb2hop_timer =
                                        new OLSR_Nb2hopTupleTimer(this, nb2hop_tuple);
                                    nb2hop_timer->resched(DELAY(nb2hop_tuple->time()));
                                }
                                else
                                {
                                    nb2hop_tuple->time() =
                                        now + OLSR::emf_to_seconds(msg.vtime());
                                }

                            }
                        }
                        else if (nt == OLSR_NOT_NEIGH)
                        {
                            // For each 2-hop node listed in the HELLO
                            // message with Neighbor Type equal to
                            // NOT_NEIGH all 2-hop tuples where:
                            // N_neighbor_main_addr == Originator
                            // Address AND N_2hop_addr  == main address
                            // of the 2-hop neighbor are deleted.
                            state_.erase_nb2hop_tuples(msg.orig_addr(),
                                                       nb2hop_addr);
                        }
                    }
                }
            }
        }
    }
    return false;
}

///
/// \brief  Updates the MPR Selector Set according to the information contained in a new
///     received HELLO message (following RFC 3626).
///
/// \param msg the %OLSR message which contains the HELLO message.
///
void
OLSR::populate_mprselset(OLSR_msg& msg)
{
    double now = CURRENT_TIME;
    OLSR_hello& hello = msg.hello();

    assert(hello.count >= 0 && hello.count <= OLSR_MAX_HELLOS);
    for (int i = 0; i < hello.count; i++)
    {
        OLSR_hello_msg& hello_msg = hello.hello_msg(i);
        int nt = hello_msg.link_code() >> 2;
        if (nt == OLSR_MPR_NEIGH)
        {
            assert(hello_msg.count >= 0 && hello_msg.count <= OLSR_MAX_ADDRS);
            for (int j = 0; j < hello_msg.count; j++)
            {
                if (get_main_addr(hello_msg.nb_iface_addr(j)) == ra_addr())
                {
                    // We must create a new entry into the mpr selector set
                    OLSR_mprsel_tuple* mprsel_tuple =
                        state_.find_mprsel_tuple(msg.orig_addr());
                    if (mprsel_tuple == NULL)
                    {
                        mprsel_tuple = new OLSR_mprsel_tuple;
                        mprsel_tuple->main_addr() = msg.orig_addr();
                        mprsel_tuple->time() =
                            now + OLSR::emf_to_seconds(msg.vtime());
                        add_mprsel_tuple(mprsel_tuple);
                        // Schedules mpr selector tuple deletion
                        OLSR_MprSelTupleTimer* mprsel_timer =
                            new OLSR_MprSelTupleTimer(this, mprsel_tuple);
                        mprsel_timer->resched(DELAY(mprsel_tuple->time()));
                    }
                    else
                        mprsel_tuple->time() =
                            now + OLSR::emf_to_seconds(msg.vtime());
                }
            }
        }
    }
}

///
/// \brief  Drops a given packet because it couldn't be delivered to the corresponding
///     destination by the MAC layer. This may cause a neighbor loss, and appropiate
///     actions are then taken.
///
/// \param p the packet which couldn't be delivered by the MAC layer.
///
void
OLSR::mac_failed(IPv4Datagram* p)
{
    double now = CURRENT_TIME;


    nsaddr_t dest_addr = p->getDestinationAddress();

    EV_WARN <<"Node " << OLSR::node_id(ra_addr()) << "MAC Layer detects a breakage on link to "  <<
    OLSR::node_id(dest_addr);

    if (dest_addr == L3Address(IPv4Address(IP_BROADCAST)))
    {
        return;
    }

    OLSR_rt_entry*  entry = rtable_.lookup(dest_addr);
    if (entry)
    {
        OLSR_link_tuple* link_tuple = state_.find_link_tuple(entry->next_addr());
        if (link_tuple != NULL)
        {
            link_tuple->lost_time() = now + OLSR_NEIGHB_HOLD_TIME;
            link_tuple->time() = now + OLSR_NEIGHB_HOLD_TIME;
            nb_loss(link_tuple);
        }
    }
    deleteIpEntry(dest_addr);
}

///
/// \brief Schedule the timer used for sending HELLO messages.
///
void
OLSR::set_hello_timer()
{
    hello_timer_.resched((double)(SIMTIME_DBL(hello_ival()) - JITTER));
}

///
/// \brief Schedule the timer used for sending TC messages.
///
void
OLSR::set_tc_timer()
{
    tc_timer_.resched((double)(SIMTIME_DBL(tc_ival()) - JITTER));
}

///
/// \brief Schedule the timer used for sending MID messages.
///
void
OLSR::set_mid_timer()
{
    mid_timer_.resched((double)(SIMTIME_DBL(mid_ival()) - JITTER));
}

///
/// \brief Performs all actions needed when a neighbor loss occurs.
///
/// Neighbor Set, 2-hop Neighbor Set, MPR Set and MPR Selector Set are updated.
///
/// \param tuple link tuple with the information of the link to the neighbor which has been lost.
///
void
OLSR::nb_loss(OLSR_link_tuple* tuple)
{
    debug("%f: Node %s detects neighbor %s loss\n",
          CURRENT_TIME,
          getNodeId(ra_addr()),
          getNodeId(tuple->nb_iface_addr()));

    updated_link_tuple(tuple,OLSR_WILL_DEFAULT);
    state_.erase_nb2hop_tuples(get_main_addr(tuple->nb_iface_addr()));
    state_.erase_mprsel_tuples(get_main_addr(tuple->nb_iface_addr()));

    mpr_computation();
    rtable_computation();
}

///
/// \brief Adds a duplicate tuple to the Duplicate Set.
///
/// \param tuple the duplicate tuple to be added.
///
void
OLSR::add_dup_tuple(OLSR_dup_tuple* tuple)
{
    /*debug("%f: Node %d adds dup tuple: addr = %d seq_num = %d\n",
        CURRENT_TIME,
        OLSR::node_id(ra_addr()),
        OLSR::node_id(tuple->getAddr()),
        tuple->seq_num());*/

    state_.insert_dup_tuple(tuple);
}

///
/// \brief Removes a duplicate tuple from the Duplicate Set.
///
/// \param tuple the duplicate tuple to be removed.
///
void
OLSR::rm_dup_tuple(OLSR_dup_tuple* tuple)
{
    /*debug("%f: Node %d removes dup tuple: addr = %d seq_num = %d\n",
        CURRENT_TIME,
        OLSR::node_id(ra_addr()),
        OLSR::node_id(tuple->getAddr()),
        tuple->seq_num());*/

    state_.erase_dup_tuple(tuple);
}

///
/// \brief Adds a link tuple to the Link Set (and an associated neighbor tuple to the Neighbor Set).
///
/// \param tuple the link tuple to be added.
/// \param willingness willingness of the node which is going to be inserted in the Neighbor Set.
///
void
OLSR::add_link_tuple(OLSR_link_tuple* tuple, uint8_t  willingness)
{
    double now = CURRENT_TIME;

    debug("%f: Node %s adds link tuple: nb_addr = %s\n",
          now,
          getNodeId(ra_addr()),
          getNodeId(tuple->nb_iface_addr()));

    state_.insert_link_tuple(tuple);
    // Creates associated neighbor tuple
    OLSR_nb_tuple* nb_tuple = new OLSR_nb_tuple;
    nb_tuple->nb_main_addr() = get_main_addr(tuple->nb_iface_addr());
    nb_tuple->willingness() = willingness;
    if (tuple->sym_time() >= now)
        nb_tuple->getStatus() = OLSR_STATUS_SYM;
    else
        nb_tuple->getStatus() = OLSR_STATUS_NOT_SYM;
    add_nb_tuple(nb_tuple);
}

///
/// \brief Removes a link tuple from the Link Set.
///
/// \param tuple the link tuple to be removed.
///
void
OLSR::rm_link_tuple(OLSR_link_tuple* tuple)
{
    nsaddr_t nb_addr = get_main_addr(tuple->nb_iface_addr());
    double now = CURRENT_TIME;

    debug("%f: Node %s removes link tuple: nb_addr = %s\n",
          now,
          getNodeId(ra_addr()),
          getNodeId(tuple->nb_iface_addr()));
    // Prints this here cause we are not actually calling rm_nb_tuple() (efficiency stuff)
    debug("%f: Node %s removes neighbor tuple: nb_addr = %s\n",
          now,
          getNodeId(ra_addr()),
          getNodeId(nb_addr));

    state_.erase_link_tuple(tuple);

    OLSR_nb_tuple* nb_tuple = state_.find_nb_tuple(nb_addr);
    state_.erase_nb_tuple(nb_tuple);
    delete nb_tuple;
}

///
/// \brief  This function is invoked when a link tuple is updated. Its aim is to
///     also update the corresponding neighbor tuple if it is needed.
///
/// \param tuple the link tuple which has been updated.
///
void
OLSR::updated_link_tuple(OLSR_link_tuple* tuple, uint8_t willingness)
{
    double now = CURRENT_TIME;

    // Each time a link tuple changes, the associated neighbor tuple must be recomputed
    OLSR_nb_tuple* nb_tuple = find_or_add_nb(tuple, willingness);

    if (nb_tuple != NULL)
    {
        if (use_mac() && tuple->lost_time() >= now)
            nb_tuple->getStatus() = OLSR_STATUS_NOT_SYM;
        else if (tuple->sym_time() >= now)
            nb_tuple->getStatus() = OLSR_STATUS_SYM;
        else
            nb_tuple->getStatus() = OLSR_STATUS_NOT_SYM;

        debug("%f: Node %s has updated link tuple: nb_addr = %s status = %s\n", now, getNodeId(ra_addr()),
                getNodeId(tuple->nb_iface_addr()), ((nb_tuple->getStatus() == OLSR_STATUS_SYM) ? "sym" : "not_sym"));
    }
}
// Auxiliary method
// add NB based in the link tuple information if the Nb doen't exist

OLSR_nb_tuple* OLSR::find_or_add_nb(OLSR_link_tuple* tuple, uint8_t willingness)
{
    OLSR_nb_tuple* nb_tuple = state_.find_nb_tuple(get_main_addr(tuple->nb_iface_addr()));
    if (nb_tuple == NULL)
    {
        double now = CURRENT_TIME;
        state_.erase_nb_tuple(tuple->nb_iface_addr());
        // Creates associated neighbor tuple
        nb_tuple = new OLSR_nb_tuple;
        nb_tuple->nb_main_addr() = get_main_addr(tuple->nb_iface_addr());
        nb_tuple->willingness() = willingness;
        if (tuple->sym_time() >= now)
            nb_tuple->getStatus() = OLSR_STATUS_SYM;
        else
            nb_tuple->getStatus() = OLSR_STATUS_NOT_SYM;
        add_nb_tuple(nb_tuple);
        nb_tuple = state_.find_nb_tuple(get_main_addr(tuple->nb_iface_addr()));
    }
    return nb_tuple;
}

///
/// \brief Adds a neighbor tuple to the Neighbor Set.
///
/// \param tuple the neighbor tuple to be added.
///
void
OLSR::add_nb_tuple(OLSR_nb_tuple* tuple)
{
    debug("%f: Node %s adds neighbor tuple: nb_addr = %s status = %s\n",
          CURRENT_TIME,
          getNodeId(ra_addr()),
          getNodeId(tuple->nb_main_addr()),
          ((tuple->getStatus() == OLSR_STATUS_SYM) ? "sym" : "not_sym"));
    state_.erase_nb_tuple(tuple->nb_main_addr());
    state_.insert_nb_tuple(tuple);
}

///
/// \brief Removes a neighbor tuple from the Neighbor Set.
///
/// \param tuple the neighbor tuple to be removed.
///
void
OLSR::rm_nb_tuple(OLSR_nb_tuple* tuple)
{
    debug("%f: Node %s removes neighbor tuple: nb_addr = %s status = %s\n",
          CURRENT_TIME,
          getNodeId(ra_addr()),
          getNodeId(tuple->nb_main_addr()),
          ((tuple->getStatus() == OLSR_STATUS_SYM) ? "sym" : "not_sym"));

    state_.erase_nb_tuple(tuple);
}

///
/// \brief Adds a 2-hop neighbor tuple to the 2-hop Neighbor Set.
///
/// \param tuple the 2-hop neighbor tuple to be added.
///
void
OLSR::add_nb2hop_tuple(OLSR_nb2hop_tuple* tuple)
{
    debug("%f: Node %s adds 2-hop neighbor tuple: nb_addr = %s nb2hop_addr = %s\n",
          CURRENT_TIME,
          getNodeId(ra_addr()),
          getNodeId(tuple->nb_main_addr()),
          getNodeId(tuple->nb2hop_addr()));

    state_.insert_nb2hop_tuple(tuple);
}

///
/// \brief Removes a 2-hop neighbor tuple from the 2-hop Neighbor Set.
///
/// \param tuple the 2-hop neighbor tuple to be removed.
///
void
OLSR::rm_nb2hop_tuple(OLSR_nb2hop_tuple* tuple)
{
    debug("%f: Node %s removes 2-hop neighbor tuple: nb_addr = %s nb2hop_addr = %s\n",
          CURRENT_TIME,
          getNodeId(ra_addr()),
          getNodeId(tuple->nb_main_addr()),
          getNodeId(tuple->nb2hop_addr()));

    state_.erase_nb2hop_tuple(tuple);
}

///
/// \brief Adds an MPR selector tuple to the MPR Selector Set.
///
/// Advertised Neighbor Sequence Number (ANSN) is also updated.
///
/// \param tuple the MPR selector tuple to be added.
///
void
OLSR::add_mprsel_tuple(OLSR_mprsel_tuple* tuple)
{
    debug("%f: Node %s adds MPR selector tuple: nb_addr = %s\n",
          CURRENT_TIME,
          getNodeId(ra_addr()),
          getNodeId(tuple->main_addr()));

    state_.insert_mprsel_tuple(tuple);
    ansn_ = (ansn_ + 1)%(OLSR_MAX_SEQ_NUM + 1);
}

///
/// \brief Removes an MPR selector tuple from the MPR Selector Set.
///
/// Advertised Neighbor Sequence Number (ANSN) is also updated.
///
/// \param tuple the MPR selector tuple to be removed.
///
void
OLSR::rm_mprsel_tuple(OLSR_mprsel_tuple* tuple)
{
    debug("%f: Node %s removes MPR selector tuple: nb_addr = %s\n",
          CURRENT_TIME,
          getNodeId(ra_addr()),
          getNodeId(tuple->main_addr()));

    state_.erase_mprsel_tuple(tuple);
    ansn_ = (ansn_ + 1)%(OLSR_MAX_SEQ_NUM + 1);
}

///
/// \brief Adds a topology tuple to the Topology Set.
///
/// \param tuple the topology tuple to be added.
///
void
OLSR::add_topology_tuple(OLSR_topology_tuple* tuple)
{
    debug("%f: Node %s adds topology tuple: dest_addr = %s last_addr = %s seq = %d\n",
          CURRENT_TIME,
          getNodeId(ra_addr()),
          getNodeId(tuple->dest_addr()),
          getNodeId(tuple->last_addr()),
          tuple->seq());

    state_.insert_topology_tuple(tuple);
}

///
/// \brief Removes a topology tuple from the Topology Set.
///
/// \param tuple the topology tuple to be removed.
///
void
OLSR::rm_topology_tuple(OLSR_topology_tuple* tuple)
{
    debug("%f: Node %s removes topology tuple: dest_addr = %s last_addr = %s seq = %d\n",
          CURRENT_TIME,
          getNodeId(ra_addr()),
          getNodeId(tuple->dest_addr()),
          getNodeId(tuple->last_addr()),
          tuple->seq());

    state_.erase_topology_tuple(tuple);
}

///
/// \brief Adds an interface association tuple to the Interface Association Set.
///
/// \param tuple the interface association tuple to be added.
///
void
OLSR::add_ifaceassoc_tuple(OLSR_iface_assoc_tuple* tuple)
{
    debug("%f: Node %s adds iface association tuple: main_addr = %s iface_addr = %s\n",
          CURRENT_TIME,
          getNodeId(ra_addr()),
          getNodeId(tuple->main_addr()),
          getNodeId(tuple->iface_addr()));

    state_.insert_ifaceassoc_tuple(tuple);
}

///
/// \brief Removes an interface association tuple from the Interface Association Set.
///
/// \param tuple the interface association tuple to be removed.
///
void
OLSR::rm_ifaceassoc_tuple(OLSR_iface_assoc_tuple* tuple)
{
    debug("%f: Node %s removes iface association tuple: main_addr = %s iface_addr = %s\n",
          CURRENT_TIME,
          getNodeId(ra_addr()),
          getNodeId(tuple->main_addr()),
          getNodeId(tuple->iface_addr()));

    state_.erase_ifaceassoc_tuple(tuple);
}

///
/// \brief Gets the main address associated with a given interface address.
///
/// \param iface_addr the interface address.
/// \return the corresponding main address.
///
const nsaddr_t &
OLSR::get_main_addr(const nsaddr_t &iface_addr) const
{
    OLSR_iface_assoc_tuple* tuple =
        state_.find_ifaceassoc_tuple(iface_addr);

    if (tuple != NULL)
        return tuple->main_addr();
    return iface_addr;
}

///
/// \brief Determines which sequence number is bigger (as it is defined in RFC 3626).
///
/// \param s1 a sequence number.
/// \param s2 a sequence number.
/// \return true if s1 > s2, false in other case.
///
bool
OLSR::seq_num_bigger_than(uint16_t s1, uint16_t s2)
{
    return (s1 > s2 && s1-s2 <= OLSR_MAX_SEQ_NUM/2)
           || (s2 > s1 && s2-s1 > OLSR_MAX_SEQ_NUM/2);
}

///
/// \brief This auxiliary function (defined in RFC 3626) is used for calculating the MPR Set.
///
/// \param tuple the neighbor tuple which has the main address of the node we are going to calculate its degree to.
/// \return the degree of the node.
///
int
OLSR::degree(OLSR_nb_tuple* tuple)
{
    int degree = 0;
    for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++)
    {
        OLSR_nb2hop_tuple* nb2hop_tuple = *it;
        if (nb2hop_tuple->nb_main_addr() == tuple->nb_main_addr())
        {
            //OLSR_nb_tuple* nb_tuple =
            //    state_.find_nb_tuple(nb2hop_tuple->nb_main_addr());
            OLSR_nb_tuple* nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb2hop_addr());
            if (nb_tuple == NULL)
                degree++;
        }
    }
    return degree;
}

///
/// \brief Converts a decimal number of seconds to the mantissa/exponent format.
///
/// \param seconds decimal number of seconds we want to convert.
/// \return the number of seconds in mantissa/exponent format.
///
uint8_t
OLSR::seconds_to_emf(double seconds)
{
    // This implementation has been taken from unik-olsrd-0.4.5 (mantissa.c),
    // licensed under the GNU Public License (GPL)

    int a, b = 0;
    while (seconds/OLSR_C >= pow((double)2, (double)b))
        b++;
    b--;

    if (b < 0)
    {
        a = 1;
        b = 0;
    }
    else if (b > 15)
    {
        a = 15;
        b = 15;
    }
    else
    {
        a = (int)(16*((double)seconds/(OLSR_C*(double)pow((double)2, b))-1));
        while (a >= 16)
        {
            a -= 16;
            b++;
        }
    }

    return (uint8_t)(a*16+b);
}

///
/// \brief Converts a number of seconds in the mantissa/exponent format to a decimal number.
///
/// \param olsr_format number of seconds in mantissa/exponent format.
/// \return the decimal number of seconds.
///
double
OLSR::emf_to_seconds(uint8_t olsr_format)
{
    // This implementation has been taken from unik-olsrd-0.4.5 (mantissa.c),
    // licensed under the GNU Public License (GPL)
    int a = olsr_format >> 4;
    int b = olsr_format - a*16;
    return (double)(OLSR_C*(1+(double)a/16)*(double)pow((double)2, b));
}

///
/// \brief Returns the identifier of a node given the address of the attached OLSR agent.
///
/// \param addr the address of the OLSR routing agent.
/// \return the identifier of the node.
///
int
OLSR::node_id(const nsaddr_t &addr)
{
    return addr.toIPv4().getInt();
    /*
        // Preventing a bad use for this function
            if ((uint32_t)addr == IP_BROADCAST)
            return addr;
        // Getting node id
        Node* node = Node::get_node_by_address(addr);
        assert(node != NULL);
        return node->nodeid();
    */
}

const char * OLSR::getNodeId(const nsaddr_t &addr)
{
    return addr.str().c_str();
}

// Interfaces with other Inet
void OLSR:: processLinkBreak(const cObject *details)
{
    if (use_mac())
    {
        if (dynamic_cast<IPv4Datagram *>(const_cast<cObject*>(details)))
        {
            IPv4Datagram * dgram = const_cast<IPv4Datagram *>(check_and_cast<const IPv4Datagram *>(details));
            mac_failed(dgram);
            return;
        }
    }
}

void OLSR::finish()
{
    /*
    rtable_.clear();
    msgs_.clear();
    delete state_ptr;
    state_ptr=NULL;
    cancelAndDelete(&hello_timer_);
    cancelAndDelete(&tc_timer_);
    cancelAndDelete(&mid_timer_);

    helloTimer= NULL;   ///< Timer for sending HELLO messages.
    tcTimer= NULL;  ///< Timer for sending TC messages.
    midTimer = NULL;    ///< Timer for sending MID messages.
    */
}

OLSR::~OLSR()
{
    rtable_.clear();
    msgs_.clear();
    if (state_ptr)
    {
        delete state_ptr;
        state_ptr = NULL;
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
    */
    if (timerMessage)
    {
        cancelAndDelete(timerMessage);
        timerMessage = NULL;
    }

    while (timerQueuePtr && timerQueuePtr->size()>0)
    {
        OLSR_Timer * timer = timerQueuePtr->begin()->second;
        timerQueuePtr->erase(timerQueuePtr->begin());
        timer->setTuple(NULL);
        if (helloTimer==timer)
            helloTimer = NULL;
        else if (tcTimer==timer)
            tcTimer = NULL;
        else if (midTimer==timer)
            midTimer = NULL;
        delete timer;
    }

    if (helloTimer)
    {
        delete helloTimer;
        helloTimer = NULL;
    }
    if (tcTimer)
    {
        delete tcTimer;
        tcTimer = NULL;
    }
    if (midTimer)
    {
        delete midTimer;
        midTimer = NULL;
    }

    if (timerQueuePtr)
    {
        delete timerQueuePtr;
        timerQueuePtr = NULL;
    }
}


uint32_t OLSR::getRoute(const L3Address &dest, std::vector<L3Address> &add)
{
    add.clear();
    OLSR_rt_entry* rt_entry = rtable_.lookup(dest);
    L3Address apAddr;
    if (!rt_entry)
    {
        if (getAp(dest, apAddr))
        {
            OLSR_rt_entry* rt_entry = rtable_.lookup(apAddr);
            if (!rt_entry)
                return 0;
            for (int i = 0; i < (int) rt_entry->route.size(); i++)
                add.push_back(rt_entry->route[i]);
            add.push_back(apAddr);
            OLSR_rt_entry* rt_entry_aux = rtable_.find_send_entry(rt_entry);
            if (rt_entry_aux->next_addr() != add[0])
                throw cRuntimeError("OLSR Data base error");
            return rt_entry->dist();
        }
        return 0;
    }

    for (int i=0; i<(int)rt_entry->route.size(); i++)
        add.push_back(rt_entry->route[i]);
    add.push_back(dest);
    OLSR_rt_entry* rt_entry_aux = rtable_.find_send_entry(rt_entry);
    if (rt_entry_aux->next_addr() != add[0])
        throw cRuntimeError("OLSR Data base error");
    return rt_entry->dist();
}


bool OLSR::getNextHop(const L3Address &dest, L3Address &add, int &iface, double &cost)
{
    OLSR_rt_entry* rt_entry = rtable_.lookup(dest);
    if (!rt_entry)
    {
        L3Address apAddr;
        if (getAp(dest, apAddr))
        {

            OLSR_rt_entry* rt_entry = rtable_.lookup(apAddr);
            if (!rt_entry)
                return false;
            if (rt_entry->route.size())
                add = rt_entry->route[0];
            else
                add = rt_entry->next_addr();
            OLSR_rt_entry* rt_entry_aux = rtable_.find_send_entry(rt_entry);
            if (rt_entry_aux->next_addr() != add)
                throw cRuntimeError("OLSR Data base error");

            InterfaceEntry * ie = getInterfaceWlanByAddress(rt_entry->iface_addr());
            iface = ie->getInterfaceId();
            cost = rt_entry->dist();
            return true;
        }
        return false;
    }

    if (rt_entry->route.size())
        add = rt_entry->route[0];
    else
        add = rt_entry->next_addr();
    OLSR_rt_entry* rt_entry_aux = rtable_.find_send_entry(rt_entry);
    if (rt_entry_aux->next_addr() != add)
        throw cRuntimeError("OLSR Data base error");

    InterfaceEntry * ie = getInterfaceWlanByAddress(rt_entry->iface_addr());
    iface = ie->getInterfaceId();
    cost = rt_entry->dist();
    return true;
}

bool OLSR::isProactive()
{
    return true;
}


bool OLSR::isOurType(cPacket * msg)
{
    OLSR_pkt * pkt = dynamic_cast<OLSR_pkt  *>(msg);
    if (pkt)
        return true;
    return false;
}

bool OLSR::getDestAddress(cPacket *msg, L3Address &dest)
{
    return false;
}

void OLSR::scheduleNextEvent()
{
    TimerQueue::iterator e = timerQueuePtr->begin();
    if (timerMessage->isScheduled())
    {
        if (e->first < timerMessage->getArrivalTime())
        {
            cancelEvent(timerMessage);
            scheduleAt(e->first, timerMessage);
        }
        else if (e->first>timerMessage->getArrivalTime())
            throw cRuntimeError("OLSR timer Queue problem");
    }
    else
    {
        scheduleAt(e->first, timerMessage);
    }
}


// Group methods, allow the anycast procedure
int OLSR::getRouteGroup(const AddressGroup &gr, std::vector<L3Address> &add)
{

    int distance = 1000;
    add.clear();
    for (AddressGroupConstIterator it = gr.begin(); it!=gr.end(); it++)
    {
        L3Address dest = *it;
        OLSR_rt_entry* rt_entry = rtable_.lookup(dest);
        if (!rt_entry)
            continue;
        if (distance<(int)rt_entry->dist() || (distance==(int)rt_entry->dist() && intrand(1)))
            continue;
        distance = rt_entry->dist();
        add.clear();
        for (int i=0; i<(int)rt_entry->route.size(); i++)
            add.push_back(rt_entry->route[i]);
        add.push_back(dest);

        add[rt_entry->route.size()] = dest;
        OLSR_rt_entry* rt_entry_aux = rtable_.find_send_entry(rt_entry);
        if (rt_entry_aux->next_addr() != add[0])
            throw cRuntimeError("OLSR Data base error");
    }
    if (distance==1000)
        return 0;
    return distance;
}

bool OLSR::getNextHopGroup(const AddressGroup &gr, L3Address &add, int &iface, L3Address &gw)
{

    int distance = 1000;
    for (AddressGroupConstIterator it = gr.begin(); it!=gr.end(); it++)
    {
        L3Address dest = *it;
        OLSR_rt_entry* rt_entry = rtable_.lookup(dest);
        if (!rt_entry)
            continue;
        if (distance<(int)rt_entry->dist() || (distance==(int)rt_entry->dist() && intrand(1)))
            continue;
        distance = rt_entry->dist();
        if (rt_entry->route.size())
            add = rt_entry->route[0];
        else
            add = rt_entry->next_addr();
        OLSR_rt_entry* rt_entry_aux = rtable_.find_send_entry(rt_entry);
        if (rt_entry_aux->next_addr() != add)
            throw cRuntimeError("OLSR Data base error");
        InterfaceEntry * ie = getInterfaceWlanByAddress(rt_entry->iface_addr());
        iface = ie->getInterfaceId();
        gw = dest;
    }
    if (distance==1000)
        return false;
    return true;
}





int  OLSR::getRouteGroup(const L3Address& dest, std::vector<L3Address> &add, L3Address& gateway, bool &isGroup, int group)
{
    AddressGroup gr;
    int distance = 0;
    if (findInAddressGroup(dest, group))
    {
        getAddressGroup(gr, group);
        distance = getRouteGroup(gr, add);
        if (distance == 0)
            return 0;
        gateway = add.back();
        isGroup = true;

     }
    else
    {
        distance = getRoute(dest, add);
        isGroup = false;
    }
    return distance;
}

bool OLSR::getNextHopGroup(const L3Address& dest, L3Address &next, int &iface, L3Address& gw, bool &isGroup, int group)
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



L3Address OLSR::getIfaceAddressFromIndex(int index)
{
    InterfaceEntry * entry = getInterfaceEntry(index);
    if (this->isInMacLayer())
        return L3Address(entry->getMacAddress());
    else
        return L3Address(entry->ipv4Data()->getIPAddress());
}

} // namespace inetmanet

} // namespace inet

