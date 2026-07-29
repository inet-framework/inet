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
#include <map>
#include <set>
#include <deque>

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
#define TCP_TIMEOUT_SYN_REXMIT        3  // initially 3 seconds
#define TCP_TIMEOUT_SYN_REXMIT_MAX    240  // 4 mins (will only be used with SYN+ACK: with SYN CONN_ESTAB occurs sooner)
//@}

#define MAX_SYN_REXMIT_COUNT          12  // will only be used with SYN+ACK: with SYN CONN_ESTAB occurs sooner
#define TFO_BLACKHOLE_RTO_THRESHOLD   2  // TCP Fast Open active blackhole detection: syn_rexmit_count value (i.e. the 3rd SYN transmission, matching the kernel's "timeouts == 2" check) that triggers a suspected-blackhole report for a data-carrying SYN

// AccECN (draft-ietf-tcpm-accurate-ecn): state->ecnMode values, mirroring the
// tcpEcnMode NED enum by index (Tcp.ned / TcpConnectionState.msg's ecnMode field).
#define TCP_ECN_MODE_OFF              0
#define TCP_ECN_MODE_PASSIVE          1
#define TCP_ECN_MODE_RFC3168          2
#define TCP_ECN_MODE_ACCECN           3
#define TCP_ECN_MODE_ACCECN_PASSIVE   4
#define TCP_MAX_WIN                   65535lu  // 65535 bytes, largest value (16 bit) for (unscaled) window size
#define TCP_MAX_WIN_SCALED            0x3fffffffL // 2^30-1 bytes, largest value for scaled window size
#define MAX_SACK_BLOCKS               60  // will only be used with SACK
#define PAWS_IDLE_TIME_THRESH         (24 * 24 * 3600)  // 24 days in seconds (RFC 7323)

/**
 * Manages a TCP connection. This class itself implements the TCP state
 * machine as well as handling control PDUs (SYN, SYN+ACK, RST, FIN, etc.),
 * timers (2MSL, CONN-ESTAB, FIN-WAIT-2) and events (OPEN, SEND, etc)
 * associated with TCP state changes.
 *
 * The implementation largely follows the functional specification at the end
 * of RFC 9293. Code comments extensively quote RFC 9293 to make it easier
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
    static simsignal_t deliveredCeSignal; // AccECN: cumulative resolved count of CE-marked packets the peer has reported via the ACE field
    static simsignal_t deliveredCeBytesSignal; // AccECN: cumulative CE byte count from AccECN option evidence only (stays 0 if the peer never sends the option)
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
    // SO_TIMESTAMPING: unrelated to TcpConnectionState's
    // ts_support/ts_enabled (the RFC 1323 TCP Timestamps wire option) -- this is the
    // app-facing, no-wire-impact socket option gating TcpRxTimestampInd delivery. A
    // plain TcpConnection member (like ttl/dscp/tos above), not a state field, since
    // it must be settable via TCP_C_SETOPTION before state exists (app code may call
    // TcpSocket::setTimestamping() before connect(), same as setTtl/setDscp/setTos).
    bool rxTimestampingEnabled = false;
    // Runtime TCP_NOTSENT_LOWAT (TcpSetNotsentLowatCommand): like
    // rxTimestampingEnabled above, must survive arriving before state exists
    // (a sockopt sent between bind() and connect()/listen()). INT_MIN = never
    // set; otherwise applied over the notsentLowat module param in
    // configureStateVariables(), and directly to state when set later.
    int notsentLowatSockopt = INT_MIN;
    // Runtime TCP_MAXSEG (TcpSetMaxSegCommand): like notsentLowatSockopt, must
    // survive arriving before state exists (a sockopt sent before connect()/
    // listen()). -1 = never set; otherwise clamps advertisedMss/snd_mss in
    // configureStateVariables(), and applied directly to state when set later.
    int userMss = -1;
    // A route MTU handed down at runtime (TcpSetPathMtuCommand), same
    // arrives-before-state discipline as userMss. 0 = never set; otherwise it
    // overrides the pathMtu module param as the RFC 4821 search's ceiling.
    int pathMtuSockopt = 0;
    // A receive-buffer size handed down at runtime (TcpSetRcvBufCommand), same
    // arrives-before-state discipline as userMss. -1 = never set; otherwise it
    // overrides the receiveBufferSize module param AND pins the buffer.
    int rcvBufSockopt = -1;
    // Runtime TCP_NODELAY / TCP_CORK (TcpSetNoDelayCommand / TcpSetCorkCommand)
    // that may arrive before OPEN creates state; INT_MIN = never set, otherwise
    // applied in configureStateVariables() (mirrors notsentLowatSockopt/userMss).
    int nodelaySockopt = INT_MIN;
    int corkSockopt = INT_MIN;
    bool autoRead = true;
    // Linux sk->sk_socket presence: false = embryonic (listening-side, not yet
    // accept()ed by the application). Gates OOO-pressure rcvbuf growth
    // (tcp_data_queue_ofo's not-yet-accepted skip). Set via TcpSetOwnedCommand;
    // defaults to owned so ordinary applications are unaffected.
    bool appOwned = true;
    // Linux SOCK_NOSPACE-while-waiting: the application's blocking write is
    // stalled on send-buffer space (TcpSetWriterBlockedCommand). While set AND
    // the send queue has run dry (everything queued is in flight), the
    // SNDBUF_LIMITED chrono accumulates -- tcp_write_xmit's queue-empty +
    // SOCK_NOSPACE start condition (tcp-info-sndbuf-limited).
    bool writerBlocked = false;
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

    // MSG_EOR: sequence numbers marking the end of a SEND that
    // requested a record boundary. sendSegment() must never build a segment
    // spanning one of these; keyed on sequence number (not send-queue position)
    // so it survives retransmission for free. Pruned lazily in sendSegment().
    std::set<uint32_t> eorSeqNums;
    // per-write PSH boundaries (Linux tcp_mark_push at sendmsg time), consumed
    // by sendSegment(); only populated under pushSegmentsOnWriteBoundary
    std::set<uint32_t> pushSeqNums;
    // Linux forced_push boundaries (tcp_sendmsg copy loop): wire segments ending
    // exactly here carry PSH; recorded at enqueue time, purged as they are acked
    std::set<uint32_t> forcedPushSeqNums;

    // windowShrinkAllowed (Linux tcp_shrink_window=1) receive-buffer accounting:
    // Linux charges the buffer at skb-TRUESIZE granularity and scales free space
    // by the measured payload/truesize ratio (tp->scaling_ratio, a u8 fraction
    // of 256 updated in tcp_measure_rcv_mss). Each accepted in-order data
    // segment appends (payloadBytes, truesize) here; explicit application reads
    // release entries front-to-back (an skb is freed only once fully copied).
    std::deque<std::pair<uint32_t, uint32_t>> rcvSkbChain;
    uint64_t rcvBufOccupancy = 0; // sum of truesizes in rcvSkbChain (Linux sk_rmem_alloc)
    uint8_t rcvScalingRatio = 128; // Linux TCP_DEFAULT_SCALING_RATIO (50% of 256)
    uint32_t rcvMssEstimate = 536; // Linux icsk_ack.rcv_mss seed; gates ratio updates

  public:
    // Set when a data segment reaching BEYOND the advertised-window promise
    // (rcv_adv) was nevertheless accepted via the empty-receive-queue
    // exception (tcp_sequence's over-accept): the kernel does NOT send an
    // immediate ACK for such an arrival -- its own selftest documents this
    // ("A too big packet is accepted if the receive queue is empty. It does
    // not trigger an immediate ACK", tcp_rcv_neg_window; the 7.1.3 golden
    // provably sends none, even though a static reading of the quickack path
    // predicts one). Consumed and cleared by
    // TcpAlgorithmBase::receiveSeqChanged's adaptive branch, which then takes
    // the DELAYED path.
    bool overWindowAcceptPending = false;

  protected:

    // Linux's skb truesize for a tun-received TCP segment: the payload plus
    // IP+TCP headers and skb_shared_info (320B), allocated page-granular for
    // large packets (tun_alloc_skb's paged path) or from a power-of-two page
    // frag for small ones (tun_build_skb), plus struct sk_buff (~232B).
    // Pins rcv_wnd_shrink_allowed's golden offers exactly (10000B payload ->
    // 12520, 1023B -> 2280).
    static uint32_t linuxSkbTruesize(uint32_t tcpPayloadBytes)
    {
        uint32_t base = tcpPayloadBytes + 40 + 320;
        uint32_t alloc;
        if (base > 16384)
            // very large packets take order-3 (32KB) page compounds
            // (alloc_skb_with_frags / PAGE_ALLOC_COSTLY_ORDER) -- two 39000/
            // 60000-byte OOO injections then genuinely overflow a 131072
            // rcvbuf (ooo-before-and-after-accept pins the resulting
            // tcp_clamp_window growth)
            alloc = ((base + 32767) / 32768) * 32768;
        else if (base > 4096)
            alloc = ((base + 4095) / 4096) * 4096;
        else {
            alloc = 1024;
            while (alloc < base)
                alloc <<= 1;
        }
        return alloc + 232;
    }

    // MSG_ZEROCOPY: pending completion notifications, keyed on the
    // sequence number marking the end of a zerocopy-marked SEND's data -> the id to
    // report once transmission (sendSegment() advancing snd_nxt past that seq)
    // reaches it. IDs are assigned sequentially per connection, mirroring Linux's
    // own SO_ZEROCOPY id assignment.
    std::map<uint32_t, uint32_t> zerocopySeqNums;
    uint32_t nextZerocopyId = 0;

    // TCP behavior in data transfer state
    TcpAlgorithm *tcpAlgorithm = nullptr;

    // timers
    cMessage *the2MSLTimer = nullptr;
    cMessage *connEstabTimer = nullptr;
    cMessage *finWait2Timer = nullptr;
    cMessage *synRexmitTimer = nullptr; // for retransmitting SYN and SYN+ACK
    cMessage *rackReoTimer = nullptr; // RACK reordering timer (Linux ICSK_TIME_REO_TIMEOUT): fires when a not-yet-lost segment's RACK.rtt+reo_wnd deadline matures between ACKs

    // statistics
    long rcvdSegments = 0;
    long sentSegments = 0;
    uint32_t lastRcvdSeqNo = 0;
    uint32_t lastSentAck = 0;

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
    /**
     * rackReoTimer expired: re-run RACK loss detection (time has advanced, so
     * pending deadlines may have matured) and let the recovery strategy react
     * (enter fast recovery / retransmit) via ITcpRecovery::reoTimeout().
     */
    virtual void processRackReoTimeout();

  public:
    /**
     * (Re)arms the RACK reordering timer for the given delay, or cancels it when
     * delay is negative. Called by the recovery strategy from RACK loss detection.
     */
    virtual void rescheduleRackReoTimer(simtime_t delay);

    /**
     * RFC 8985 section 7.2 loss probe: send one segment of new data if any is
     * available, else retransmit the last outstanding segment. Returns whether
     * anything was actually sent.
     */
    virtual bool sendTlpProbe();

  protected:

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

    /**
     * AccECN: pick between the ACE field's packet-count-only naiveDelta
     * and safeDelta candidates using the AccECN option's byte-exact CEB evidence
     * as corroboration -- whichever candidate's byte estimate (delta * snd_mss)
     * is closer to the observed cebByteDelta wins. Isolated as its own method so it's
     * independently unit-testable (design reference: tcp_accecn_process's naive/safe/
     * option-evidence *shape* only, tcp_input.c, re-derived not transcribed -- see the
     * plan's Verified Facts point 3).
     */
    virtual int resolveAceDelta(int naiveDelta, int safeDelta, uint32_t cebByteDelta) const;

    /** @name Processing of TCP options. Invoked from readHeaderOptions(). Return value indicates whether the option was valid. */
    //@{
    virtual bool processMSSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionMaxSegmentSize& option);
    virtual bool processWSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionWindowScale& option);
    virtual bool processSACKPermittedOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionSackPermitted& option);
    virtual bool processTSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionTimestamp& option);
    virtual bool processFastOpenOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionTcpFastOpen& option);
    virtual bool processFastOpenExpOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionTcpFastOpenExp& option);
    //@}

    /** Shared cookie-processing core for both the standard (kind 34) and legacy
     * experimental (kind 254 + 0xF989 magic) Fast Open options. */
    virtual bool processFastOpenCookieBytes(const std::vector<uint8_t>& cookie);

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

    /** Utility: force out a partial segment currently withheld by TCP_CORK / MSG_MORE
     *  (uncork, TCP_NODELAY, or the cork timer). forcePush sets PSH on the flushed partial. */
    virtual void flushCorkedData(bool forcePush);

    /** Utility: sends 1 bytes as "probe", called by the "persist" mechanism */
    virtual bool sendProbe();

    /** Sends a zero-length keepalive probe (seq = snd_una - 1) to elicit an ACK from an idle peer. */
    virtual void sendKeepAliveProbe();

    /** Utility: retransmit one segment from snd_una */
    virtual void retransmitOneSegment(bool called_at_rto);

    /**
     * RFC 5681 / Linux tcp_enter_loss: on an RTO, mark the un-SACKed outstanding
     * data lost in the SACK scoreboard, so getBytesInFlight() stops counting it as
     * in the network and the whole window can be clocked out on the recovering ACKs
     * (rather than one segment per backed-off RTO on a tail drop with no SACK). No-op
     * without SACK. Called by each flavour's RTO handler after it computes ssthresh
     * from the pre-loss FlightSize.
     */
    virtual void markOutstandingLostOnRto();

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

    /**
     * MSG_EOR: enqueues a SEND's data into sendQueue, and if the
     * packet carries a TcpSendEorReq tag, records the new end of that data as a
     * boundary sendSegment() must not build a segment across.
     */
    virtual void enqueueSendCommandData(Packet *packet);

    /** Utility: adds control info to segment and sends it to IP */
    virtual void sendToIP(Packet *tcpSegment, const Ptr<TcpHeader>& tcpHeader);

    /**
     * Utility: the AccECN ECN-field reflector encoding (RFC 9768 section 3.2.3.2).
     * Maps the IP-ECN codepoint of the received SYN (reflected on the SYN-ACK) or
     * SYN-ACK (reflected on the handshake-completing ACK) to the ACE value that
     * carries it back: Not-ECT->0b010, ECT(1)->0b011, ECT(0)->0b100, CE->0b110.
     * The gaps in the encoding are deliberate: 0b000/0b001/0b111 are reserved by
     * table 2 for "no ECN" and "classic ECN only" during negotiation.
     */
    static uint8_t accEcnReflectedAce(int ipEcnCodepoint);

    /** Utility: the IP-ECN codepoint a received segment arrived with, or IP_ECN_NOT_ECT if untagged */
    static int receivedEcnCodepoint(Packet *tcpSegment);

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

    /** SNDBUF_LIMITED chrono sampling: (re)evaluate the "transmission starved
     * by the send buffer" condition (writerBlocked && no unsent data && data
     * outstanding) and open/close the accumulation interval accordingly.
     * Called after sends, enqueues, the writer-blocked toggles and ACKs. */
    virtual void updateSndbufLimitedChrono();

    /** Utility: returns true when receive queue has enough space for store the tcpHeader */
    virtual bool hasEnoughSpaceForSegmentInReceiveQueue(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader);

    /** Utility: update receive window (rcv_wnd), and calculate scaled value if window scaling enabled.
     *  Returns the (scaled) receive window size.
     */
    virtual uint16_t updateRcvWnd();

    /** Utility: update window information (snd_wnd, snd_wl1, snd_wl2) */
    virtual void updateWndInfo(const Ptr<const TcpHeader>& tcpHeader, bool doAlways = false);

    std::string validationInfo() const;

    virtual uint32_t calculateEffectiveMss();

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

    /**
     * Maps INET's independent loss-recovery bools onto Linux's tcp_ca_state
     * ordinals (TCP_CA_Open=0, Disorder=1, CWR=2, Recovery=3, Loss=4), for
     * TcpStatusInfo::ca_state. INET has no state tracking Linux's Disorder --
     * the pre-recovery "first dupACK seen" phase -- so that ordinal is never
     * returned; scripts asserting ca_state==Disorder will still diverge. This
     * is a known, documented imprecision, not a bug.
     */
    /**
     * The sequence number that DATA accounting -- the peer's advertised window and
     * the bytes-in-flight estimate -- is measured from. Normally snd_una, but a TCP
     * Fast Open server transmitting its response from SYN_RCVD still has snd_una at
     * the ISS: INET keeps the unacknowledged SYN-ACK there (the SYN-REXMIT timer owns
     * that slot), while Linux's child socket starts at snd_una = ISN+1
     * (tcp_create_openreq_child) and never counts the handshake segment as data --
     * its packets_out only ever sees the write queue. Left uncorrected, the SYN-ACK's
     * sequence slot eats one byte of both the peer's window and the congestion
     * window, costing the response its last full segment.
     */
    virtual uint32_t getDataSndUna() const;

    /**
     * MTU <-> MSS conversions for the RFC 4821 search (Linux tcp_mtu_to_mss /
     * tcp_mss_to_mtu). The header allowance is the FIXED part -- network header
     * plus TCP header plus the options carried on every established segment --
     * so the two are exact inverses and the search's bounds stay comparable.
     */
    /** Gives back the receive-buffer charge of the skbs an application read drained. */
    virtual void releaseRcvBufOccupancy(uint64_t readBytes);

    /**
     * How many segments go out as one GSO super-segment. Only the wire-realism PSH
     * rule depends on it: Linux forces PSH on a multi-segment skb, and after the
     * split the flag lands on the burst's last slice.
     */
    virtual uint32_t gsoBurstSegments(uint32_t congestionWindow, uint32_t bytesInFlight, uint32_t effectiveMss) const;

    virtual uint32_t mtuHeaderOverhead() const;
    virtual uint32_t mtuToMss(uint32_t mtu) const;
    virtual uint32_t mssToMtu(uint32_t mss) const;

    /** Arms the RFC 4821 search bounds once the connection's MSS is settled (Linux tcp_mtup_init). */
    virtual void mtupInit();

    /**
     * The payload of the RFC 4821 probe segment to send right now, or 0 when the
     * conditions for probing are not met (Linux tcp_mtu_probe). The probe is
     * deliberately larger than snd_mss: it is the experiment that decides whether
     * the path carries a bigger segment.
     */
    virtual uint32_t mtuProbeBytes(uint32_t buffered, uint32_t congestionWindow) const;

    /** The probe was acknowledged, so the path carries it: raise the lower bound and the MSS (Linux tcp_mtup_probe_success). */
    virtual void mtupProbeSucceeded();

    /** The probe was lost, so the path does not carry it: lower the upper bound (Linux tcp_mtup_probe_failed). */
    virtual void mtupProbeFailed();
    virtual int deriveLinuxCaState() const;
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

