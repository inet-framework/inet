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
/// \file   OLSR_ETX.h
/// \brief  Header file for OLSR agent and related classes.
///
/// Here are defined all timers used by OLSR, including those for managing internal
/// state and those for sending messages. Class OLSR is also defined, therefore this
/// file has signatures for the most important methods. Lots of constants are also
/// defined.
///

#ifndef __OLSR_ETX_h__

#define __OLSR_ETX_h__

#include "OLSR.h"
#include "OLSR_ETX_state.h"
#include "OLSR_ETX_repositories.h"
#include "OLSR_ETX_parameter.h"

#define OLSR_ETX_C OLSR_C


/********** Intervals **********/

/// HELLO messages emission interval.
#define OLSR_ETX_HELLO_INTERVAL OLSR_HELLO_INTERVAL

/// TC messages emission interval.
#define OLSR_ETX_TC_INTERVAL OLSR_TC_INTERVAL

/// MID messages emission interval.
#define OLSR_ETX_MID_INTERVA OLSR_MID_INTERVA

///
/// \brief Period at which a node must cite every link and every neighbor.
///
/// We only use this value in order to define OLSR_NEIGHB_HOLD_TIME.
///
#define OLSR_ETX_REFRESH_INTERVAL OLSR_REFRESH_INTERVAL


/********** Holding times **********/

/// Neighbor holding time.
#define OLSR_ETX_NEIGHB_HOLD_TIME   OLSR_NEIGHB_HOLD_TIME
/// Top holding time.
#define OLSR_ETX_TOP_HOLD_TIME  OLSR_TOP_HOLD_TIME
/// Dup holding time.
#define OLSR_ETX_DUP_HOLD_TIME  OLSR_DUP_HOLD_TIME
/// MID holding time.
#define OLSR_ETX_MID_HOLD_TIME  OLSR_MID_HOLD_TIME


/********** Link types **********/

/// Unspecified link type.
#define OLSR_ETX_UNSPEC_LINK    OLSR_UNSPEC_LINK
/// Asymmetric link type.
#define OLSR_ETX_ASYM_LINK      OLSR_ASYM_LINK
/// Symmetric link type.
#define OLSR_ETX_SYM_LINK       OLSR_SYM_LINK
/// Lost link type.
#define OLSR_ETX_LOST_LINK      OLSR_LOST_LINK

/********** Neighbor types **********/

/// Not neighbor type.
#define OLSR_ETX_NOT_NEIGH      OLSR_NOT_NEIGH
/// Symmetric neighbor type.
#define OLSR_ETX_SYM_NEIGH      OLSR_SYM_NEIGH
/// Asymmetric neighbor type.
#define OLSR_ETX_MPR_NEIGH      OLSR_MPR_NEIGH


/********** Willingness **********/

/// Willingness for forwarding packets from other nodes: never.
#define OLSR_ETX_WILL_NEVER     OLSR_WILL_NEVER
/// Willingness for forwarding packets from other nodes: low.
#define OLSR_ETX_WILL_LOW       OLSR_WILL_LOW
/// Willingness for forwarding packets from other nodes: medium.
#define OLSR_ETX_WILL_DEFAULT   OLSR_WILL_DEFAULT
/// Willingness for forwarding packets from other nodes: high.
#define OLSR_ETX_WILL_HIGH      OLSR_WILL_HIGH
/// Willingness for forwarding packets from other nodes: always.
#define OLSR_ETX_WILL_ALWAYS    OLSR_WILL_ALWAYS


/********** Miscellaneous constants **********/

/// Maximum allowed jitter.
#define OLSR_ETX_MAXJITTER      OLSR_MAXJITTER
/// Maximum allowed sequence number.
//#define OLSR_ETX_MAX_SEQ_NUM  OLSR_ETX_MAX_SEQ_NUM
/// Used to set status of an OLSR_nb_tuple as "not symmetric".
#define OLSR_ETX_STATUS_NOT_SYM OLSR_ETX_STATUS_NOT_SYM
/// Used to set status of an OLSR_nb_tuple as "symmetric".
#define OLSR_ETX_STATUS_SYM     OLSR_STATUS_SYM

class OLSR_ETX;         // forward declaration

/// Timer for sending MID messages.
class OLSR_ETX_LinkQualityTimer : public OLSR_Timer
{
  public:
    OLSR_ETX_LinkQualityTimer(OLSR* agent) : OLSR_Timer(agent) {}
    OLSR_ETX_LinkQualityTimer():OLSR_Timer() {}
    virtual void expire();
};



typedef OLSR_msg OLSR_ETX_msg; // OLSR_msg defined in OLSRpkt.msg

///
/// \brief Routing agent which implements %OLSR protocol following RFC 3626.
///
/// Interacts with TCL interface through command() method. It implements all
/// functionalities related to sending and receiving packets and managing
/// internal state.
///
class OLSR_ETX : public OLSR
{


    /// Address of the routing agent.

        friend class OLSR_ETX_LinkQualityTimer;
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
        friend class OLSR_ETX_state;
        friend class Dijkstra;

        OLSR_ETX_parameter parameter_;

        /// Fish Eye State Routing...
#define MAX_TC_MSG_TTL  13
        int tc_msg_ttl_[MAX_TC_MSG_TTL];
        int tc_msg_ttl_index_;

        /// Link delay extension
        long cap_sn_;

        // PortClassifier*  dmux_;      ///< For passing packets up to agents.
        // Trace*       logtarget_; ///< For logging.
        //OLSR_rtable       rtable_;

        /// Internal state with all needed data structs.

        /// A list of pending messages which

        //are buffered awaiting for being sent.
        //std::vector<OLSR_ETX_msg> msgs_;

    protected:

        OLSR_ETX_state *state_etx_ptr;

        OLSR_ETX_LinkQualityTimer *linkQualityTimer;
        /// Link delay extension
        inline long& cap_sn()
        {
            cap_sn_++;
            return cap_sn_;
        }

#define link_quality_timer_  (*linkQualityTimer)

        virtual void recv_olsr(cMessage*);

        // void     mpr_computation();
        // void     rtable_computation();
        virtual void olsr_mpr_computation();
        virtual void olsr_r1_mpr_computation();
        virtual void olsr_r2_mpr_computation();
        virtual void qolsr_mpr_computation();
        virtual void olsrd_mpr_computation();

        virtual void rtable_default_computation();
        virtual void rtable_dijkstra_computation();

        virtual bool process_hello(OLSR_msg&, const nsaddr_t &, const nsaddr_t &, uint16_t, const int &);
        virtual bool process_tc(OLSR_msg&, const nsaddr_t &, const int &);
        // void     process_mid(OLSR_msg&, const nsaddr_t &);

        //void      forward_default(OLSR_msg&, OLSR_dup_tuple*, nsaddr_t,nsaddr_t);
        virtual void forward_data(cMessage* p) {}

//  void        enque_msg(OLSR_msg&, double);
        virtual void send_hello();
        virtual void send_tc();
        //void      send_mid();
        virtual void send_pkt();

        virtual bool link_sensing(OLSR_msg&, const nsaddr_t &, const nsaddr_t &, uint16_t, const int &);
        //void      populate_nbset(OLSR_msg&);
        virtual bool populate_nb2hopset(OLSR_msg&);
        //void      populate_mprselset(OLSR_msg&);

        //void      set_hello_timer();
        //void      set_tc_timer();
        //void      set_mid_timer();

        virtual void nb_loss(OLSR_link_tuple*);

        static bool seq_num_bigger_than(uint16_t, uint16_t);
        NotificationBoard *nb;
        virtual int numInitStages() const { return 5; }
        virtual void initialize(int stage);
        // virtual void receiveChangeNotification(int category, cObject *details);
        // void mac_failed(IPv4Datagram*);
        virtual void recv(cMessage *p) {};
        // virtual void handleMessage(cMessage *msg);
        virtual void finish();
        virtual void link_quality();

    public:
        bool getNextHop(const Uint128 &dest, Uint128 &add, int &iface, double &cost);
        OLSR_ETX() {}
        ~OLSR_ETX();
        static double emf_to_seconds(uint8_t);
        static uint8_t seconds_to_emf(double);
        static int node_id(const nsaddr_t&);

};

#endif
