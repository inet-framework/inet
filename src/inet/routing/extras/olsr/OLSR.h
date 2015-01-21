/***************************************************************************
 *   Copyright (C) 2004 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
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
/// \file   OLSR.h
/// \brief  Header file for OLSR agent and related classes.
///
/// Here are defined all timers used by OLSR, including those for managing internal
/// state and those for sending messages. Class OLSR is also defined, therefore this
/// file has signatures for the most important methods. Lots of constants are also
/// defined.
///

#ifndef __OLSR_omnet_h__

#define __OLSR_omnet_h__
#include "inet/routing/extras/base/ManetRoutingBase.h"

#include "inet/routing/extras/olsr/OLSRpkt_m.h"
#include "inet/routing/extras/olsr/OLSR_state.h"
#include "inet/routing/extras/olsr/OLSR_rtable.h"
#include "inet/routing/extras/olsr/OLSR_repositories.h"
#include "inet/common/INETUtils.h"

#include <map>
#include <vector>

#include <assert.h>

namespace inet {

namespace inetmanet {

/********** Useful macros **********/

/// Returns maximum of two numbers.
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

/// Returns minimum of two numbers.
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/// Defines nullptr as zero, used for pointers.
#ifndef nullptr
#define nullptr 0
#endif

#define IP_BROADCAST     ((uint32_t) 0xffffffff)

/// Gets current time from the scheduler.
#ifndef CURRENT_TIME
#define CURRENT_TIME    SIMTIME_DBL(simTime())
#endif

#ifndef CURRENT_TIME_T
#define CURRENT_TIME_T  SIMTIME_DBL(simTime())
#endif

#define debug  EV << utils::stringf



///
/// \brief Gets the delay between a given time and the current time.
///
/// If given time is previous to the current one, then this macro returns
/// a number close to 0. This is used for scheduling events at a certain moment.
///
#define DELAY(time) (((time) < (CURRENT_TIME)) ? (0.000001) : \
    (time - CURRENT_TIME + 0.000001))

#define DELAY_T(time) (((time) < (CURRENT_TIME_T)) ? (0.000001) : \
    (time - CURRENT_TIME_T + 0.000001))


/// Scaling factor used in RFC 3626.
#define OLSR_C      0.0625


    /********** Holding times **********/

    /// Neighbor holding time.
#define OLSR_NEIGHB_HOLD_TIME   3*OLSR_REFRESH_INTERVAL
    /// Top holding time.
#define OLSR_TOP_HOLD_TIME  3*OLSR_TC_INTERVAL
    /// Dup holding time.
#define OLSR_DUP_HOLD_TIME  30
    /// MID holding time.
#define OLSR_MID_HOLD_TIME  3*OLSR_MID_INTERVAL

/********** Link types **********/

/// Unspecified link type.
#define OLSR_UNSPEC_LINK    0
/// Asymmetric link type.
#define OLSR_ASYM_LINK      1
/// Symmetric link type.
#define OLSR_SYM_LINK       2
/// Lost link type.
#define OLSR_LOST_LINK      3

/********** Neighbor types **********/

/// Not neighbor type.
#define OLSR_NOT_NEIGH      0
/// Symmetric neighbor type.
#define OLSR_SYM_NEIGH      1
/// Asymmetric neighbor type.
#define OLSR_MPR_NEIGH      2


/********** Willingness **********/

/// Willingness for forwarding packets from other nodes: never.
#define OLSR_WILL_NEVER     0
/// Willingness for forwarding packets from other nodes: low.
#define OLSR_WILL_LOW       1
/// Willingness for forwarding packets from other nodes: medium.
#define OLSR_WILL_DEFAULT   3
/// Willingness for forwarding packets from other nodes: high.
#define OLSR_WILL_HIGH      6
/// Willingness for forwarding packets from other nodes: always.
#define OLSR_WILL_ALWAYS    7


/********** Miscellaneous constants **********/

/// Maximum allowed jitter.
#define OLSR_MAXJITTER      OLSR_HELLO_INTERVAL/4
/// Maximum allowed sequence number.
#define OLSR_MAX_SEQ_NUM    65535
/// Used to set status of an OLSR_nb_tuple as "not symmetric".
#define OLSR_STATUS_NOT_SYM 0
/// Used to set status of an OLSR_nb_tuple as "symmetric".
#define OLSR_STATUS_SYM     1
/// Random number between [0-OLSR_MAXJITTER] used to jitter OLSR packet transmission.
//#define JITTER            (Random::uniform()*OLSR_MAXJITTER)

class OLSR;         // forward declaration

/********** Timers **********/

/// Basic timer class

class OLSR_Timer :  public cOwnedObject /*cMessage*/
{
  protected:
    OLSR*       agent_; ///< OLSR agent which created the timer.
    cObject* tuple_;
  public:

    virtual void removeTimer();
    OLSR_Timer(OLSR* agent);
    OLSR_Timer();
    ~OLSR_Timer();
    virtual void expire() = 0;
    virtual void removeQueueTimer();
    virtual void resched(double time);
    virtual void setTuple(cObject *tuple) {tuple_ = tuple;}
};


/// Timer for sending an enqued message.
class OLSR_MsgTimer : public OLSR_Timer
{
  public:
    OLSR_MsgTimer(OLSR* agent) : OLSR_Timer(agent) {}
    OLSR_MsgTimer():OLSR_Timer() {}
    void expire() override;
};

/// Timer for sending HELLO messages.
class OLSR_HelloTimer : public OLSR_Timer
{
  public:
    OLSR_HelloTimer(OLSR* agent) : OLSR_Timer(agent) {}
    OLSR_HelloTimer():OLSR_Timer() {}
    void expire() override;
};


/// Timer for sending TC messages.
class OLSR_TcTimer : public OLSR_Timer
{
  public:
    OLSR_TcTimer(OLSR* agent) : OLSR_Timer(agent) {}
    OLSR_TcTimer():OLSR_Timer() {}
    void expire() override;
};


/// Timer for sending MID messages.
class OLSR_MidTimer : public OLSR_Timer
{
  public:
    OLSR_MidTimer(OLSR* agent) : OLSR_Timer(agent) {}
    OLSR_MidTimer():OLSR_Timer() {}
    virtual void expire() override;
};



/// Timer for removing duplicate tuples: OLSR_dup_tuple.
class OLSR_DupTupleTimer : public OLSR_Timer
{
//  protected:
//  OLSR_dup_tuple* tuple_;
  public:
    OLSR_DupTupleTimer(OLSR* agent, OLSR_dup_tuple* tuple) : OLSR_Timer(agent)
    {
        tuple_ = tuple;
    }
    void setTuple(OLSR_dup_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~OLSR_DupTupleTimer();
    virtual void expire() override;
};

/// Timer for removing link tuples: OLSR_link_tuple.
class OLSR_LinkTupleTimer : public OLSR_Timer
{
  public:
    OLSR_LinkTupleTimer(OLSR* agent, OLSR_link_tuple* tuple);

    void setTuple(OLSR_link_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~OLSR_LinkTupleTimer();
    virtual void expire() override;
  protected:
    //OLSR_link_tuple*  tuple_; ///< OLSR_link_tuple which must be removed.
    bool            first_time_;

};


/// Timer for removing nb2hop tuples: OLSR_nb2hop_tuple.

class OLSR_Nb2hopTupleTimer : public OLSR_Timer
{
  public:
    OLSR_Nb2hopTupleTimer(OLSR* agent, OLSR_nb2hop_tuple* tuple) : OLSR_Timer(agent)
    {
        tuple_ = tuple;
    }

    void setTuple(OLSR_nb2hop_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~OLSR_Nb2hopTupleTimer();
    virtual void expire() override;
//  protected:
//  OLSR_nb2hop_tuple*  tuple_; ///< OLSR_link_tuple which must be removed.

};




/// Timer for removing MPR selector tuples: OLSR_mprsel_tuple.
class OLSR_MprSelTupleTimer : public OLSR_Timer
{
  public:
    OLSR_MprSelTupleTimer(OLSR* agent, OLSR_mprsel_tuple* tuple) : OLSR_Timer(agent)
    {
        tuple_ = tuple;
    }

    void setTuple(OLSR_mprsel_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~OLSR_MprSelTupleTimer();
    virtual void expire() override;

//  protected:
//  OLSR_mprsel_tuple*  tuple_; ///< OLSR_link_tuple which must be removed.

};


/// Timer for removing topology tuples: OLSR_topology_tuple.

class OLSR_TopologyTupleTimer : public OLSR_Timer
{
  public:
    OLSR_TopologyTupleTimer(OLSR* agent, OLSR_topology_tuple* tuple) : OLSR_Timer(agent)
    {
        tuple_ = tuple;
    }

    void setTuple(OLSR_topology_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~OLSR_TopologyTupleTimer();
    virtual void expire() override;
//  protected:
//  OLSR_topology_tuple*    tuple_; ///< OLSR_link_tuple which must be removed.

};

/// Timer for removing interface association tuples: OLSR_iface_assoc_tuple.
class OLSR_IfaceAssocTupleTimer : public OLSR_Timer
{
  public:
    OLSR_IfaceAssocTupleTimer(OLSR* agent, OLSR_iface_assoc_tuple* tuple) : OLSR_Timer(agent)
    {
        tuple_ = tuple;
    }

    void setTuple(OLSR_iface_assoc_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~OLSR_IfaceAssocTupleTimer();
    virtual void expire() override;
//  protected:
//  OLSR_iface_assoc_tuple* tuple_; ///< OLSR_link_tuple which must be removed.

};

/********** OLSR Agent **********/


///
/// \brief Routing agent which implements %OLSR protocol following RFC 3626.
///
/// Interacts with TCL interface through command() method. It implements all
/// functionalities related to sending and receiving packets and managing
/// internal state.
///

typedef std::set<OLSR_Timer *> TimerPendingList;
typedef std::multimap <simtime_t, OLSR_Timer *> TimerQueue;


class OLSR : public ManetRoutingBase
{
  protected:
    /********** Intervals **********/
    /// HELLO messages emission interval.
    double OLSR_HELLO_INTERVAL;// 2

    /// TC messages emission interval.
    double OLSR_TC_INTERVAL;//    5

    /// MID messages emission interval.
    double OLSR_MID_INTERVAL;//   OLSR_TC_INTERVAL

    ///
    /// \brief Period at which a node must cite every link and every neighbor.
    ///
    /// We only use this value in order to define OLSR_NEIGHB_HOLD_TIME.
    ///
    double OLSR_REFRESH_INTERVAL;//   2

    double jitter() {return uniform(0,(double)OLSR_MAXJITTER);}
#define JITTER jitter()

  private:
    friend class OLSR_HelloTimer;
    friend class OLSR_TcTimer;
    friend class OLSR_MidTimer;
    friend class OLSR_DupTupleTimer;
    friend class OLSR_LinkTupleTimer;
    friend class OLSR_Nb2hopTupleTimer;
    friend class OLSR_MprSelTupleTimer;
    friend class OLSR_TopologyTupleTimer;
    friend class OLSR_IfaceAssocTupleTimer;
    friend class OLSR_MsgTimer;
    friend class OLSR_Timer;
  protected:

    //std::priority_queue<TimerQueueElem> *timerQueuePtr;
    bool topologyChange;
    virtual void setTopologyChanged(bool p) {topologyChange = p;}
    virtual bool getTopologyChanged() {return topologyChange;}
    TimerQueue *timerQueuePtr;

    cMessage *timerMessage;

// must be protected and used for dereved class OLSR_ETX
    /// A list of pending messages which are buffered awaiting for being sent.
    std::vector<OLSR_msg>   msgs_;
    /// Routing table.
    OLSR_rtable     rtable_;
    /// Internal state with all needed data structs.

    OLSR_state      *state_ptr;

    /// Packets sequence number counter.
    uint16_t    pkt_seq_;
    /// Messages sequence number counter.
    uint16_t    msg_seq_;
    /// Advertised Neighbor Set sequence number.
    uint16_t    ansn_;

    /// HELLO messages' emission interval.
    SimTime     hello_ival_;
    /// TC messages' emission interval.
    SimTime     tc_ival_;
    /// MID messages' emission interval.
    SimTime     mid_ival_;
    /// Willingness for forwarding packets on behalf of other nodes.
    int     willingness_;
    /// Determines if layer 2 notifications are enabled or not.
    int     use_mac_;
    bool useIndex;


    /// Address of the routing agent.
    nsaddr_t ra_addr_;

    bool optimizedMid;

  protected:
// Omnet INET vaiables and functions
    char nodeName[50];


    virtual OLSR_pkt * check_packet(cPacket*, nsaddr_t &, int &);

    // PortClassifier*  dmux_;      ///< For passing packets up to agents.
    // Trace*       logtarget_; ///< For logging.

    OLSR_HelloTimer *helloTimer;    ///< Timer for sending HELLO messages.
    OLSR_TcTimer    *tcTimer;   ///< Timer for sending TC messages.
    OLSR_MidTimer   *midTimer;  ///< Timer for sending MID messages.

#define hello_timer_  (*helloTimer)
#define  tc_timer_  (*tcTimer)
#define mid_timer_  (*midTimer)

    /// Increments packet sequence number and returns the new value.
    inline uint16_t pkt_seq()
    {
        pkt_seq_ = (pkt_seq_ + 1)%(OLSR_MAX_SEQ_NUM + 1);
        return pkt_seq_;
    }
    /// Increments message sequence number and returns the new value.
    inline uint16_t msg_seq()
    {
        msg_seq_ = (msg_seq_ + 1)%(OLSR_MAX_SEQ_NUM + 1);
        return msg_seq_;
    }

    inline nsaddr_t&    ra_addr()   { return ra_addr_;}

    inline SimTime&     hello_ival()    { return hello_ival_;}
    inline SimTime&     tc_ival()   { return tc_ival_;}
    inline SimTime&     mid_ival()  { return mid_ival_;}
    inline int&     willingness()   { return willingness_;}
    inline int&     use_mac()   { return use_mac_;}

    inline linkset_t&   linkset()   { return state_ptr->linkset(); }
    inline mprset_t&    mprset()    { return state_ptr->mprset(); }
    inline mprselset_t& mprselset() { return state_ptr->mprselset(); }
    inline nbset_t&     nbset()     { return state_ptr->nbset(); }
    inline nb2hopset_t& nb2hopset() { return state_ptr->nb2hopset(); }
    inline topologyset_t&   topologyset()   { return state_ptr->topologyset(); }
    inline dupset_t&    dupset()    { return state_ptr->dupset(); }
    inline ifaceassocset_t& ifaceassocset() { return state_ptr->ifaceassocset(); }

    virtual void        recv_olsr(cMessage*);

    virtual void        mpr_computation();
    virtual void        rtable_computation();

    virtual bool        process_hello(OLSR_msg&, const nsaddr_t &, const nsaddr_t &, const int &);
    virtual bool        process_tc(OLSR_msg&, const nsaddr_t &, const int &);
    virtual void        process_mid(OLSR_msg&, const nsaddr_t &, const int &);

    virtual void        forward_default(OLSR_msg&, OLSR_dup_tuple*, const nsaddr_t &, const nsaddr_t &);
    virtual void        forward_data(cMessage* p) {}

    virtual void        enque_msg(OLSR_msg&, double);
    virtual void        send_hello();
    virtual void        send_tc();
    virtual void        send_mid();
    virtual void        send_pkt();

    virtual bool        link_sensing(OLSR_msg&, const nsaddr_t &, const nsaddr_t &, const int &);
    virtual bool        populate_nbset(OLSR_msg&);
    virtual bool        populate_nb2hopset(OLSR_msg&);
    virtual void        populate_mprselset(OLSR_msg&);

    virtual void        set_hello_timer();
    virtual void        set_tc_timer();
    virtual void        set_mid_timer();

    virtual void        nb_loss(OLSR_link_tuple*);
    virtual void        add_dup_tuple(OLSR_dup_tuple*);
    virtual void        rm_dup_tuple(OLSR_dup_tuple*);
    virtual void        add_link_tuple(OLSR_link_tuple*, uint8_t);
    virtual void        rm_link_tuple(OLSR_link_tuple*);
    virtual void        updated_link_tuple(OLSR_link_tuple*, uint8_t willingness);
    virtual void        add_nb_tuple(OLSR_nb_tuple*);
    virtual void        rm_nb_tuple(OLSR_nb_tuple*);
    virtual void        add_nb2hop_tuple(OLSR_nb2hop_tuple*);
    virtual void        rm_nb2hop_tuple(OLSR_nb2hop_tuple*);
    virtual void        add_mprsel_tuple(OLSR_mprsel_tuple*);
    virtual void        rm_mprsel_tuple(OLSR_mprsel_tuple*);
    virtual void        add_topology_tuple(OLSR_topology_tuple*);
    virtual void        rm_topology_tuple(OLSR_topology_tuple*);
    virtual void        add_ifaceassoc_tuple(OLSR_iface_assoc_tuple*);
    virtual void        rm_ifaceassoc_tuple(OLSR_iface_assoc_tuple*);
    virtual OLSR_nb_tuple*    find_or_add_nb(OLSR_link_tuple*, uint8_t willingness);

    const nsaddr_t  & get_main_addr(const nsaddr_t&) const;
    virtual int     degree(OLSR_nb_tuple*);

    static bool seq_num_bigger_than(uint16_t, uint16_t);
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void    mac_failed(IPv4Datagram*);
    virtual void    recv(cMessage *p) {}

    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    //virtual void processPromiscuous(const cObject *details){};
    virtual void processLinkBreak(const cObject *details) override;
    virtual void scheduleNextEvent();

    L3Address getIfaceAddressFromIndex(int index);

    const char * getNodeId(const nsaddr_t &addr);

  public:
    OLSR();
    virtual ~OLSR();


    static double       emf_to_seconds(uint8_t);
    static uint8_t      seconds_to_emf(double);
    static int      node_id(const nsaddr_t&);

    // Routing information access
    virtual bool supportGetRoute() override {return true;}
    virtual uint32_t getRoute(const L3Address &, std::vector<L3Address> &) override;
    virtual bool getNextHop(const L3Address &, L3Address &add, int &iface, double &) override;
    virtual bool isProactive() override;
    virtual void setRefreshRoute(const L3Address &destination, const L3Address & nextHop,bool isReverse) override {}
    virtual bool isOurType(cPacket *) override;
    virtual bool getDestAddress(cPacket *, L3Address &) override;
    virtual int getRouteGroup(const AddressGroup &gr, std::vector<L3Address>&) override;
    virtual bool getNextHopGroup(const AddressGroup &gr, L3Address &add, int &iface, L3Address&) override;
    virtual int  getRouteGroup(const L3Address&, std::vector<L3Address> &, L3Address&, bool &, int group = 0) override;
    virtual bool getNextHopGroup(const L3Address&, L3Address &add, int &iface, L3Address&, bool &, int group = 0) override;
};

} // namespace inetmanet

} // namespace inet

#endif

