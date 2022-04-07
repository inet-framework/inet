//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPCONNECTION_H
#define __INET_TCPCONNECTION_H

#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpConnectionState_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {

class TcpCommand;
class TcpOpenCommand;

namespace tcp {

class TcpSackRexmitQueue;
class TcpAlgorithm;

/** @name Timeout values */
//@{
#define TCP_TIMEOUT_CONN_ESTAB        75  // 75 seconds
#define TCP_TIMEOUT_FIN_WAIT_2        600  // 10 minutes
#define TCP_TIMEOUT_SYN_REXMIT        3  // initially 3 seconds
#define TCP_TIMEOUT_SYN_REXMIT_MAX    240  // 4 mins (will only be used with SYN+ACK: with SYN CONN_ESTAB occurs sooner)
//@}

#define MAX_SYN_REXMIT_COUNT          12  // will only be used with SYN+ACK: with SYN CONN_ESTAB occurs sooner
#define TCP_MAX_WIN                   65535  // 65535 bytes, largest value (16 bit) for (unscaled) window size
#define TCP_MAX_WIN_SCALED            0x3fffffffL // 2^30-1 bytes, largest value for scaled window size
#define MAX_SACK_BLOCKS               60  // will only be used with SACK
#define PAWS_IDLE_TIME_THRESH         (24 * 24 * 3600)  // 24 days in seconds (RFC 1323)

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
 * TcpConnection objects are not used alone -- they are instantiated and managed
 * by a TCP module.
 *
 * TcpConnection "outsources" several tasks to objects subclassed from
 * TcpSendQueue, TcpReceiveQueue and TcpAlgorithm; see overview of this
 * with TCP documentation.
 *
 * Connection variables (TCB) are kept in TcpStateVariables. TcpAlgorithm
 * implementations can extend TcpStateVariables to add their own stuff
 * (see TcpAlgorithm::createStateVariables() factory method.)
 *
 * The "entry points" of TCPConnnection from TCP are:
 *  - processTimer(cMessage *msg): handle self-messages which belong to the connection
 *  - processTCPSegment(TcpHeader *tcpSeg, Address srcAddr, Address destAddr):
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
 *  -# after they return, we'll know the state machine event (TcpEventCode,
 *     TCP_E_xxx) for sure, so we can:
 *  -# invoke performStateTransition() which executes the necessary state
 *     transition (for example, TCP_E_RCV_SYN will take the state machine
 *     from TCP_S_LISTEN to TCP_S_SYN_RCVD). No other actions are taken
 *     in this step.
 *  -# if there was a state change (for example, we entered the
 *     TCP_S_ESTABLISHED state), performStateTransition() invokes stateEntered(),
 *     which performs some necessary housekeeping (cancel the CONN-ESTAB timer).
 *
 * When the CLOSED state is reached, TCP will delete the TcpConnection object.
 *
 */
class INET_API TcpConnection : public cSimpleModule
{
  public:
    static simsignal_t tcpConnectionAddedSignal;
    static simsignal_t stateSignal; // FSM state
    static simsignal_t sndWndSignal; // snd_wnd
    static simsignal_t rcvWndSignal; // rcv_wnd
    static simsignal_t rcvAdvSignal; // current advertised window (=rcv_adv)
    static simsignal_t sndNxtSignal; // sent seqNo
    static simsignal_t sndAckSignal; // sent ackNo
    static simsignal_t rcvSeqSignal; // received seqNo
    static simsignal_t rcvAckSignal; // received ackNo (=snd_una)
    static simsignal_t unackedSignal; // number of bytes unacknowledged
    static simsignal_t dupAcksSignal; // current number of received dupAcks
    static simsignal_t pipeSignal; // current sender's estimate of bytes outstanding in the network
    static simsignal_t sndSacksSignal; // number of sent Sacks
    static simsignal_t rcvSacksSignal; // number of received Sacks
    static simsignal_t rcvOooSegSignal; // number of received out-of-order segments
    static simsignal_t rcvNASegSignal; // number of received not acceptable segments
    static simsignal_t sackedBytesSignal; // current number of received sacked bytes
    static simsignal_t tcpRcvQueueBytesSignal; // current amount of used bytes in tcp receive queue
    static simsignal_t tcpRcvQueueDropsSignal; // number of drops in tcp receive queue
    static simsignal_t tcpRcvPayloadBytesSignal; // amount of payload bytes received (including duplicates, out of order etc) for TCP throughput

    // connection identification by apps: socketId
    int socketId = -1; // identifies connection within the app
    int getSocketId() const { return socketId; }
    void setSocketId(int newSocketId) { ASSERT(socketId == -1); socketId = newSocketId; }

    int listeningSocketId = -1; // identifies listening connection within the app
    int getListeningSocketId() const { return listeningSocketId; }

    // socket pair
    L3Address localAddr;
    const L3Address& getLocalAddr() const { return localAddr; }
    L3Address remoteAddr;
    const L3Address& getRemoteAddr() const { return remoteAddr; }
    int localPort = -1;
    int remotePort = -1;

    // TCP options for this connection
    int ttl = -1;
    short dscp = -1;
    short tos = -1;

  protected:
    Tcp *tcpMain = nullptr; // Tcp module

    // TCP state machine
    cFSM fsm;

    // variables associated with TCP state
    TcpStateVariables *state = nullptr;

    // TCP queues
    TcpSendQueue *sendQueue = nullptr;
    TcpSendQueue *getSendQueue() const { return sendQueue; }
    TcpReceiveQueue *receiveQueue = nullptr;
    TcpReceiveQueue *getReceiveQueue() const { return receiveQueue; }

  public:
    TcpSackRexmitQueue *rexmitQueue = nullptr;
    TcpSackRexmitQueue *getRexmitQueue() const { return rexmitQueue; }

  protected:
    // TCP behavior in data transfer state
    TcpAlgorithm *tcpAlgorithm = nullptr;
    TcpAlgorithm *getTcpAlgorithm() const { return tcpAlgorithm; }

    // timers
    cMessage *the2MSLTimer = nullptr;
    cMessage *connEstabTimer = nullptr;
    cMessage *finWait2Timer = nullptr;
    cMessage *synRexmitTimer = nullptr; // for retransmitting SYN and SYN+ACK

  protected:
    /** @name FSM transitions: analysing events and executing state transitions */
    //@{
    /** Maps app command codes (msg kind of app command msgs) to TCP_E_xxx event codes */
    virtual TcpEventCode preanalyseAppCommandEvent(int commandCode);
    /** Implemements the pure TCP state machine */
    virtual bool performStateTransition(const TcpEventCode& event);
    /** Perform cleanup necessary when entering a new state, e.g. cancelling timers */
    virtual void stateEntered(int state, int oldState, TcpEventCode event);
    //@}

    /** @name Processing app commands. Invoked from processAppCommand(). */
    //@{
    virtual void process_OPEN_ACTIVE(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg);
    virtual void process_OPEN_PASSIVE(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg);
    virtual void process_ACCEPT(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg);
    virtual void process_SEND(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg);
    virtual void process_CLOSE(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg);
    virtual void process_ABORT(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg);
    virtual void process_DESTROY(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg);
    virtual void process_STATUS(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg);
    virtual void process_QUEUE_BYTES_LIMIT(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg);
    virtual void process_READ_REQUEST(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg);
    virtual void process_OPTIONS(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg);
    //@}

    /** @name Processing TCP segment arrivals. Invoked from processTCPSegment(). */
    //@{
    /**
     * Shortcut to process most common case as fast as possible. Returns false
     * if segment requires normal (slow) route.
     */
    virtual bool tryFastRoute(const Ptr<const TcpHeader>& tcpHeader);
    /**
     * Process incoming TCP segment. Returns a specific event code (e.g. TCP_E_RCV_SYN)
     * which will drive the state machine.
     */
    virtual TcpEventCode process_RCV_SEGMENT(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address src, L3Address dest);
    virtual TcpEventCode processSegmentInListen(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address src, L3Address dest);
    virtual TcpEventCode processSynInListen(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address srcAddr, L3Address destAddr);
    virtual TcpEventCode processSegmentInSynSent(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address src, L3Address dest);
    virtual TcpEventCode processSegment1stThru8th(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader);
    virtual TcpEventCode processRstInSynReceived(const Ptr<const TcpHeader>& tcpHeader);
    virtual bool processAckInEstabEtc(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader);
    //@}

    /** @name Processing of TCP options. Invoked from readHeaderOptions(). Return value indicates whether the option was valid. */
    //@{
    virtual bool processMSSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionMaxSegmentSize& option);
    virtual bool processWSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionWindowScale& option);
    virtual bool processSACKPermittedOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionSackPermitted& option);
    virtual bool processSACKOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionSack& option);
    virtual bool processTSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionTimestamp& option);
    //@}

    /** @name Processing timeouts. Invoked from processTimer(). */
    //@{
    virtual void process_TIMEOUT_2MSL();
    virtual void process_TIMEOUT_CONN_ESTAB();
    virtual void process_TIMEOUT_FIN_WAIT_2();
    virtual void process_TIMEOUT_SYN_REXMIT(TcpEventCode& event);
    //@}

    /** Utility: clone a listening connection. Used for forking. */
    virtual TcpConnection *cloneListeningConnection();

    virtual void initClonedConnection(TcpConnection *listenerConn);

    /** Utility: creates send/receive queues and tcpAlgorithm */
    virtual void initConnection(TcpOpenCommand *openCmd);

    /** Utility: set snd_mss, rcv_wnd and sack in newly created state variables block */
    virtual void configureStateVariables();

    /** Utility: generates ISS and initializes corresponding state variables */
    virtual void selectInitialSeqNum();

    /** Utility: check if segment is acceptable (all bytes are in receive window) */
    virtual bool isSegmentAcceptable(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader) const;

    /** Utility: send SYN */
    virtual void sendSyn();

    /** Utility: send SYN+ACK */
    virtual void sendSynAck();

    /** Utility: readHeaderOptions (Currently only EOL, NOP, MSS, WS, SACK_PERMITTED, SACK and TS are implemented) */
    virtual void readHeaderOptions(const Ptr<const TcpHeader>& tcpHeader);

    /** Utility: writeHeaderOptions (Currently only EOL, NOP, MSS, WS, SACK_PERMITTED, SACK and TS are implemented) */
    virtual TcpHeader writeHeaderOptions(const Ptr<TcpHeader>& tcpHeader);

    /** Utility: adds SACKs to segments header options field */
    virtual TcpHeader addSacks(const Ptr<TcpHeader>& tcpHeader);

    /** Utility: get TSval from segments TS header option */
    virtual uint32_t getTSval(const Ptr<const TcpHeader>& tcpHeader) const;

    /** Utility: get TSecr from segments TS header option */
    virtual uint32_t getTSecr(const Ptr<const TcpHeader>& tcpHeader) const;

    /** Utility: returns true if the connection is not yet accepted by the application */
    virtual bool isToBeAccepted() const { return listeningSocketId != -1; }

  public:
    /** Utility: send ACK */
    virtual void sendAck();

    /**
     * Utility: Send data from sendQueue, at most congestionWindow.
     */
    virtual bool sendData(uint32_t congestionWindow);

    /** Utility: sends 1 bytes as "probe", called by the "persist" mechanism */
    virtual bool sendProbe();

    /** Utility: retransmit one segment from snd_una */
    virtual void retransmitOneSegment(bool called_at_rto);

    /** Utility: retransmit all from snd_una to snd_max */
    virtual void retransmitData();

    /** Utility: sends RST */
    virtual void sendRst(uint32_t seqNo);
    /** Utility: sends RST; does not use connection state */
    virtual void sendRst(uint32_t seq, L3Address src, L3Address dest, int srcPort, int destPort);
    /** Utility: sends RST+ACK; does not use connection state */
    virtual void sendRstAck(uint32_t seq, uint32_t ack, L3Address src, L3Address dest, int srcPort, int destPort);

    /** Utility: sends FIN */
    virtual void sendFin();

    /**
     * Utility: sends one segment of 'bytes' bytes from snd_nxt, and advances snd_nxt.
     * sendData(), sendProbe() and retransmitData() internally all rely on this one.
     * Returns the number of bytes sent.
     */
    virtual uint32_t sendSegment(uint32_t bytes);

    /** Utility: adds control info to segment and sends it to IP */
    virtual void sendToIP(Packet *tcpSegment, const Ptr<TcpHeader>& tcpHeader);

    /** Utility: start SYN-REXMIT timer */
    virtual void startSynRexmitTimer();

    /** Utility: signal to user that connection timed out */
    virtual void signalConnectionTimeout();

  protected:
    /** Utility: send IP packet */
    virtual void sendToIP(Packet *pkt, const Ptr<TcpHeader>& tcpHeader, L3Address src, L3Address dest);

    /** Utility: sends packet to application */
    virtual void sendToApp(cMessage *msg);

    /** Utility: sends status indication (TCP_I_xxx) to application */
    virtual void sendIndicationToApp(int code, const int id = 0);

    /** Utility: sends TCP_I_AVAILABLE indication with TcpAvailableInfo to application */
    virtual void sendAvailableIndicationToApp();

    /** Utility: sends TCP_I_ESTABLISHED indication with TcpConnectInfo to application */
    virtual void sendEstabIndicationToApp();

    /** Utility: sends data or data notification to application */
    virtual void sendAvailableDataToApp();

  public:
    /** Utility: prints local/remote addr/port and app gate index/socketId */
    virtual void printConnBrief() const;
    /** Utility: prints important header fields */
    static void printSegmentBrief(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader);
    /** Utility: returns name of TCP_S_xxx constants */
    static const char *stateName(int state);
    /** Utility: returns name of TCP_E_xxx constants */
    static const char *eventName(int event);
    /** Utility: returns name of TCP_I_xxx constants */
    static const char *indicationName(int code);
    /** Utility: returns name of TCPOPTION_xxx constants */
    static const char *optionName(int option);
    /** Utility: update receiver queue related variables and statistics - called before setting rcv_wnd */
    virtual void updateRcvQueueVars();

    /** Utility: returns true when receive queue has enough space for store the tcpHeader */
    virtual bool hasEnoughSpaceForSegmentInReceiveQueue(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader);

    /** Utility: update receive window (rcv_wnd), and calculate scaled value if window scaling enabled.
     *  Returns the (scaled) receive window size.
     */
    virtual unsigned short updateRcvWnd();

    /** Utility: update window information (snd_wnd, snd_wl1, snd_wl2) */
    virtual void updateWndInfo(const Ptr<const TcpHeader>& tcpHeader, bool doAlways = false);

  public:
    TcpConnection() {}
    TcpConnection(const TcpConnection& other) {} // FIXME kludge
    void initialize() {}

    /**
     * The "normal" constructor.
     */
    void initConnection(Tcp *mod, int socketId);

    /**
     * Destructor.
     */
    virtual ~TcpConnection();

    int getLocalPort() const { return localPort; }
    L3Address getLocalAddress() const { return localAddr; }

    int getRemotePort() const { return remotePort; }
    L3Address getRemoteAddress() const { return remoteAddr; }

    /**
     * This method gets invoked from TCP when a segment arrives which
     * doesn't belong to an existing connection. TCP creates a temporary
     * connection object so that it can call this method, then immediately
     * deletes it.
     */
    virtual void segmentArrivalWhileClosed(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address src, L3Address dest);

    /** @name Various getters **/
    //@{
    int getFsmState() const { return fsm.getState(); }
    const TcpStateVariables *getState() const { return state; }
    TcpStateVariables *getState() { return state; }
    TcpSendQueue *getSendQueue() { return sendQueue; }
    TcpSackRexmitQueue *getRexmitQueue() { return rexmitQueue; }
    TcpReceiveQueue *getReceiveQueue() { return receiveQueue; }
    TcpAlgorithm *getTcpAlgorithm() { return tcpAlgorithm; }
    Tcp *getTcpMain() { return tcpMain; }
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
    virtual bool processTCPSegment(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address srcAddr, L3Address destAddr);

    /**
     * Process commands from the application.
     * Normally returns true. A return value of false means that the
     * connection structure must be deleted by the caller (TCP).
     */
    virtual bool processAppCommand(cMessage *msg);

    virtual void handleMessage(cMessage *msg);

    /**
     * For SACK TCP. RFC 3517, page 3: "This routine returns whether the given
     * sequence number is considered to be lost.  The routine returns true when
     * either DupThresh discontiguous SACKed sequences have arrived above
     * 'SeqNum' or (DupThresh * SMSS) bytes with sequence numbers greater
     * than 'SeqNum' have been SACKed.  Otherwise, the routine returns
     * false."
     */
    virtual bool isLost(uint32_t seqNum);

    /**
     * For SACK TCP. RFC 3517, page 3: "This routine traverses the sequence
     * space from HighACK to HighData and MUST set the "pipe" variable to an
     * estimate of the number of octets that are currently in transit between
     * the TCP sender and the TCP receiver."
     */
    virtual void setPipe();

    /**
     * For SACK TCP. RFC 3517, page 3: "This routine uses the scoreboard data
     * structure maintained by the Update() function to determine what to transmit
     * based on the SACK information that has arrived from the data receiver
     * (and hence been marked in the scoreboard).  NextSeg () MUST return the
     * sequence number range of the next segment that is to be
     * transmitted..."
     * Returns true if a valid sequence number (for the next segment) is found and
     * returns false if no segment should be send.
     */
    virtual bool nextSeg(uint32_t& seqNum);

    /**
     * Utility: send data during Loss Recovery phase (if SACK is enabled).
     */
    virtual void sendDataDuringLossRecoveryPhase(uint32_t congestionWindow);

    /**
     * Utility: send segment during Loss Recovery phase (if SACK is enabled).
     * Returns the number of bytes sent.
     */
    virtual uint32_t sendSegmentDuringLossRecoveryPhase(uint32_t seqNum);

    /**
     * Utility: send one new segment from snd_max if allowed (RFC 3042).
     */
    virtual void sendOneNewSegment(bool fullSegmentsOnly, uint32_t congestionWindow);

    /**
     * Utility: converts a given simtime to a timestamp (TS).
     */
    static uint32_t convertSimtimeToTS(simtime_t simtime);

    /**
     * Utility: converts a given timestamp (TS) to a simtime.
     */
    static simtime_t convertTSToSimtime(uint32_t timestamp);

    /**
     * Utility: checks if send queue is empty (no data to send).
     */
    virtual bool isSendQueueEmpty();
};

} // namespace tcp

} // namespace inet

#endif

