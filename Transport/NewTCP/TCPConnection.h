//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __TCPCONNECTION_H
#define __TCPCONNECTION_H

#include <omnetpp.h>
#include "IPAddress.h"


class TCPSegment;
class TCPInterfacePacket;
class TCPOpenCommand;
class IPInterfacePacket;
class TCPSendQueue;
class TCPReceiveQueue;
class TCPAlgorithm;

/**
 * TCP sequence number
 */
typedef unsigned long tcpseq_t;


/**
 * Shorthand for unsigned long
 */
typedef unsigned long ulong;


//
// TCP FSM states
//
enum TcpState
{
    TCP_S_INIT        = 0,
    TCP_S_CLOSED      = FSM_Steady(1),
    TCP_S_LISTEN      = FSM_Steady(2),
    TCP_S_SYN_SENT    = FSM_Steady(3),
    TCP_S_SYN_RCVD    = FSM_Steady(4),
    TCP_S_ESTABLISHED = FSM_Steady(5),
    TCP_S_CLOSE_WAIT  = FSM_Steady(6),
    TCP_S_LAST_ACK    = FSM_Steady(7),
    TCP_S_FIN_WAIT_1  = FSM_Steady(8),
    TCP_S_FIN_WAIT_2  = FSM_Steady(9),
    TCP_S_CLOSING     = FSM_Steady(10),
    TCP_S_TIME_WAIT   = FSM_Steady(11)
};


//
// Event, strictly for the FSM state transition purposes.
// DO NOT USE outside performStateTransition()!
//
enum TCPEvent
{
    // app commands
    TCP_E_OPEN_ACTIVE,
    TCP_E_OPEN_PASSIVE,
    TCP_E_SEND,
    TCP_E_RECEIVE,
    TCP_E_CLOSE,
    TCP_E_ABORT,
    TCP_E_STATUS,

    // TPDU types
    TCP_E_RCV_DATA,
    TCP_E_RCV_ACK,
    TCP_E_RCV_SYN,
    TCP_E_RCV_SYN_ACK,
    TCP_E_RCV_FIN,
    TCP_E_RCV_FIN_ACK,
    TCP_E_RCV_RST,  // covers RST+ACK too

    // timers
    TCP_E_TIMEOUT_TIME_WAIT,    // rfc793
    TCP_E_TIMEOUT_REXMT,        // rfc793
    TCP_E_TIMEOUT_PERSIST,      //???
    TCP_E_TIMEOUT_KEEPALIVE,    //???
    TCP_E_TIMEOUT_CONN_ESTAB,   //???
    TCP_E_TIMEOUT_FIN_WAIT_2,   //???
    TCP_E_TIMEOUT_DELAYED_ACK,  //???
};



//
// State variables for TCP
//
struct TCPStateVariables
{
    // set if the connection was initiated by an active open
    bool active;

    // number of bits requested by a client TCP
    unsigned long num_bit_req;

    // send sequence number variables (RFC 793)
    tcpseq_t snd_una;      // send unacknowledged
    tcpseq_t snd_nxt;      // send next

    int snd_up;            // send urgent pointer
    int snd_wnd;           // send window

    tcpseq_t savenext;     // save-variable for snd_nxt in fast rexmt
    tcpseq_t snd_wl1;      // segment sequence number used for last window update
    tcpseq_t snd_wl2;      // segment ack. number used for last window update
    tcpseq_t iss;          // initial sequence number (ISS)

    tcpseq_t snd_fin_seq;  // last seq. no.
    int snd_fin_valid;     // FIN flag set?
    int snd_up_valid;      // urgent pointer valid/URG flag set?
    int snd_mss;           // maximum segment size

    // slow start and congestion avoidance variables (RFC 2001)
    int snd_cwnd;          // congestion window
    int ssthresh;          // slow start threshold
    int cwnd_cnt;

    // receive sequence number variables
    tcpseq_t rcv_nxt;      // receive next
    tcpseq_t rcv_wnd;      // receive window
    tcpseq_t rcv_wnd_last; // last receive window
    tcpseq_t rcv_up;       // receive urgent pointer;
    tcpseq_t irs;          // initial receive sequence number

    // receive variables
    tcpseq_t rcv_fin_seq;
    bool rcv_fin_valid;
    bool rcv_up_valid;
    tcpseq_t rcv_buf_seq;
    unsigned long rcv_buff; //???
    double  rcv_buf_usage_thresh;

    // retransmit variables
    tcpseq_t snd_max;      // highest sequence number sent; used to recognize retransmits
    tcpseq_t max_retrans_seq; // sequence number of a retransmitted segment

    // segment variables
    tcpseq_t seg_len;      // segment length
    tcpseq_t seg_seq;      // segment sequence number
    tcpseq_t seg_ack;      // segment acknoledgement number

    // timing information (round-trip)
    short t_rtt;                // round-trip time
    tcpseq_t rtseq;             // starting sequence number of timed data
    short rttmin;
    short srtt;                 // smoothed round-trip time
    short rttvar;               // variance of round-trip time
    double last_timed_data;     // timestamp for measurement

    // retransmission timeout
    short rxtcur;
    // backoff for rto
    short rxtshift;
    bool rexmt_sch;

    // duplicate ack counter
    short dupacks;

    // max. ack. delay
    double max_del_ack;
    // bool to handle delayed ACKs
    bool ack_sch;

    // number of bits requested by the application
    long num_pks_req;

    // last time a segment was send
    double last_snd_time;

    // application notification variable
    bool tcp_app_notified;

    // ACK times
    double ack_send_time;
    double ack_rcv_time;

    // bool to handle the time wait timer
    bool time_wait_sch;
    bool finwait2_sch;
};



/**
 * Implements a TCP connection. The class itself manages only the TCP
 * state machine. Data transfer in the ESTAB state is left to
 * the class ... (FIXME)
 */
class TCPConnection
{
  protected:
    int connId; // connection id
    cSimpleModule *module;  // TCP module (TCPMain)

    // socket pair
    IPAddress localAddr;
    IPAddress remoteAddr;
    int localPort;
    int remotePort;

    // TCP state machine
    cFSM fsm;

    // variables associated with TCP state
    TCPStateVariables state;

    // TCP queues
    TCPSendQueue *sendQueue;
    TCPReceiveQueue *receiveQueue;

    // TCP behavior is data transfer state
    TCPAlgorithm *tcpAlgorithm;

    // timers
    //...

  protected:
    /** @name FSM transitions: analysing events and executing state transitions */
    //@{
    TCPEvent analyseTCPSegmentEvent(TCPSegment *tcpseg);
    TCPEvent analyseTimerEvent(cMessage *msg);
    TCPEvent analyseAppCommandEvent(TCPInterfacePacket *tcpIfPacket);
    bool performStateTransition(const TCPEvent& event);
    //@}

    /** @name Processing app commands */
    //@{
    void process_OPEN_ACTIVE(TCPInterfacePacket *tcpIfPacket);
    void process_OPEN_PASSIVE(TCPInterfacePacket *tcpIfPacket);
    void process_SEND(TCPInterfacePacket *tcpIfPacket);
    void process_RECEIVE(TCPInterfacePacket *tcpIfPacket);
    void process_CLOSE(TCPInterfacePacket *tcpIfPacket);
    void process_ABORT(TCPInterfacePacket *tcpIfPacket);
    void process_STATUS(TCPInterfacePacket *tcpIfPacket);
    //@}

    /** @name Processing TCP segment arrivals */
    //@{
    void process_RCV_DATA(TCPSegment *tcpseg);
    void process_RCV_SYN(TCPSegment *tcpseg);
    void process_RCV_SYN_ACK(TCPSegment *tcpseg);
    void process_RCV_FIN(TCPSegment *tcpseg);
    void process_RCV_FIN_ACK(TCPSegment *tcpseg);
    void process_RCV_RST(TCPSegment *tcpseg);
    //@}

    /** @name Processing timeouts */
    //@{
    void process_TIMEOUT_TIME_WAIT(cMessage *msg);
    void process_TIMEOUT_REXMT(cMessage *msg);
    void process_TIMEOUT_PERSIST(cMessage *msg);
    void process_TIMEOUT_KEEPALIVE(cMessage *msg);
    void process_TIMEOUT_CONN_ESTAB(cMessage *msg);
    void process_TIMEOUT_FIN_WAIT_2(cMessage *msg);
    void process_TIMEOUT_DELAYED_ACK(cMessage *msg);
    //@}

    /** @name Comparing sequence numbers */
    //@{
    bool seqNoLt(tcpseq_t a, tcpseq_t b) {return a!=b && b-a<(1UL<<31);}
    bool seqNoLeq(tcpseq_t a, tcpseq_t b) {return b-a<(1UL<<31);}
    bool seqNoGt(tcpseq_t a, tcpseq_t b) {return a!=b && a-b<(1UL<<31);}
    bool seqNoGeq(tcpseq_t a, tcpseq_t b) {return a-b<(1UL<<31);}
    //@}

    /** Utility: creates send/receive queues and tcpAlgorithm */
    void initConnection(TCPOpenCommand *openCmd);

    /** Utility: generates ISS and sends initial SYN */
    void sendSyn();

    /** Utility: encapsulates segment into IPInterfacePacket and sends it to IP */
    void sendToIP(TCPSegment *tcpseg);

    /** Utility: sends packet to application */
    void sendToApp(TCPInterfacePacket *tcpIfPacket);

  public:
    /**
     * Ctor.
     */
    TCPConnection(int connId, cSimpleModule *mod);

    /**
     * Dtor.
     */
    ~TCPConnection();

    /**
     * Process self-messages (timers).
     * Normally returns true. A return value of false means that the
     * connection structure must be deleted by the caller (TCPMain).
     */
    bool processTimer(cMessage *msg);

    /**
     * Process incoming TCP segments, wrapped in IPInterfacePacket.
     * Normally returns true. A return value of false means that the
     * connection structure must be deleted by the caller (TCPMain).
     */
    bool processTCPSegment(IPInterfacePacket *wrappedTcpSeg);

    /**
     * Process commands from the application.
     * Normally returns true. A return value of false means that the
     * connection structure must be deleted by the caller (TCPMain).
     */
    bool processAppCommand(TCPInterfacePacket *tcpIfPacket);
};

#endif


