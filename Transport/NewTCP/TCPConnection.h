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
class IPInterfacePacket;


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
typedef int TCPEvent;

//
// State variables for TCP
//
struct TCPStateVariables
{
    // number of bits requested by a client TCP
    unsigned long num_bit_req;

    // send sequence number variables (RFC 793)
    unsigned long snd_una;      // send unacknowledged
    unsigned long snd_nxt;      // send next
    unsigned long savenext;     // save-variable for snd_nxt in fast rexmt
    unsigned long snd_up;       // send urgent pointer
    unsigned long snd_wnd;      // send window
    unsigned long snd_wl1;      // segment sequence number used for last window update
    unsigned long snd_wl2;      // segment ack. number used for last window update
    unsigned long iss;          // initial sequence number (ISS)

    unsigned long snd_fin_seq;  // last seq. no.
    int snd_fin_valid;          // FIN flag set?
    int snd_up_valid;           // urgent pointer valid/URG flag set?
    unsigned long snd_mss;      // maximum segment size

    // slow start and congestion avoidance variables (RFC 2001)
    unsigned long snd_cwnd;     // congestion window
    unsigned long ssthresh;     // slow start threshold
    int cwnd_cnt;

    // receive sequence number variables
    unsigned long rcv_nxt;      // receive next
    unsigned long rcv_wnd;      // receive window
    unsigned long rcv_wnd_last; // last receive window
    unsigned long rcv_up;       // receive urgent pointer;
    unsigned long irs;          // initial receive sequence number

    // receive variables
    unsigned long rcv_fin_seq;
    int rcv_fin_valid;
    int rcv_up_valid;
    unsigned long rcv_buf_seq;
    unsigned long rcv_buff;
    double  rcv_buf_usage_thresh;

    // retransmit variables
    unsigned long snd_max;      // highest sequence number sent; used to recognize retransmits
    unsigned long max_retrans_seq; // sequence number of a retransmitted segment

    // segment variables
    unsigned long seg_len;      // segment length
    unsigned long seg_seq;      // segment sequence number
    unsigned long seg_ack;      // segment acknoledgement number

    // timing information (round-trip)
    short t_rtt;                // round-trip time
    unsigned long rtseq;        // starting sequence number of timed data
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

    // timers
    //...
  protected:
    /**
     * Utility: encapsulates segment into IPInterfacePacket and sends it to IP.
     */
    void sendToIP(TCPSegment *tcpseg);

    /**
     * Utility: sends packet to application.
     */
    void sendToApp(TCPInterfacePacket *tcpIfPacket);

    /** @name Processing events in different states -- 12 states times 3 kinds of event */
    //@{
    void processTCPSegmentInInit(TCPSegment *tcpseg);
    void processTimerInInit(cMessage *event);
    void processAppCommandInInit(TCPInterfacePacket *tcpIfPacket);

    void processTCPSegmentInClosed(TCPSegment *tcpseg);
    void processTimerInClosed(cMessage *event);
    void processAppCommandInClosed(TCPInterfacePacket *tcpIfPacket);

    void processTCPSegmentInListen(TCPSegment *tcpseg);
    void processTimerInListen(cMessage *event);
    void processAppCommandInListen(TCPInterfacePacket *tcpIfPacket);

    void processTCPSegmentInSynRcvd(TCPSegment *tcpseg);
    void processTimerInSynRcvd(cMessage *event);
    void processAppCommandInSynRcvd(TCPInterfacePacket *tcpIfPacket);

    void processTCPSegmentInSynSent(TCPSegment *tcpseg);
    void processTimerInSynSent(cMessage *event);
    void processAppCommandInSynSent(TCPInterfacePacket *tcpIfPacket);

    void processTCPSegmentInEstablished(TCPSegment *tcpseg);
    void processTimerInEstablished(cMessage *event);
    void processAppCommandInEstablished(TCPInterfacePacket *tcpIfPacket);

    void processTCPSegmentInCloseWait(TCPSegment *tcpseg);
    void processTimerInCloseWait(cMessage *event);
    void processAppCommandInCloseWait(TCPInterfacePacket *tcpIfPacket);

    void processTCPSegmentInLastAck(TCPSegment *tcpseg);
    void processTimerInLastAck(cMessage *event);
    void processAppCommandInLastAck(TCPInterfacePacket *tcpIfPacket);

    void processTCPSegmentInFinWait1(TCPSegment *tcpseg);
    void processTimerInFinWait1(cMessage *event);
    void processAppCommandInFinWait1(TCPInterfacePacket *tcpIfPacket);

    void processTCPSegmentInFinWait2(TCPSegment *tcpseg);
    void processTimerInFinWait2(cMessage *event);
    void processAppCommandInFinWait2(TCPInterfacePacket *tcpIfPacket);

    void processTCPSegmentInClosing(TCPSegment *tcpseg);
    void processTimerInClosing(cMessage *event);
    void processAppCommandInClosing(TCPInterfacePacket *tcpIfPacket);

    void processTCPSegmentInTimeWait(TCPSegment *tcpseg);
    void processTimerInTimeWait(cMessage *event);
    void processAppCommandInTimeWait(TCPInterfacePacket *tcpIfPacket);
    //@}

    /** @name FSM transitions: analysing events and executing state transitions */
    //@{
    TCPEvent analyseTCPSegmentEvent(TCPSegment *tcpseg);
    TCPEvent analyseTimerEvent(cMessage *msg);
    TCPEvent analyseAppCommandEvent(TCPInterfacePacket *tcpIfPacket);
    bool performStateTransition(const TCPEvent& event);
    //@}

  public:
    /**
     * Ctor.
     */
    TCPConnection(int connId, cSimpleModule *mod);

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


