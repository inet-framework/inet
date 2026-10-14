//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPCONNECTION_H
#define __INET_TCPCONNECTION_H

#include <climits>
#include <deque>
#include <map>

#include "inet/common/SimpleModule.h"
#include "inet/networklayer/common/IcmpType_m.h"
#include "inet/networklayer/common/Icmpv6Type_m.h"
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
//@}

// the values of state->ecnMode, from the tcpEcnMode parameter
#define TCP_ECN_MODE_OFF              0
#define TCP_ECN_MODE_PASSIVE          1
#define TCP_ECN_MODE_RFC3168          2
#define TCP_ECN_MODE_ACCECN           3
#define TCP_ECN_MODE_ACCECN_PASSIVE   4
#define TFO_BLACKHOLE_REXMIT_COUNT    2   // TCP Fast Open: the retransmission of a Fast Open SYN that suggests a blackhole (the third transmission, as Linux)
#define TCP_MAX_WIN                   65535lu  // 65535 bytes, largest value (16 bit) for (unscaled) window size
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
class INET_API TcpConnection : public SimpleModule
{
  protected:
    static simsignal_t sndNxtSignal; // sent seqNo
    // connection identification by apps: socketId
    int socketId = -1; // identifies connection within the app

    int listeningSocketId = -1; // identifies listening connection within the app

    // socket pair
    L3Address localAddr;
    L3Address remoteAddr;
    int localPort = -1;
    int remotePort = -1;

    // TCP options for this connection
    int ttl = -1;
    short dscp = -1;
    short tos = -1;
    bool autoRead = true;
    bool peerClosedSentUp = false;
    long maxByteCountRequested = 0;  // from READ requests

    Tcp *tcpMain = nullptr; // Tcp module

    // TCP state machine
    cFSM fsm;

    // variables associated with TCP state
    TcpStateVariables *state = nullptr;

    // TCP queues
    TcpSendQueue *sendQueue = nullptr;
    TcpReceiveQueue *receiveQueue = nullptr;
    TcpSackRexmitQueue *rexmitQueue = nullptr;

    // TCP behavior in data transfer state
    TcpAlgorithm *tcpAlgorithm = nullptr;

    // timers
    cMessage *the2MSLTimer = nullptr;
    cMessage *connEstabTimer = nullptr;
    cMessage *finWait2Timer = nullptr;
    cMessage *synRexmitTimer = nullptr; // for retransmitting SYN and SYN+ACK
    cMessage *rackReoTimer = nullptr; // RACK reordering timer (Linux ICSK_TIME_REO_TIMEOUT): fires when a not-yet-lost segment's RACK.rtt+reo_wnd deadline matures between ACKs

    bool sendingWindowProbe = false; // sendProbe() sends a window probe, which gets no ECT and no CWR (RFC 3168 section 6.1.6)

    // statistics
    long rcvdSegments = 0;
    long sentSegments = 0;
    uint32_t lastRcvdSeqNo = 0;
    uint32_t lastSentAck = 0;

    // The receive buffer in the accounting of Linux, for windowShrinkAllowed: each
    // received data segment costs the truesize of its skb (sk_rmem_alloc), until the
    // application reads all of it, and the window counts the free buffer at the ratio
    // of payload to truesize (scaling_ratio of tcp_measure_rcv_mss())
    std::deque<std::pair<uint32_t, uint32_t>> rcvSkbChain; // the unread received segments: payload bytes and truesize
    uint64_t rcvBufOccupancy = 0; // the sum of the truesizes in rcvSkbChain
    uint8_t rcvScalingRatio = 128; // payload / truesize, in units of 1/256 (Linux TCP_DEFAULT_SCALING_RATIO)
    uint32_t rcvMssEstimate = 536; // the estimate of the received MSS that gates the ratio (Linux icsk_ack.rcv_mss)
    // the advertised window and the receive buffer of this connection from its open
    // command (TcpSocket::setAdvertisedWindow(), setReceiveBufferSize()); -1: the
    // parameters of the Tcp module (D-4 of the staged landing: a socket option is
    // not a module parameter)
    int socketAdvertisedWindow = -1;
    int socketReceiveBufferSize = -1;
    int socketSynRetries = -1; // the SYN retries of this connection (TcpSocket::setSynRetries()); -1: the parameter
    // MSG_ZEROCOPY: the pending completions, by the sequence number that ends the data
    // of each zero-copy SEND, and the id of the next one (per connection from 0, as the
    // ids of Linux SO_ZEROCOPY)
    std::map<uint32_t, uint32_t> zerocopySeqNums;
    uint32_t nextZerocopyId = 0;
    // MSG_EOR: the record boundaries, that is the sequence numbers that end the data of
    // each SEND with TcpSendEorReq, in sequence order. No segment spans a boundary.
    std::deque<uint32_t> eorSeqNums;
    // pushSegmentsOnWriteBoundary: the sequence numbers that end a segment with PSH, in
    // sequence order: the end of each write without MSG_MORE and of the data of a Fast
    // Open SYN (pushSeqNums), and the marks of forced_push() (forcedPushSeqNums)
    std::deque<uint32_t> pushSeqNums;
    std::deque<uint32_t> forcedPushSeqNums;
    // SO_TIMESTAMPING: the delivered data carries the time of its delivery
    // (TcpRxTimestampInd), as TcpSetTimestampingCommand asks; a member, because the
    // command can come before the open
    bool rxTimestampingEnabled = false;
    // TCP_NOTSENT_LOWAT from TcpSetNotsentLowatCommand, which can come before the open;
    // INT_MIN: no command, so the notsentLowat parameter
    int notsentLowatSockopt = INT_MIN;
    // the application writer waits for space in the send buffer (Linux SOCK_NOSPACE), as
    // TcpSetWriterBlockedCommand tells; it drives the send buffer limited time
    bool writerBlocked = false;
    // the application owns the connection (Linux sk->sk_socket): false for a connection
    // that the application did not accept yet, as TcpSetOwnedCommand tells
    bool appOwned = true;
    // an accepted segment ends beyond the right edge of the last offered window
    // (acceptBeyondBufferAtEmptyQueue); adaptiveDelayedAcks then sends no ACK at once
    bool overWindowAcceptPending = false;
    // SO_KEEPALIVE of this connection (TcpSocket::setKeepAlive()), also from before the
    // open; -1: the keepAliveEnabled parameter
    int keepAliveSockopt = -1;
    // TCP_NODELAY of this connection (TcpSocket::setNoDelay()), also from before the
    // open; -1: the nagleEnabled parameter
    int noDelaySockopt = -1;
    // TCP_MAXSEG of this connection (TcpSocket::setMaxSeg()), also from before the open;
    // -1: the mss parameter alone
    int maxSegSockopt = -1;
    // mss = -1: snd_mss takes the MSS of the address family when the peer is known
    // (resolveAddressFamilyMss())
    bool mssFromAddressFamily = false;
    // the route MTU of this connection (TcpSocket::setPathMtu()), also from before the
    // open; 0: the pathMtu parameter
    int pathMtuSockopt = 0;
    // TCP_CORK of this connection (TcpSocket::setCork()), also from before the open; -1: off
    int corkSockopt = -1;
    int rcvBufSockopt = -1; // SO_RCVBUF of the open connection (TcpSetRcvBufCommand); -1: none

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
    /** RFC 9293 MUST-15: without an MSS option in the SYN of the peer, use the default send MSS of its address family. */
    virtual void applyDefaultSendMss(const Ptr<const TcpHeader>& tcpHeader, const L3Address& peerAddr);
    virtual bool processWSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionWindowScale& option);
    virtual bool processSACKPermittedOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionSackPermitted& option);
    virtual bool processTSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionTimestamp& option);
    virtual bool processFastOpenOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionTcpFastOpen& option);
    virtual bool processFastOpenExpOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionTcpFastOpenExp& option);
    //@}

    /** TCP Fast Open (RFC 7413): processes the cookie of a Fast Open option of a SYN or a SYN-ACK. */
    virtual bool processFastOpenCookieBytes(const std::vector<uint8_t>& cookie);

    /** @name Processing timeouts. Invoked from processTimer(). */
    //@{
    virtual void process_TIMEOUT_2MSL();
    virtual void process_TIMEOUT_CONN_ESTAB();
    virtual void process_TIMEOUT_FIN_WAIT_2();
    virtual void process_TIMEOUT_SYN_REXMIT(TcpEventCode& event);

    /**
     * rackReoTimer expired: re-run RACK loss detection (time has advanced, so
     * pending deadlines may have matured) and let the recovery strategy react
     * (enter fast recovery / retransmit) via ITcpRecovery::reoTimeout().
     */
    virtual void processRackReoTimeout();
    //@}

  public:
    /**
     * (Re)arms the RACK reordering timer for the given delay, or cancels it when
     * delay is negative. Called by the recovery strategy from RACK loss detection.
     */
    virtual void rescheduleRackReoTimer(simtime_t delay);

    /**
     * Maps INET's independent loss-recovery bools onto Linux's tcp_ca_state
     * ordinals (TCP_CA_Open=0, Disorder=1, CWR=2, Recovery=3, Loss=4), for
     * TcpStatusInfo::caState.
     */
    virtual int deriveLinuxCaState() const;

    /**
     * The sequence number from which the advertised window and the bytes in flight
     * count the data: snd_una, but the first data byte while a TCP Fast Open
     * server sends data in SYN_RCVD. Its SYN-ACK is then not acknowledged, and
     * snd_una stays at the ISS, but the SYN-ACK is not data (Linux starts the
     * child socket at snd_una = ISS+1).
     */
    virtual uint32_t getDataSndUna() const;

    /**
     * Returns whether the last accepted segment ended beyond the right edge of the
     * last offered window, and clears that mark. Linux sends no immediate ACK for
     * such a segment (rcv_neg_window).
     */
    bool takeOverWindowAccept() { bool b = overWindowAcceptPending; overWindowAcceptPending = false; return b; }

    /**
     * Sends the partial segment that TCP_CORK or MSG_MORE holds: at an uncork, at
     * TCP_NODELAY, and at the cork timer, which also asks for PSH (forcePush).
     */
    virtual void flushCorkedData(bool forcePush);

    /**
     * MSG_EOR: the last record boundary after snd_nxt and before the given sequence
     * number, or snd_nxt if there is none.
     */
    virtual uint32_t getLastRecordBoundaryBefore(uint32_t seq) const;

    /** MSG_ZEROCOPY: notifies the application of each zero-copy SEND whose data went out */
    virtual void notifyZerocopyCompletions();

    /**
     * pushSegmentsOnWriteBoundary: the number of segments that Linux gives the NIC as
     * one GSO burst (tcp_write_xmit(), tcp_tso_autosize()), at the given congestion
     * window and bytes in flight.
     */
    virtual uint32_t getGsoBurstSegments(uint32_t congestionWindow, uint32_t bytesInFlight, uint32_t effectiveMss) const;

  protected:

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

    /** Utility: sets TS.Recent from an acceptable segment (RFC 7323 section 5.3, R3) */
    virtual void updateTsRecent(const Ptr<const TcpHeader>& tcpHeader);

    /** Utility: the TS option of the segment, or nullptr */
    virtual const TcpOptionTimestamp *findTSOption(const Ptr<const TcpHeader>& tcpHeader) const;

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

    /**
     * Makes the octet of a zero-window probe that the receiver did not accept unsent
     * data again: snd_max and snd_nxt go back to snd_una, and the retransmission queue
     * forgets the octet. Only in the persist state, where the probe octet alone is in
     * flight.
     */
    virtual void unsendWindowProbe();

    /** Utility: sends a keepalive probe, a segment without data with SEG.SEQ = SND.UNA - 1, which the peer answers with an ACK (RFC 9293 section 3.8.4) */
    virtual void sendKeepAliveProbe();

    /** Utility: sends a segment without data with SEG.SEQ = SND.UNA - 1, which the peer answers with an ACK (Linux tcp_xmit_probe_skb()): the keepalive probe, and the zero-window probe of Linux */
    virtual void sendSegmentBelowWindow(const char *name);

    /** Utility: retransmit one segment from snd_una */
    virtual void retransmitOneSegment(bool called_at_rto);

    /**
     * RFC 8985 section 7.3 loss probe: send one segment of new data if the data
     * and the receive window allow it, else the FIN again, else the last segment
     * again. Returns true if a segment was sent.
     */
    virtual bool sendTlpProbe();

    /**
     * Called at a retransmission timeout: marks the outstanding data lost in the
     * retransmission queue, as Linux tcp_timeout_mark_lost() does. Without SACK,
     * it also removes the inferred SACKs and the retransmitted marks.
     */
    virtual void markOutstandingLostOnRto();

    /** Utility: retransmit all from snd_una to snd_max */
    virtual void retransmitData();

    /** Utility: sends RST */
    virtual void sendRst(uint32_t seqNo);
    /** Utility: sends RST; does not use connection state */
    virtual void sendRst(uint32_t seq, L3Address src, L3Address dest, int srcPort, int destPort, bool withTimestamp = false);

    /** Utility: sends the RST of a user abort, with the TS option if it is enabled (RFC 7323 section 5.2) */
    virtual void sendAbortRst();
    /** Utility: sends RST+ACK; does not use connection state */
    virtual void sendRstAck(uint32_t seq, uint32_t ack, L3Address src, L3Address dest, int srcPort, int destPort, bool withTimestamp = false);

    /** Utility: sends FIN */
    virtual void sendFin();

    /**
     * Utility: sends one segment of 'bytes' bytes from snd_nxt, and advances snd_nxt.
     * sendData(), sendProbe() and retransmitData() internally all rely on this one.
     * Returns the number of bytes sent.
     */
    virtual uint32_t sendSegment(uint32_t bytes);

    /** Puts the data of a SEND command into the send queue, and starts the busy time of TCP_INFO. */
    virtual void enqueueSendCommandData(Packet *packet);

    /** Utility: adds control info to segment and sends it to IP */
    virtual void sendToIP(Packet *tcpSegment, const Ptr<TcpHeader>& tcpHeader);

    /** AccECN: the ACE value that feeds back an IP ECN codepoint in the handshake (RFC 9768 Tables 2 and 3) */
    static uint8_t accEcnReflectedAce(int ipEcnCodepoint);

    /** Utility: the IP ECN codepoint of a received segment, IP_ECN_NOT_ECT without one */
    static int receivedEcnCodepoint(Packet *tcpSegment);

    /** AccECN: reads the ACE field of an ACK that makes progress, and counts the CE marks that it reports (RFC 9768 section 3.2.2.5) */
    virtual void processAccEcnAce(const Ptr<const TcpHeader>& tcpHeader);
    /** AccECN receiver: counts the CE mark and the payload bytes of a received segment (RFC 9768 section 3.2). */
    virtual void countAccEcnReceived(Packet *tcpSegment, int payloadLength);
    /**
     * AccECN (RFC 9768 section 3.2.3): whether the next segment after the handshake
     * carries the AccECN option, and with which fields. order: 0 (kind 172) or 1
     * (kind 174); wanted: the number of fields to send if they fit; required: the
     * fewest fields to send, those that changed since the last option.
     */
    virtual bool getAccEcnOptionFields(int& order, int& wanted, int& required) const;
    /** Appends the AccECN option with the fields that fit in the free option space. */
    virtual void appendAccEcnOption(const Ptr<TcpHeader>& tcpHeader, int order, int wanted, int required);
    /**
     * AccECN: the option bytes in use, for the fit of the AccECN option: the real
     * length, or with accEcnOptionPolicy="linux" the length of the Linux layout
     * (NOP, NOP, TS and NOP, NOP, SACK).
     */
    virtual unsigned int getAccEcnOptionSpaceUsed(const Ptr<TcpHeader>& tcpHeader) const;
    /** AccECN: whether an option of the given number of fields fits after the used option bytes. */
    virtual bool accEcnOptionFits(unsigned int used, int numFields, bool sack) const;

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

    /**
     * The truesize of the skb of a received TCP segment in Linux, from a tun device as
     * packetdrill uses: the payload, the IP and TCP headers and skb_shared_info (320
     * bytes) in a page-sized or power-of-two allocation, and struct sk_buff (232 bytes).
     */
    static uint32_t linuxSkbTruesize(uint32_t payloadBytes);
    /**
     * The time that the send buffer limits the sender (Linux TCP_CHRONO_SNDBUF_LIMITED):
     * it runs while the application writer is blocked, no unsent data waits, and data
     * is in flight. Called at each event that can change this condition.
     */
    virtual void updateSndbufLimitedChrono();
    /** Charges the receive buffer with a received data segment (rcvSkbChain). */
    virtual void chargeRcvBuffer(uint32_t payloadBytes);
    /** Releases the receive buffer for the data that the application read. */
    virtual void releaseRcvBuffer(uint64_t readBytes);

    /** Utility: returns true when receive queue has enough space for store the tcpHeader */
    virtual bool hasEnoughSpaceForSegmentInReceiveQueue(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader);

    /** Utility: the window that the receiver can offer now, from its free receive buffer,
     *  before the rule that the window does not shrink (Linux __tcp_select_window()).
     *  It changes no state.
     */
    virtual uint32_t selectRcvWnd();

    /** Utility: update receive window (rcv_wnd), and calculate scaled value if window scaling enabled.
     *  Returns the (scaled) receive window size.
     */
    virtual uint16_t updateRcvWnd();

    /** Utility: update window information (snd_wnd, snd_wl1, snd_wl2) */
    /**
     * FlightSize for the ssthresh of fast retransmit (RFC 5681 equation (4)): the data
     * that was sent but is not cumulatively acknowledged. Step 2 of RFC 5681 section
     * 3.2 leaves out the data that Limited Transmit sent on the duplicate ACKs.
     */
    virtual uint32_t getFlightSize() const;

    virtual void updateWndInfo(const Ptr<const TcpHeader>& tcpHeader, bool doAlways = false);

    /**
     * The MSS that congestion control counts with: snd_mss less the space of the
     * TCP options that every segment of the established connection carries.
     */
    virtual uint32_t calculateEffectiveMss();

    /** mss = -1: snd_mss and the announced MSS take the MSS of the address family of the peer, 1460 for IPv4 and 1440 for IPv6 */
    virtual void resolveAddressFamilyMss();

    /** The path MTU search of RFC 4821 (mtuProbing): the MSS of the peer, limited by TCP_MAXSEG (Linux mss_clamp) */
    virtual uint32_t getMssClamp() const;
    /** The payload of a segment of this MTU, without the options of each segment, at most getMssClamp() (Linux tcp_mtu_to_mss()) */
    virtual uint32_t mtuToMss(uint32_t mtu) const;
    /** The MTU of a segment with this payload (Linux tcp_mss_to_mtu()) */
    virtual uint32_t mssToMtu(uint32_t mss) const;
    /** An ICMP error lowered the MSS in SYN_SENT: the next SYN announces it, and a SYN with data goes again at once (Linux tcp_v4_mtu_reduced()) */
    virtual void retransmitSynAfterMtuReduction(uint32_t newMss);
    /** A new route MTU bounds the path MTU search (Linux tcp_sync_mss()) */
    virtual void limitMtuSearch(uint32_t mtu);
    /** Arms the path MTU search when the MSS is settled (Linux tcp_mtup_init()) */
    virtual void initMtuProbing();
    /** The payload of the probe to send now; 0: wait for room for a probe; -1: no probe (Linux tcp_mtu_probe()) */
    virtual int64_t getMtuProbeBytes(uint32_t buffered, uint32_t congestionWindow, uint32_t bytesInFlight) const;
    /** The peer acknowledged the probe (Linux tcp_mtup_probe_success()) */
    virtual void mtuProbeSucceeded();
    /** The probe is lost (Linux tcp_mtup_probe_failed()) */
    virtual void mtuProbeFailed();

  public:
    TcpConnection() {}
    TcpConnection(const TcpConnection& other) {} // FIXME kludge
    void initialize();

    /**
     * The "normal" constructor.
     */
    void initConnection(Tcp *mod, int socketId);

    /**
     * Destructor.
     */
    virtual ~TcpConnection();

    int getTtl() const { return ttl; }
    int getSocketId() const { return socketId; }
    void setSocketId(int newSocketId) { ASSERT(socketId == -1); socketId = newSocketId; }
    int getListeningSocketId() const { return listeningSocketId; }

    int getLocalPort() const { return localPort; }
    const L3Address& getLocalAddress() const { return localAddr; }

    int getRemotePort() const { return remotePort; }
    const L3Address& getRemoteAddress() const { return remoteAddr; }

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
    TcpStateVariables *getStateForUpdate() { return state; }
    const TcpSendQueue *getSendQueue() const { return sendQueue; }
    TcpSendQueue *getSendQueueForUpdate() { return sendQueue; }
    const TcpSackRexmitQueue *getRexmitQueue() const { return rexmitQueue; }
    TcpSackRexmitQueue *getRexmitQueueForUpdate() { return rexmitQueue; }
    const TcpReceiveQueue *getReceiveQueue() const { return receiveQueue; }
    TcpReceiveQueue *getReceiveQueueForUpdate() { return receiveQueue; }
    const TcpAlgorithm *getTcpAlgorithm() const { return tcpAlgorithm; }
    TcpAlgorithm *getTcpAlgorithmForUpdate() { return tcpAlgorithm; }

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
     * Process an ICMPv4 error indication for this connection.
     * Hard errors (port/protocol unreachable) abort connections in SYN_SENT state.
     * All other cases send a soft TCP_I_ICMPv4_ERROR notification to the app.
     * Returns false if the connection has moved to CLOSED.
     */
    virtual bool processIcmpv4Error(Indication *indication);

    /**
     * Process an ICMPv6 error indication for this connection.
     * Hard errors (port unreachable, admin prohibited) abort connections in SYN_SENT state.
     * All other cases send a soft TCP_I_ICMPv6_ERROR notification to the app.
     * Returns false if the connection has moved to CLOSED.
     */
    virtual bool processIcmpv6Error(Indication *indication);

    /**
     * Returns true if the given ICMPv4 type+code is a "hard" error
     * (protocol unreachable, port unreachable, admin prohibited).
     */
    static bool isHardIcmpv4Error(int type, int code);

    /**
     * Returns true if the given ICMPv6 type+code is a "hard" error
     * (port unreachable, admin prohibited).
     */
    static bool isHardIcmpv6Error(int type, int code);

    /**
     * Returns false if icmpSequenceCheck is on and the segment that the ICMP error quotes
     * has a sequence number outside SND.UNA to SND.MAX (RFC 5927 section 4.1).
     */
    virtual bool isIcmpErrorInWindow(const Packet *originalPacket) const;

    /**
     * Returns true if the ICMPv4 error is "Fragmentation Needed and DF Set"
     * (type=3, code=4), used for Path MTU Discovery (RFC 1191).
     */
    static bool isFragNeeded(IcmpType type, int code);

    /**
     * Returns true if the ICMPv6 error is "Packet Too Big"
     * (type=2), used for Path MTU Discovery (RFC 1981).
     */
    static bool isPacketTooBig(Icmpv6Type type, int code);

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

    friend class Tcp;
};

} // namespace tcp

} // namespace inet

#endif

