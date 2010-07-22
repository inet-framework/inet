//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_TCPCONNECTION_OLD_H
#define __INET_TCPCONNECTION_OLD_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "IPvXAddress.h"
#include "TCP_old.h"

class TCPSegment;
class TCPCommand;
class TCPOpenCommand;

namespace tcp_old {
using namespace tcp_old;

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
#define TCP_TIMEOUT_SYN_REXMIT     3    // initially 3 seconds
#define TCP_TIMEOUT_SYN_REXMIT_MAX 240  // 4 mins (will only be used with SYN+ACK: with SYN CONN_ESTAB occurs sooner)
//@}

#define MAX_SYN_REXMIT_COUNT     12     // will only be used with SYN+ACK: with SYN CONN_ESTAB occurs sooner

#define TCP_MAX_WIN            65535    // largest value (16 bit) for (unscaled) window size

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
class INET_API TCPStateVariables : public cPolymorphic
{
  public:
    TCPStateVariables();
    virtual std::string info() const;
    virtual std::string detailedInfo() const;
  public:
    bool active;         // set if the connection was initiated by an active open
    bool fork;           // if passive and in LISTEN: whether to fork on an incoming connection

    uint snd_mss;        // maximum segment size (without headers, i.e. only segment text)

    // send sequence number variables (see RFC 793, "3.2. Terminology")
    uint32 snd_una;      // send unacknowledged
    uint32 snd_nxt;      // send next (drops back on retransmission)
    uint32 snd_max;      // max seq number sent (needed because snd_nxt is re-set on retransmission)

    uint snd_wnd;        // send window
    uint32 snd_up;       // send urgent pointer
    uint32 snd_wl1;      // segment sequence number used for last window update
    uint32 snd_wl2;      // segment ack. number used for last window update
    uint32 iss;          // initial sequence number (ISS)

    // receive sequence number variables
    uint32 rcv_nxt;      // receive next
    uint32 rcv_wnd;      // receive window
    uint32 rcv_up;       // receive urgent pointer;
    uint32 irs;          // initial receive sequence number

    // number of consecutive duplicate ACKs (this counter would logically
    // belong to TCPAlgorithm, but it's a lot easier to manage here)
    short dupacks;

    // SYN, SYN+ACK retransmission variables (handled separately
    // because normal rexmit belongs to TCPAlgorithm)
    int syn_rexmit_count;   // number of SYN/SYN+ACK retransmissions (=1 after first rexmit)
    simtime_t syn_rexmit_timeout; // current SYN/SYN+ACK retransmission timeout

    // whether ACK of our FIN has been received. Needed in FIN bit processing
    // to decide between transition to TIME-WAIT and CLOSING (set event code
    // TCP_E_RCV_FIN or TCP_E_RCV_FIN_ACK).
    bool fin_ack_rcvd;

    bool send_fin;       // true if a user CLOSE command has been "queued"
    uint32 snd_fin_seq;  // if send_fin==true: FIN should be sent just before this sequence number

    bool fin_rcvd;       // whether FIN received or not
    uint32 rcv_fin_seq;  // if fin_rcvd: sequence number of received FIN

    bool afterRto;       // set when the retransmission timer expires, reset when snd_nxt == snd_max or snd_una == snd_max

    uint32 last_ack_sent;// RFC 1323, page 31: "Last ACK field sent"

    //bool rcv_up_valid;
    //uint32 rcv_buf_seq;
    //unsigned long rcv_buff;
    //double  rcv_buf_usage_thresh;
};



/**
 * Manages a TCP connection. This class itself implements the TCP state
 * machine as well as handling control PDUs (SYN, SYN+ACK, RST, FIN, etc.),
 * timers (2MSL, CONN-ESTAB, FIN-WAIT-2) and events (OPEN, SEND, etc)
 * associated with TCP state changes.
 *
 * The implementation largely follows the functional specification at the end
 * of RFC 793. Code comments extensively quote RFC 793 to make it easier
 * to understand.
 *
 * TCPConnection objects are not used alone -- they are instantiated and managed
 * by a TCP module.
 *
 * TCPConnection "outsources" several tasks to objects subclassed from
 * TCPSendQueue, TCPReceiveQueue and TCPAlgorithm; see overview of this
 * with TCP documentation.
 *
 * Connection variables (TCB) are kept in TCPStateVariables. TCPAlgorithm
 * implementations can extend TCPStateVariables to add their own stuff
 * (see TCPAlgorithm::createStateVariables() factory method.)
 *
 * The "entry points" of TCPConnnection from TCP are:
 *  - processTimer(cMessage *msg): handle self-messages which belong to the connection
 *  - processTCPSegment(TCPSegment *tcpSeg, IPvXAddress srcAddr, IPvXAddress destAddr):
 *    handle segment arrivals
 *  - processAppCommand(cMessage *msg): process commands which arrive from the
 *    application (TCP_C_xxx)
 *
 * All three methods follow a common structure:
 *
 *  -# dispatch to specific methods. For example, processAppCommand() invokes
 *     one of process_OPEN_ACTIVE(), process_OPEN_PASSIVE() or process_SEND(),
 *     etc., and processTCPSegment() dispatches to processSegmentInListen(),
 *     processSegmentInSynSent() or processSegment1stThru8th().
 *     Those methods will do the REAL JOB.
 *  -# after they return, we'll know the state machine event (TCPEventCode,
 *     TCP_E_xxx) for sure, so we can:
 *  -# invoke performStateTransition() which executes the necessary state
 *     transition (for example, TCP_E_RCV_SYN will take the state machine
 *     from TCP_S_LISTEN to TCP_S_SYN_RCVD). No other actions are taken
 *     in this step.
 *  -# if there was a state change (for example, we entered the
 *     TCP_S_ESTABLISHED state), performStateTransition() invokes stateEntered(),
 *     which performs some necessary housekeeping (cancel the CONN-ESTAB timer).
 *
 * When the CLOSED state is reached, TCP will delete the TCPConnection object.
 *
 */
class INET_API TCPConnection
{
  public:
    // connection identification by apps: appgateIndex+connId
    int appGateIndex; // application gate index
    int connId;       // identifies connection within the app

    // socket pair
    IPvXAddress localAddr;
    IPvXAddress remoteAddr;
    int localPort;
    int remotePort;

  protected:
    TCP *tcpMain;  // TCP module

    // TCP state machine
    cFSM fsm;

    // variables associated with TCP state
    TCPStateVariables *state;

    // TCP queues
    TCPSendQueue *sendQueue;
    TCPReceiveQueue *receiveQueue;

    // TCP behavior in data transfer state
    TCPAlgorithm *tcpAlgorithm;

    // timers
    cMessage *the2MSLTimer;
    cMessage *connEstabTimer;
    cMessage *finWait2Timer;
    cMessage *synRexmitTimer;  // for retransmitting SYN and SYN+ACK

    // statistics
    cOutVector *sndWndVector;   // snd_wnd
    cOutVector *sndNxtVector;   // sent seqNo
    cOutVector *sndAckVector;   // sent ackNo
    cOutVector *rcvSeqVector;   // received seqNo
    cOutVector *rcvAckVector;   // received ackNo (= snd_una)
    cOutVector *unackedVector;  // number of bytes unacknowledged

  protected:
    /** @name FSM transitions: analysing events and executing state transitions */
    //@{
    /** Maps app command codes (msg kind of app command msgs) to TCP_E_xxx event codes */
    virtual TCPEventCode preanalyseAppCommandEvent(int commandCode);
    /** Implemements the pure TCP state machine */
    virtual bool performStateTransition(const TCPEventCode& event);
    /** Perform cleanup necessary when entering a new state, e.g. cancelling timers */
    virtual void stateEntered(int state);
    //@}

    /** @name Processing app commands. Invoked from processAppCommand(). */
    //@{
    virtual void process_OPEN_ACTIVE(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    virtual void process_OPEN_PASSIVE(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    virtual void process_SEND(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    virtual void process_CLOSE(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    virtual void process_ABORT(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    virtual void process_STATUS(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg);
    //@}

    /** @name Processing TCP segment arrivals. Invoked from processTCPSegment(). */
    //@{
    /**
     * Shortcut to process most common case as fast as possible. Returns false
     * if segment requires normal (slow) route.
     */
    virtual bool tryFastRoute(TCPSegment *tcpseg);
    /**
     * Process incoming TCP segment. Returns a specific event code (e.g. TCP_E_RCV_SYN)
     * which will drive the state machine.
     */
    virtual TCPEventCode process_RCV_SEGMENT(TCPSegment *tcpseg, IPvXAddress src, IPvXAddress dest);
    virtual TCPEventCode processSegmentInListen(TCPSegment *tcpseg, IPvXAddress src, IPvXAddress dest);
    virtual TCPEventCode processSegmentInSynSent(TCPSegment *tcpseg, IPvXAddress src, IPvXAddress dest);
    virtual TCPEventCode processSegment1stThru8th(TCPSegment *tcpseg);
    virtual TCPEventCode processRstInSynReceived(TCPSegment *tcpseg);
    virtual bool processAckInEstabEtc(TCPSegment *tcpseg);
    //@}

    /** @name Processing timeouts. Invoked from processTimer(). */
    //@{
    virtual void process_TIMEOUT_2MSL();
    virtual void process_TIMEOUT_CONN_ESTAB();
    virtual void process_TIMEOUT_FIN_WAIT_2();
    virtual void process_TIMEOUT_SYN_REXMIT(TCPEventCode& event);
    //@}

    /** Utility: clone a listening connection. Used for forking. */
    virtual TCPConnection *cloneListeningConnection();

    /** Utility: creates send/receive queues and tcpAlgorithm */
    virtual void initConnection(TCPOpenCommand *openCmd);

    /** Utility: set snd_mss and rcv_wnd in newly created state variables block */
    virtual void configureStateVariables();

    /** Utility: generates ISS and initializes corresponding state variables */
    virtual void selectInitialSeqNum();

    /** Utility: check if segment is acceptable (all bytes are in receive window) */
    virtual bool isSegmentAcceptable(TCPSegment *tcpseg);

    /** Utility: send SYN */
    virtual void sendSyn();

    /** Utility: send SYN+ACK */
    virtual void sendSynAck();

  public:
    /** Utility: send ACK */
    virtual void sendAck();

    /**
     * Utility: Send data from sendQueue, at most congestionWindow (-1 means no limit).  FIXME adjust comment!!!
     * If fullSegmentsOnly is set, don't send segments smaller than MSS (needed for Nagle).
     * Returns true if some data was actually sent.
     */
    virtual bool sendData(bool fullSegmentsOnly, int congestionWindow=-1);

    /** Utility: sends 1 bytes as "probe", called by the "persist" mechanism */
    virtual bool sendProbe();

    /** Utility: retransmit one segment from snd_una */
    virtual void retransmitOneSegment(bool called_at_rto);

    /** Utility: retransmit all from snd_una to snd_max */
    virtual void retransmitData();

    /** Utility: sends RST */
    virtual void sendRst(uint32 seqNo);
    /** Utility: sends RST; does not use connection state */
    virtual void sendRst(uint32 seq, IPvXAddress src, IPvXAddress dest, int srcPort, int destPort);
    /** Utility: sends RST+ACK; does not use connection state */
    virtual void sendRstAck(uint32 seq, uint32 ack, IPvXAddress src, IPvXAddress dest, int srcPort, int destPort);

    /** Utility: sends FIN */
    virtual void sendFin();

    /**
     * Utility: sends one segment of 'bytes' bytes from snd_nxt, and advances snd_nxt.
     * sendData(), sendProbe() and retransmitData() internally all rely on this one.
     */
    virtual void sendSegment(uint32 bytes);

    /** Utility: adds control info to segment and sends it to IP */
    virtual void sendToIP(TCPSegment *tcpseg);

    /**
     * Utility: This factory method gets invoked throughout the TCP model to
     * create a TCPSegment. Override it if you need to subclass TCPSegment.
     */
    virtual TCPSegment *createTCPSegment(const char *name);

    /** Utility: start SYN-REXMIT timer */
    virtual void startSynRexmitTimer();

    /** Utility: signal to user that connection timed out */
    virtual void signalConnectionTimeout();

    /** Utility: start a timer */
    void scheduleTimeout(cMessage *msg, simtime_t timeout)
        {tcpMain->scheduleAt(simTime()+timeout, msg);}

  protected:
    /** Utility: cancel a timer */
    cMessage *cancelEvent(cMessage *msg)  {return tcpMain->cancelEvent(msg);}

    /** Utility: send IP packet */
    static void sendToIP(TCPSegment *tcpseg, IPvXAddress src, IPvXAddress dest);

    /** Utility: sends packet to application */
    virtual void sendToApp(cMessage *msg);

    /** Utility: sends status indication (TCP_I_xxx) to application */
    virtual void sendIndicationToApp(int code);

    /** Utility: sends TCP_I_ESTABLISHED indication with TCPConnectInfo to application */
    virtual void sendEstabIndicationToApp();

  public:
    /** Utility: prints local/remote addr/port and app gate index/connId */
    virtual void printConnBrief();
    /** Utility: prints important header fields */
    static void printSegmentBrief(TCPSegment *tcpseg);
    /** Utility: returns name of TCP_S_xxx constants */
    static const char *stateName(int state);
    /** Utility: returns name of TCP_E_xxx constants */
    static const char *eventName(int event);
    /** Utility: returns name of TCP_I_xxx constants */
    static const char *indicationName(int code);

  public:
    /**
     * The "normal" constructor.
     */
    TCPConnection(TCP *mod, int appGateIndex, int connId);

    /**
     * Note: this default ctor is NOT used to create live connections, only
     * temporary ones so that TCPMain can invoke their segmentArrivalWhileClosed().
     */
    TCPConnection();

    /**
     * Destructor.
     */
    virtual ~TCPConnection();

    /**
     * This method gets invoked from TCP when a segment arrives which
     * doesn't belong to an existing connection. TCP creates a temporary
     * connection object so that it can call this method, then immediately
     * deletes it.
     */
    virtual void segmentArrivalWhileClosed(TCPSegment *tcpseg, IPvXAddress src, IPvXAddress dest);

    /* @name Various getters */
    //@{
    int getFsmState() const {return fsm.getState();}
    TCPStateVariables *getState() {return state;}
    TCPSendQueue *getSendQueue() {return sendQueue;}
    TCPReceiveQueue *getReceiveQueue() {return receiveQueue;}
    TCPAlgorithm *getTcpAlgorithm() {return tcpAlgorithm;}
    TCP *getTcpMain() {return tcpMain;}
    //@}

    /**
     * Process self-messages (timers).
     * Normally returns true. A return value of false means that the
     * connection structure must be deleted by the caller (TCP).
     */
    virtual bool processTimer(cMessage *msg);

    /**
     * Process incoming TCP segment. Normally returns true. A return value
     * of false means that the connection structure must be deleted by the
     * caller (TCP).
     */
    virtual bool processTCPSegment(TCPSegment *tcpSeg, IPvXAddress srcAddr, IPvXAddress destAddr);

    /**
     * Process commands from the application.
     * Normally returns true. A return value of false means that the
     * connection structure must be deleted by the caller (TCP).
     */
    virtual bool processAppCommand(cMessage *msg);
};

}
#endif


