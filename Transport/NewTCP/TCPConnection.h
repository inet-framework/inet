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
#include "ipsuite_defs.h"
#include "IPAddress.h"
#include "TCPMain.h"


class TCPSegment;
class TCPCommand;
class TCPOpenCommand;
class TCPSendQueue;
class TCPReceiveQueue;
class TCPAlgorithm;


//
// TCP FSM states
//
// Brief descriptions (cf RFC 793, page 20):
//
// LISTEN - waiting for a connection request
// SYN-SENT - part of 3-way handshake (waiting for peer's SYN+ACK or SYN)
// SYN-RECEIVED - part of 3-way handshake (we sent SYN too, waiting for it to be acked)
// ESTABLISHED - normal data transfer
// FIN-WAIT-1 - FIN sent, waiting for its ACK (or peer's FIN)
// FIN-WAIT-2 - our side of the connection closed (our FIN acked), waiting for peer's FIN
// CLOSE-WAIT - FIN received and acked, waiting for local user to close
// LAST-ACK - remote side closed, FIN sent, waiting for its ACK
// CLOSING - simultaneous close: sent FIN, then got peer's FIN
// TIME-WAIT - both FIN's acked, waiting for some time to be sure remote TCP received our ACK
// CLOSED - represents no connection state at all.
//
// Note: FIN-WAIT-1, FIN-WAIT-2, CLOSING, TIME-WAIT represents active close (that is,
// local user closes first), and CLOSE-WAIT and LAST-ACK represents passive close.
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
enum TCPEventCode
{
    TCP_E_IGNORE,

    // app commands
    // (note: no RECEIVE command, data are automatically passed up)
    TCP_E_OPEN_ACTIVE,
    TCP_E_OPEN_PASSIVE,
    TCP_E_SEND,
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

    TCP_E_RCV_UNEXP_SYN,  // unexpected SYN

    // timers
    TCP_E_TIMEOUT_2MSL,     // RFC 793, a.k.a. TIME-WAIT timer
    TCP_E_TIMEOUT_CONN_ESTAB,
    TCP_E_TIMEOUT_FIN_WAIT_2,

    // All other timers (REXMT, PERSIST, DELAYED-ACK, KEEP-ALIVE, etc.),
    // are handled in TCPAlgorithm.
};


/** @name Timeout values */
//@{
#define TCP_TIMEOUT_CONN_ESTAB    75    // 75 seconds
#define TCP_TIMEOUT_FIN_WAIT_2   600    // 10 minutes
#define TCP_TIMEOUT_2MSL         240    // 2 * 2 minutes
//@}


/** @name Comparing sequence numbers */
//@{
inline bool seqLess(uint32 a, uint32 b) {return a!=b && b-a<(1UL<<31);}
inline bool seqLE(uint32 a, uint32 b) {return b-a<(1UL<<31);}
inline bool seqGreater(uint32 a, uint32 b) {return a!=b && a-b<(1UL<<31);}
inline bool seqGE(uint32 a, uint32 b) {return a-b<(1UL<<31);}
//@}


/**
 * Contains state variables ("TCB") for TCP.
 *
 * TCPStateVariables is effectively a "struct" -- it only contains
 * public data members. (Only declared as a class so that we can use
 * cPolymorphic as base class and make it possible to inspect
 * it in Tkenv.)
 *
 * TCPStateVariables only contains variables needed to implement
 * the "base" (RFC 793) TCP. More advanced TCP variants are encapsulated
 * into TCPAlgorithm subclasses which can have their own state blocks,
 * subclassed from TCPStateVariables. See TCPAlgorithm::createStateVariables().
 */
class TCPStateVariables : public cPolymorphic
{
  public:
    TCPStateVariables();
    virtual std::string detailedInfo() const;
  public:
    // set if the connection was initiated by an active open
    bool active;

    // maximum segment size (without headers, i.e. only segment text)
    int snd_mss;

    // send sequence number variables (see RFC 793, "3.2. Terminology")
    uint32 snd_una;      // send unacknowledged
    uint32 snd_nxt;      // send next
    int snd_wnd;         // send window
    int snd_up;          // send urgent pointer
    uint32 snd_wl1;      // segment sequence number used for last window update
    uint32 snd_wl2;      // segment ack. number used for last window update
    uint32 iss;          // initial sequence number (ISS)

    // receive sequence number variables
    uint32 rcv_nxt;      // receive next
    uint32 rcv_wnd;      // receive window
    uint32 rcv_up;       // receive urgent pointer;
    uint32 irs;          // initial receive sequence number

    // whether ACK of our FIN has been received. Needed in FIN bit processing
    // to decide between transition to TIME-WAIT and CLOSING (set event code
    // TCP_E_RCV_FIN or TCP_E_RCV_FIN_ACK).
    bool fin_ack_rcvd;

    bool send_fin;       // true if a user CLOSE command has been "queued" (FIXME probably redundant? state contains this too)
    uint32 snd_fin_seq;  // if send_fin==true: FIN should be sent just before this sequence number

    bool fin_rcvd;       // whether FIN received or not (FIXME probably redundant? state contains this too)
    uint32 rcv_fin_seq;  // if fin_rcvd: sequence number of received FIN (FIXME really needed?)

    //
    // Rest is commented out for now -- probably should go to specific
    // TCPAlgorithm classes.
    //

    //uint32 snd_fin_seq;  // last seq. no.
    // slow start and congestion avoidance variables (RFC 2001)
    //int snd_cwnd;          // congestion window
    //int ssthresh;          // slow start threshold

    // receive variables
    //uint32 rcv_fin_seq;
    //bool rcv_fin_valid;
    //bool rcv_up_valid;
    //uint32 rcv_buf_seq;
    //unsigned long rcv_buff;
    //double  rcv_buf_usage_thresh;

    // retransmit variables
    //uint32 snd_max;      // highest sequence number sent; used to recognize retransmits
    //uint32 max_retrans_seq; // sequence number of a retransmitted segment

    // timing information (round-trip)
    //short t_rtt;                // round-trip time
    //uint32 rtseq;             // starting sequence number of timed data
    //short rttmin;
    //short srtt;                 // smoothed round-trip time
    //short rttvar;               // variance of round-trip time
    //double last_timed_data;     // timestamp for measurement

    // retransmission timeout
    //short rxtcur;
    // backoff for rto
    //short rxtshift;
    //bool rexmt_sch;

    // duplicate ack counter
    //short dupacks;

    // max. ack. delay
    //double max_del_ack;
    // bool to handle delayed ACKs
    //bool ack_sch;

    // number of bits requested by the application
    //long num_pks_req;

    // last time a segment was send
    //double last_snd_time;

    // ACK times
    //double ack_send_time;
    //double ack_rcv_time;

    // bool to handle the time wait timer
    //bool time_wait_sch;
    //bool finwait2_sch;
};



/**
 * Manages a TCP connection. This class itself implements the TCP state
 * machine as well as handling control PDUs (SYN, SYN+ACK, RST, FIN, etc.),
 * timers (2MSL, CONN-ESTAB, FIN-WAIT-2) and events (OPEN, SEND, etc)
 * associated with TCP state changes.
 *
 * It delegates all details of the TCP data transfer (in ESTABLISHED,
 * etc. state) to a TCPAlgorithm subclass. This covers handling timers
 * such as REXMIT, PERSIST, KEEPALIVE, DELAYED-ACK; window management,
 * congestion control schemes, SACK, etc. The concrete TCPAlgorithm
 * class to use can be chosen per connection (in OPEN) or in a module
 * parameter. Delegation to TCPAlgorithm facilitates experimenting with
 * various TCP features.
 *
 * TCPConnection also delegates the handling of send and receive buffers
 * to instances of TCPSendQueue and TCPReceiveQueue subclasses. This
 * makes it possible to easily accomodate need for various types of
 * simulated data transfer: transmitting a real byte stream, a "dummy"
 * transfer (no actual bytes transmitted in the model, only octet counts),
 * and transmitting a sequence of cMessage objects (where every message
 * object is mapped to a TCP sequence number range).
 *
 * TCPConnections are not used alone -- they are instantiated and managed by
 * a TCPMain module.
 *
 * Remarks: there's no implicit RECEIVE command expected from applications --
 * received data are automatically and immediately passed up to the app.
 */
class TCPConnection
{
  public:
    // connection identification by apps: appgateIndex+connId
    int appGateIndex; // application gate index
    int connId;

    // socket pair
    IPAddress localAddr;
    IPAddress remoteAddr;
    int localPort;
    int remotePort;

  protected:
    TCPMain *tcpMain;  // TCP module

    // TCP state machine
    cFSM fsm;

    // variables associated with TCP state
    TCPStateVariables *state;

    // TCP queues
    TCPSendQueue *sendQueue;
    TCPReceiveQueue *receiveQueue;

    // TCP behavior is data transfer state
    TCPAlgorithm *tcpAlgorithm;

    // timers
    cMessage *the2MSLTimer;
    cMessage *connEstabTimer;
    cMessage *finWait2Timer;

  protected:
    /** @name FSM transitions: analysing events and executing state transitions */
    //@{
    TCPEventCode preanalyseAppCommandEvent(int commandCode);
    bool performStateTransition(const TCPEventCode& event);
    //@}

    /** @name Processing app commands (may override event code) */
    //@{
    void process_OPEN_ACTIVE(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    void process_OPEN_PASSIVE(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    void process_SEND(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    void process_CLOSE(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    void process_ABORT(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    void process_STATUS(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    //@}

    /** @name Processing TCP segment arrivals.
     * Event code TCP_E_SEGMENT gets replaced by specific ones (e.g. TCP_E_RCV_SYN) here.
     */
    //@{
    bool tryFastRoute(TCPSegment *tcpseg); // shortcut most common case
    TCPEventCode process_RCV_SEGMENT(TCPSegment *tcpseg, IPAddress src, IPAddress dest);

    TCPEventCode processSegmentInListen(TCPSegment *tcpseg, IPAddress src, IPAddress dest);
    TCPEventCode processSegmentInSynSent(TCPSegment *tcpseg, IPAddress src, IPAddress dest);
    TCPEventCode processSegment1stThru8th(TCPSegment *tcpseg);
    TCPEventCode processRstInSynReceived(TCPSegment *tcpseg);
    void processAckInEstabEtc(TCPSegment *tcpseg);
    void processUrgInEstabEtc(TCPSegment *tcpseg);
    void processSegmentTextInEstabEtc(TCPSegment *tcpseg);
    void doConnectionReset();
    //@}

    /** @name Processing timeouts (may override event code) */
    //@{
    void process_TIMEOUT_2MSL();
    void process_TIMEOUT_CONN_ESTAB();
    void process_TIMEOUT_FIN_WAIT_2();
    //@}

    /** Utility: creates send/receive queues and tcpAlgorithm */
    void initConnection(TCPOpenCommand *openCmd);

    /** Utility: generates ISS and initializes corresponding state variables */
    void selectInitialSeqNum();

    /** check if segment is acceptable (seq num is in valid range) */
    bool checkSegmentSeqNum(TCPSegment *tcpseg);

    /** Utility: send SYN, SYN+ACK, ACK */
    void sendSyn();
    void sendSynAck();
  public:
    void sendAck();

    /** Utility: send data from sendQueue, at most this many segments (-1 means no limit) */
    void sendData(int maxNumSegments=-1);

    /** Utility: sends RST */
    void sendRst(uint32 seqNo);
    static void sendRst(uint32 seq, IPAddress src, IPAddress dest, int srcPort, int destPort);
    static void sendRstAck(uint32 seq, uint32 ack, IPAddress src, IPAddress dest, int srcPort, int destPort);

    /** Utility: sends FIN */
    void sendFin();

    /** Utility: adds control info to segment and sends it to IP */
  public:
    void sendToIP(TCPSegment *tcpseg);
  protected:
    static void sendToIP(TCPSegment *tcpseg, IPAddress src, IPAddress dest);

    /** Utility: sends packet to application */
    void sendToApp(cMessage *msg);

    /** Utility: start a timer */
    void scheduleTimeout(cMessage *msg, simtime_t timeout) {
        tcpMain->scheduleAt(tcpMain->simTime()+timeout, msg);
    }


  protected:
    static void printSegmentBrief(TCPSegment *tcpseg);
    static const char *stateName(int state);
    static const char *eventName(int event);

  public:
    /**
     * Static function, invoked from TCPMain when a segment arrives which
     * doesn't belong to an existing connection.
     */
    static void segmentArrivalWhileClosed(TCPSegment *tcpseg, IPAddress src, IPAddress dest);

    /**
     * Ctor.
     */
    TCPConnection(TCPMain *mod, int appGateIndex, int connId);

    /**
     * Dtor.
     */
    ~TCPConnection();

    /* @name Various getters */
    //@{
    int getFsmState() {return fsm.state();}
    TCPStateVariables *getState() {return state;}
    TCPSendQueue *getSendQueue() {return sendQueue;}
    TCPReceiveQueue *getReceiveQueue() {return receiveQueue;}
    TCPAlgorithm *getTcpAlgorithm() {return tcpAlgorithm;}
    //@}

    /**
     * Process self-messages (timers).
     * Normally returns true. A return value of false means that the
     * connection structure must be deleted by the caller (TCPMain).
     */
    bool processTimer(cMessage *msg);

    /**
     * Process incoming TCP segment. Normally returns true. A return value
     * of false means that the connection structure must be deleted by the
     * caller (TCPMain).
     */
    bool processTCPSegment(TCPSegment *tcpSeg, IPAddress srcAddr, IPAddress destAddr);

    /**
     * Process commands from the application.
     * Normally returns true. A return value of false means that the
     * connection structure must be deleted by the caller (TCPMain).
     */
    bool processAppCommand(cMessage *msg);
};

#endif


