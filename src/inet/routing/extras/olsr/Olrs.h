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
#include <map>
#include <vector>
#include <assert.h>
#include "inet/common/INETUtils.h"
#include "inet/routing/extras/base/ManetRoutingBase.h"
#include "inet/routing/extras/olsr/Olrs_repositories.h"
#include "inet/routing/extras/olsr/Olrs_rtable.h"
#include "inet/routing/extras/olsr/Olrs_state.h"
#include "inet/routing/extras/olsr/OlrsPkt_m.h"

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
#define OLSR_NEIGHB_HOLD_TIME   3 * OLSR_REFRESH_INTERVAL
	/// Top holding time.
#define OLSR_TOP_HOLD_TIME  3 * tc_ival()
	/// Dup holding time.
#define OLSR_DUP_HOLD_TIME  30
	/// MID holding time.
#define OLSR_MID_HOLD_TIME  3 * mid_ival()

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
#define OLSR_MAXJITTER      hello_ival()/4
/// Maximum allowed sequence number.
#define OLSR_MAX_SEQ_NUM    65535
/// Used to set status of an OLSR_nb_tuple as "not symmetric".
#define OLSR_STATUS_NOT_SYM 0
/// Used to set status of an OLSR_nb_tuple as "symmetric".
#define OLSR_STATUS_SYM     1
/// Random number between [0-OLSR_MAXJITTER] used to jitter OLSR packet transmission.
//#define JITTER            (Random::uniform()*OLSR_MAXJITTER)

class Olsr;         // forward declaration

/********** Timers **********/

/// Basic timer class

class Olsr_Timer :  public ManetTimer /*cMessage*/
{
  protected:
    cObject* tuple_ = nullptr;
  public:
    Olsr_Timer(ManetRoutingBase* agent) : ManetTimer(agent) {}
    Olsr_Timer() : ManetTimer() {}
    virtual void setTuple(cObject *tuple) {tuple_ = tuple;}
};


/// Timer for sending an enqued message.
class Olsr_MsgTimer : public Olsr_Timer
{
  public:
    Olsr_MsgTimer(ManetRoutingBase* agent) : Olsr_Timer(agent) {}
    Olsr_MsgTimer():Olsr_Timer() {}
    void expire() override;
};

/// Timer for sending HELLO messages.
class Olsr_HelloTimer : public Olsr_Timer
{
  public:
    Olsr_HelloTimer(ManetRoutingBase* agent) : Olsr_Timer(agent) {}
    Olsr_HelloTimer():Olsr_Timer() {}
    void expire() override;
};


/// Timer for sending TC messages.
class Olsr_TcTimer : public Olsr_Timer
{
  public:
    Olsr_TcTimer(ManetRoutingBase* agent) : Olsr_Timer(agent) {}
    Olsr_TcTimer():Olsr_Timer() {}
    void expire() override;
};


/// Timer for sending MID messages.
class Olsr_MidTimer : public Olsr_Timer
{
  public:
    Olsr_MidTimer(ManetRoutingBase* agent) : Olsr_Timer(agent) {}
    Olsr_MidTimer():Olsr_Timer() {}
    virtual void expire() override;
};



/// Timer for removing duplicate tuples: OLSR_dup_tuple.
class Olsr_DupTupleTimer : public Olsr_Timer
{
//  protected:
//  OLSR_dup_tuple* tuple_;
  public:
    Olsr_DupTupleTimer(ManetRoutingBase* agent, Olsr_dup_tuple* tuple) : Olsr_Timer(agent)
    {
        tuple_ = tuple;
    }
    void setTuple(Olsr_dup_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~Olsr_DupTupleTimer();
    virtual void expire() override;
};

/// Timer for removing link tuples: OLSR_link_tuple.
class Olsr_LinkTupleTimer : public Olsr_Timer
{
  public:
    Olsr_LinkTupleTimer(ManetRoutingBase* agent, Olsr_link_tuple* tuple);

    void setTuple(Olsr_link_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~Olsr_LinkTupleTimer();
    virtual void expire() override;
  protected:
    //OLSR_link_tuple*  tuple_; ///< OLSR_link_tuple which must be removed.
    bool            first_time_;

};


/// Timer for removing nb2hop tuples: OLSR_nb2hop_tuple.

class Olsr_Nb2hopTupleTimer : public Olsr_Timer
{
  public:
    Olsr_Nb2hopTupleTimer(ManetRoutingBase* agent, Olsr_nb2hop_tuple* tuple) : Olsr_Timer(agent)
    {
        tuple_ = tuple;
    }

    void setTuple(Olsr_nb2hop_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~Olsr_Nb2hopTupleTimer();
    virtual void expire() override;
//  protected:
//  OLSR_nb2hop_tuple*  tuple_; ///< OLSR_link_tuple which must be removed.

};




/// Timer for removing MPR selector tuples: OLSR_mprsel_tuple.
class Olsr_MprSelTupleTimer : public Olsr_Timer
{
  public:
    Olsr_MprSelTupleTimer(ManetRoutingBase* agent, Olsr_mprsel_tuple* tuple) : Olsr_Timer(agent)
    {
        tuple_ = tuple;
    }

    void setTuple(Olsr_mprsel_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~Olsr_MprSelTupleTimer();
    virtual void expire() override;

//  protected:
//  OLSR_mprsel_tuple*  tuple_; ///< OLSR_link_tuple which must be removed.

};


/// Timer for removing topology tuples: OLSR_topology_tuple.

class Olsr_TopologyTupleTimer : public Olsr_Timer
{
  public:
    Olsr_TopologyTupleTimer(ManetRoutingBase* agent, Olsr_topology_tuple* tuple) : Olsr_Timer(agent)
    {
        tuple_ = tuple;
    }

    void setTuple(Olsr_topology_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~Olsr_TopologyTupleTimer();
    virtual void expire() override;
//  protected:
//  OLSR_topology_tuple*    tuple_; ///< OLSR_link_tuple which must be removed.

};

/// Timer for removing interface association tuples: OLSR_iface_assoc_tuple.
class Olsr_IfaceAssocTupleTimer : public Olsr_Timer
{
  public:
    Olsr_IfaceAssocTupleTimer(ManetRoutingBase* agent, Olsr_iface_assoc_tuple* tuple) : Olsr_Timer(agent)
    {
        tuple_ = tuple;
    }

    void setTuple(Olsr_iface_assoc_tuple* tuple) {tuple_ = tuple; tuple->asocTimer = this;}
    ~Olsr_IfaceAssocTupleTimer();
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

typedef std::set<Olsr_Timer *> TimerPendingList;

class Olsr : public ManetRoutingBase
{
  protected:
    /********** Intervals **********/
    ///
    /// \brief Period at which a node must cite every link and every neighbor.
    ///
    /// We only use this value in order to define OLSR_NEIGHB_HOLD_TIME.
    ///
    double OLSR_REFRESH_INTERVAL;//   2

    double jitter() {return uniform(0,(double)OLSR_MAXJITTER);}
#define JITTER jitter()

  private:
    friend class Olsr_HelloTimer;
    friend class Olsr_TcTimer;
    friend class Olsr_MidTimer;
    friend class Olsr_DupTupleTimer;
    friend class Olsr_LinkTupleTimer;
    friend class Olsr_Nb2hopTupleTimer;
    friend class Olsr_MprSelTupleTimer;
    friend class Olsr_TopologyTupleTimer;
    friend class Olsr_IfaceAssocTupleTimer;
    friend class Olsr_MsgTimer;
    friend class Olsr_Timer;
  protected:

    //std::priority_queue<TimerQueueElem> *timerQueuePtr;
    bool topologyChange = false;
    virtual void setTopologyChanged(bool p) {topologyChange = p;}
    virtual bool getTopologyChanged() {return topologyChange;}

// must be protected and used for dereved class OLSR_ETX
    /// A list of pending messages which are buffered awaiting for being sent.
    std::vector<OlsrMsg>   msgs_;
    /// Routing table.
    Olsr_rtable     rtable_;

    typedef std::map<nsaddr_t,Olsr_rtable*> GlobalRtable;
    static GlobalRtable globalRtable;
    typedef std::map<nsaddr_t,std::vector<nsaddr_t> > DistributionPath;
    static DistributionPath distributionPath;
    bool computed = false;
    /// Internal state with all needed data structs.

    Olsr_state      *state_ptr = nullptr;

    /// Packets sequence number counter.
    uint16_t    pkt_seq_ = OLSR_MAX_SEQ_NUM;
    /// Messages sequence number counter.
    uint16_t    msg_seq_ = OLSR_MAX_SEQ_NUM;
    /// Advertised Neighbor Set sequence number.
    uint16_t    ansn_ = OLSR_MAX_SEQ_NUM;

    /// HELLO messages' emission interval.
    cPar     *hello_ival_ = nullptr;
    /// TC messages' emission interval.
    cPar     *tc_ival_ = nullptr;
    /// MID messages' emission interval.
    cPar     *mid_ival_ = nullptr;
    /// Willingness for forwarding packets on behalf of other nodes.
    cPar     *willingness_ = nullptr;
    /// Determines if layer 2 notifications are enabled or not.
    int  use_mac_ = false;
    bool useIndex = false;

    bool optimizedMid = false;

  protected:
// Omnet INET vaiables and functions
    char nodeName[50];

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;

    bool check_packet(Packet*, nsaddr_t &, int &);

    // PortClassifier*  dmux_;      ///< For passing packets up to agents.
    // Trace*       logtarget_; ///< For logging.

    Olsr_HelloTimer *helloTimer = nullptr;    ///< Timer for sending HELLO messages.
    Olsr_TcTimer    *tcTimer = nullptr;   ///< Timer for sending TC messages.
    Olsr_MidTimer   *midTimer = nullptr;  ///< Timer for sending MID messages.

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

    inline nsaddr_t    ra_addr()   { return getAddress();}

    inline double     hello_ival()    { return hello_ival_->doubleValue();}
    inline double     tc_ival()   { return tc_ival_->doubleValue();}
    inline double     mid_ival()  { return mid_ival_->doubleValue();}
    inline int     willingness()   { return willingness_->intValue();}
    inline int     use_mac()   { return use_mac_;}

    inline linkset_t&   linkset()   { return state_ptr->linkset(); }
    inline mprset_t&    mprset()    { return state_ptr->mprset(); }
    inline mprselset_t& mprselset() { return state_ptr->mprselset(); }
    inline nbset_t&     nbset()     { return state_ptr->nbset(); }
    inline nb2hopset_t& nb2hopset() { return state_ptr->nb2hopset(); }
    inline topologyset_t&   topologyset()   { return state_ptr->topologyset(); }
    inline dupset_t&    dupset()    { return state_ptr->dupset(); }
    inline ifaceassocset_t& ifaceassocset() { return state_ptr->ifaceassocset(); }

    virtual void        recv_olsr(Packet*);

    virtual void        CoverTwoHopNeighbors(const nsaddr_t &neighborMainAddr, nb2hopset_t & N2);
    virtual void        mpr_computation();
    virtual void        rtable_computation();

    virtual bool        process_hello(OlsrMsg&, const nsaddr_t &, const nsaddr_t &, const int &);
    virtual bool        process_tc(OlsrMsg&, const nsaddr_t &, const int &);
    virtual void        process_mid(OlsrMsg&, const nsaddr_t &, const int &);

    virtual void        forward_default(OlsrMsg&, Olsr_dup_tuple*, const nsaddr_t &, const nsaddr_t &);
    virtual void        forward_data(cMessage* p) {}

    virtual void        enque_msg(OlsrMsg&, double);
    virtual void        send_hello();
    virtual void        send_tc();
    virtual void        send_mid();
    virtual void        send_pkt();

    virtual bool        link_sensing(OlsrMsg&, const nsaddr_t &, const nsaddr_t &, const int &);
    virtual bool        populate_nbset(OlsrMsg&);
    virtual bool        populate_nb2hopset(OlsrMsg&);
    virtual void        populate_mprselset(OlsrMsg&);

    virtual void        set_hello_timer();
    virtual void        set_tc_timer();
    virtual void        set_mid_timer();

    virtual void        nb_loss(Olsr_link_tuple*);
    virtual void        add_dup_tuple(Olsr_dup_tuple*);
    virtual void        rm_dup_tuple(Olsr_dup_tuple*);
    virtual void        add_link_tuple(Olsr_link_tuple*, uint8_t);
    virtual void        rm_link_tuple(Olsr_link_tuple*);
    virtual void        updated_link_tuple(Olsr_link_tuple*, uint8_t willingness);
    virtual void        add_nb_tuple(Olsr_nb_tuple*);
    virtual void        rm_nb_tuple(Olsr_nb_tuple*);
    virtual void        add_nb2hop_tuple(Olsr_nb2hop_tuple*);
    virtual void        rm_nb2hop_tuple(Olsr_nb2hop_tuple*);
    virtual void        add_mprsel_tuple(Olsr_mprsel_tuple*);
    virtual void        rm_mprsel_tuple(Olsr_mprsel_tuple*);
    virtual void        add_topology_tuple(Olsr_topology_tuple*);
    virtual void        rm_topology_tuple(Olsr_topology_tuple*);
    virtual void        add_ifaceassoc_tuple(Olsr_iface_assoc_tuple*);
    virtual void        rm_ifaceassoc_tuple(Olsr_iface_assoc_tuple*);
    virtual Olsr_nb_tuple*    find_or_add_nb(Olsr_link_tuple*, uint8_t willingness);

    const nsaddr_t  & get_main_addr(const nsaddr_t&) const;
    virtual int     degree(Olsr_nb_tuple*);

    static bool seq_num_bigger_than(uint16_t, uint16_t);
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void mac_failed(Packet*);
    virtual void    recv(cMessage *p) {}

    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    //virtual void processPromiscuous(const cObject *details){};
    virtual void processLinkBreak(const Packet *) override;

    L3Address getIfaceAddressFromIndex(int index);

    const char * getNodeId(const nsaddr_t &addr);

    void computeDistributionPath(const nsaddr_t &initNode);

  public:
    Olsr() {}
    virtual ~Olsr();


    static double       emf_to_seconds(uint8_t);
    static uint8_t      seconds_to_emf(double);
    static int      node_id(const nsaddr_t&);

    // Routing information access
    virtual bool supportGetRoute() override {return true;}
    virtual uint32_t getRoute(const L3Address &, std::vector<L3Address> &) override;
    virtual bool getNextHop(const L3Address &, L3Address &add, int &iface, double &) override;
    virtual bool isProactive() override;
    virtual void setRefreshRoute(const L3Address &destination, const L3Address & nextHop,bool isReverse) override {}
    virtual bool isOurType(const Packet *) override;
    virtual bool getDestAddress(Packet *, L3Address &) override;
    virtual int getRouteGroup(const AddressGroup &gr, std::vector<L3Address>&) override;
    virtual bool getNextHopGroup(const AddressGroup &gr, L3Address &add, int &iface, L3Address&) override;
    virtual int  getRouteGroup(const L3Address&, std::vector<L3Address> &, L3Address&, bool &, int group = 0) override;
    virtual bool getNextHopGroup(const L3Address&, L3Address &add, int &iface, L3Address&, bool &, int group = 0) override;

    //
    virtual void getDistributionPath(const L3Address&, std::vector<L3Address> &path);

    virtual bool isNodeCandidate(const L3Address&);

    virtual bool handleNodeStart(IDoneCallback *doneCallback) override;
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;
    virtual void handleNodeCrash() override;


    virtual Result ensureRouteForDatagram(Packet *datagram) override {throw cRuntimeError("ensureRouteForDatagram called with OLSR protocol");}

};

} // namespace inetmanet

} // namespace inet

#endif
