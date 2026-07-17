//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.transportlayer.tcp_common.TcpHeader;

namespace inet::tcp;

cplusplus {{
typedef std::list<Sack> SackList;
typedef std::vector<uint8_t> FastOpenCookie;
}}

class SackList { @existingClass; }
class FastOpenCookie { @existingClass; }

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
enum TcpState {
    TCP_S_INIT        = 0;
    TCP_S_CLOSED      = 1;
    TCP_S_LISTEN      = 2;
    TCP_S_SYN_SENT    = 3;
    TCP_S_SYN_RCVD    = 4;
    TCP_S_ESTABLISHED = 5;
    TCP_S_CLOSE_WAIT  = 6;
    TCP_S_LAST_ACK    = 7;
    TCP_S_FIN_WAIT_1  = 8;
    TCP_S_FIN_WAIT_2  = 9;
    TCP_S_CLOSING     = 10;
    TCP_S_TIME_WAIT   = 11;
};

//
// Event, strictly for the FSM state transition purposes.
// DO NOT USE outside performStateTransition()!
//
enum TcpEventCode {
    TCP_E_IGNORE = 0;

    // app commands
    // (Note: no RECEIVE command, data are automatically passed up)
    TCP_E_OPEN_ACTIVE = 1;
    TCP_E_OPEN_PASSIVE = 2;
    TCP_E_ACCEPT = 3;
    TCP_E_SEND = 4;
    TCP_E_CLOSE = 5;
    TCP_E_ABORT = 6;
    TCP_E_DESTROY = 7;
    TCP_E_STATUS = 8;
    TCP_E_QUEUE_BYTES_LIMIT = 9;
    TCP_E_READ = 10;
    TCP_E_SETOPTION = 11;

    // TPDU types
    TCP_E_RCV_DATA = 12;
    TCP_E_RCV_ACK = 13;
    TCP_E_RCV_SYN = 14;
    TCP_E_RCV_SYN_ACK = 15;
    TCP_E_RCV_FIN = 16;
    TCP_E_RCV_FIN_ACK = 17;
    TCP_E_RCV_RST = 18; // covers RST+ACK too

    TCP_E_RCV_UNEXP_SYN = 19; // unexpected SYN

    // timers
    TCP_E_TIMEOUT_2MSL = 20; // RFC 793, a.k.a. TIME-WAIT timer
    TCP_E_TIMEOUT_CONN_ESTAB = 21;
    TCP_E_TIMEOUT_FIN_WAIT_2 = 22;

    // All other timers (REXMT, PERSIST, DELAYED-ACK, KEEP-ALIVE, etc.),
    // are handled in TcpAlgorithm.
};

//
// Contains state variables ("TCB") for TCP.
//
// TcpStateVariables is effectively a "struct" -- it only contains
// public data members. (Only declared as a class so that we can use
// cObject as base class and make it possible to inspect
// it in Tkenv.)
//
// TcpStateVariables only contains variables needed to implement
// the "base" (RFC 793) TCP. More advanced TCP variants are encapsulated
// into TcpAlgorithm subclasses which can have their own state blocks,
// subclassed from TcpStateVariables. See TcpAlgorithm::createStateVariables().
//
struct TcpStateVariables
{
    @implements(cObject);
    @descriptor(readonly);

    bool active = false;            // set if the connection was initiated by an active open
    bool fork = false;              // if passive and in LISTEN: whether to fork on an incoming connection

    uint32_t snd_mss = 0;           // sender maximum segment size (without headers, i.e. only segment text); see RFC 2581, page 1.
    uint32_t advertisedMss = 0;     // our OWN maximum receivable segment size, as announced in this side's SYN/SYN-ACK MSS option
                                    // (RFC 793/9293: the option describes the announcing side's receive limit). Unlike snd_mss it is
                                    // NEVER clamped by the peer's announced MSS -- a server used to advertise the already-clamped
                                    // snd_mss, wrongly echoing a small client MSS back (caught by the packetdrill oracle: Linux
                                    // advertises 1460 in the SYN-ACK regardless of the client's mss 1000).
                                    // This will be set to the minimum of the local smss parameter and the value specified in the
                                    // MSS option received during connection setup.

    // send sequence number variables (see RFC 793, "3.2. Terminology")
    uint32_t snd_una = 0;           // send unacknowledged
    uint32_t snd_nxt = 0;           // send next (drops back on retransmission)
    uint32_t snd_max = 0;           // max seq number sent (needed because snd_nxt is re-set on retransmission)
    uint32_t maxPacketsOut = 0;     // peak segments in flight observed at send time (Linux max_packets_out); drives the RFC 5681 cwnd-limited slow-start gate
    uint32_t snd_wnd = 0;           // send window
    uint32_t snd_up = 0;            // send urgent pointer
    uint32_t snd_wl1 = 0;           // segment sequence number used for last window update
    uint32_t snd_wl2 = 0;           // segment ack. number used for last window update
    uint32_t iss = 0;               // initial sequence number (ISS)

    // receive sequence number variables
    uint32_t rcv_nxt = 0;           // receive next
    uint32_t rcv_wnd = 0;           // receive window
    uint32_t rcv_up = 0;            // receive urgent pointer;
    uint32_t irs = 0;               // initial receive sequence number
    uint32_t rcv_adv = 0;           // advertised window

    // SYN, SYN+ACK retransmission variables (handled separately
    // because normal rexmit belongs to TcpAlgorithm)
    int syn_rexmit_count = 0;       // number of SYN/SYN+ACK retransmissions (=1 after first rexmit)
    simtime_t syn_rexmit_timeout;   // current SYN/SYN+ACK retransmission timeout

    // whether ACK of our FIN has been received. Needed in FIN bit processing
    // to decide between transition to TIME-WAIT and CLOSING (set event code
    // TCP_E_RCV_FIN or TCP_E_RCV_FIN_ACK).
    bool fin_ack_rcvd = false;

    bool send_fin = false;          // true if a user CLOSE command has been "queued"
    uint32_t snd_fin_seq = 0;       // if send_fin == true: FIN should be sent just before this sequence number

    bool fin_rcvd = false;          // whether FIN received or not
    uint32_t rcv_fin_seq = 0;       // if fin_rcvd: sequence number of received FIN

    bool nagle_enabled = false;                 // set if Nagle's algorithm (RFC 896) is enabled
    bool pushOnWriteBoundary = false;           // Linux parity: set PSH on the segment carrying the last buffered byte of a send (from the pushSegmentsOnWriteBoundary parameter)
    bool delayed_acks_enabled = false;          // set if delayed ACK algorithm (RFC 1122) is enabled
    bool limited_transmit_enabled = false;      // set if Limited Transmit algorithm (RFC 3042) is enabled
    bool increased_IW_enabled = false;          // set if Increased Initial Window (RFC 3390) is enabled (deprecated, mapped to init_cwnd_mode)
    int init_cwnd_mode = 0;                      // initial window: 0=RFC2001 (1 SMSS), 1=RFC3390, 2=RFC6928 (IW10)

    uint32_t full_sized_segment_counter = 0;    // this counter is needed for delayed ACK
    bool ack_now = false;                       // send ACK immediately, needed if delayed_acks_enabled is set
                                                // Based on [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 861].
                                                // ack_now should be set when:
                                                //   - delayed ACK timer expires
                                                //   - an out-of-order segment is received
                                                //   - SYN is received during the three-way handshake
                                                //   - a persist probe is received
                                                //   - FIN is received

    bool afterRto = false;                      // set at RTO, reset when snd_nxt == snd_max or snd_una == snd_max

    // WINDOW_SCALE related variables
    bool ws_support = false;                    // set if the host supports Window Scale (header option) (RFC 1323)
    bool ws_enabled = false;                    // set if the connection uses Window Scale (header option)
    int  ws_manual_scale = -1;                  // the value of scale parameter if it was set manually (-1 otherwise)
    bool snd_ws = false;                        // set if initial WINDOW_SCALE has been sent
    bool rcv_ws = false;                        // set if initial WINDOW_SCALE has been received
    unsigned int rcv_wnd_scale = 0;             // RFC 1323, page 31: "Receive window scale power"
    unsigned int snd_wnd_scale = 0;             // RFC 1323, page 31: "Send window scale power"

    // TIMESTAMP related variables
    bool ts_support = false;        // set if the host supports Timestamps (header option) (RFC 1322)
    bool ts_enabled = false;        // set if the connection uses Window Scale (header option)
    bool snd_initial_ts = false;    // set if initial TIMESTAMP has been sent
    bool rcv_initial_ts = false;    // set if initial TIMESTAMP has been received
    uint32_t ts_recent = 0;         // RFC 1323, page 31: "Latest received Timestamp"
    uint32_t last_ack_sent = 0;     // RFC 1323, page 31: "Last ACK field sent"
    simtime_t time_last_data_sent;  // time at which the last data segment was sent (needed to compute the IDLE time for PAWS)
    simtime_t time_last_segment_received = 0; // time at which the last segment was received (idle base for keepalive)

    // SACK related variables
    bool sack_support = false;      // set if the host supports selective acknowledgment (header option) (RFC 2018, 2883, 3517)
    bool sack_enabled = false;      // set if the connection uses selective acknowledgment (header option)
    bool snd_sack_perm = false;     // set if SACK_PERMITTED has been sent
    bool rcv_sack_perm = false;     // set if SACK_PERMITTED has been received
    uint32_t start_seqno = 0;       // start sequence number of last received out-of-order segment
    uint32_t end_seqno = 0;         // end sequence number of last received out-of-order segment
    bool snd_sack = false;          // set if received vaild out-of-order segment or rcv_nxt changed, but receivedQueue is not empty
    bool snd_dsack = false;         // set if received duplicated segment (sequenceNo+PLength < rcv_nxt) or (segment is not acceptable)
    SackList sacks_array;    // MAX_SACK_BLOCKS is set to 60
    uint32_t highRxt = 0;    // RFC 3517, page 3: ""HighRxt" is the highest sequence number which has been retransmitted during the current loss recovery phase."
    uint32_t pipe = 0;    // RFC 3517, page 3: ""Pipe" is a sender's estimate of the number of bytes outstanding in the network."
    uint32_t recoveryPoint = 0;    // RFC 3517
    uint32_t sackedBytes = 0;    // number of sackedBytes
    uint32_t sackedBytes_old = 0;    // old number of sackedBytes - needed for RFC 3042 to check if last dupAck contained new sack information
    bool lossRecovery = false;    // indicates if algorithm is in loss recovery phase

    // delivered-bytes accounting (RFC 8985 delivery; RFC 6937 PRR input)
    uint64_t deliveredBytes = 0;   // cumulative newly-delivered (cumulatively acked + selectively acked) bytes
    uint64_t prrDeliveredMark = 0; // snapshot of deliveredBytes at the start of the current segment (per-ACK delta = newly acked+sacked)

    // PRR - Proportional Rate Reduction (RFC 6937)
    bool prrEnabled = false;       // use PRR during fast recovery (set from the prrEnabled parameter)
    uint32_t prrDelivered = 0;     // bytes delivered during the current recovery episode
    uint32_t prrOut = 0;           // bytes transmitted during the current recovery episode
    uint32_t priorCwnd = 0;        // snd_cwnd snapshot at recovery entry (RFC 6937 RecoverFS proxy; also the undo target)

    // loss undo (RFC 2883 D-SACK, RFC 3522 Eifel)
    bool lossUndoEnabled = false;  // set from the lossUndoEnabled parameter
    uint32_t undoMarker = 0;       // snd_una at recovery entry; 0 = no undo pending
    int32_t undoRetrans = 0;       // retransmitted segments still to be confirmed spurious (-1 = unknown)
    uint32_t priorSsthresh = 0;    // ssthresh before the reduction (restored on undo)
    uint32_t retransStampTS = 0;   // TSval echoed on the first retransmit of the episode (Eifel); 0 = none
    bool dsackSeen = false;        // a D-SACK was seen while processing the current segment (reset per segment)
    uint32_t dsackBytes = 0;       // bytes covered by the D-SACK seen on the current segment

    // adaptive reordering / dynamic DupThresh (Linux tp->reordering)
    bool adaptiveReorderingEnabled = false; // set from the adaptiveReorderingEnabled parameter
    uint32_t reordering = 3;       // learned reordering degree in segments; initialized to dupthresh
    uint32_t maxReordering = 300;  // upper bound (set from the maxReordering parameter)

    // RACK-TLP loss detection (RFC 8985)
    int lossDetectionMode = 0;    // 0 = classic DupThresh (RFC 3517), 1 = RACK time-based
    simtime_t minRtt = 0;         // minimum RTT observed on the connection (0 = unset)
    simtime_t rackXmitTime = 0;   // send time of the most recently delivered (SACKed/ACKed) segment
    uint32_t rackEndSeq = 0;      // end sequence number of that segment
    simtime_t rackRtt = 0;        // RTT of the ACK that last advanced the RACK point
    bool rackReordSeen = false;   // whether reordering has been observed (widens the reordering window)

    // queue management
    uint32_t sendQueueLimit = 0;
    bool queueUpdate = true;

    // TCP_NOTSENT_LOWAT (Workstream H4): independent of sendQueueLimit/queueUpdate
    // above (which track total outstanding+unsent bytes from snd_una); this tracks
    // only the not-yet-transmitted portion from snd_nxt. (uint32_t)-1 = disabled.
    uint32_t notsentLowat = (uint32_t)-1;
    bool notsentLowatUpdate = true;

    // TCP_INFO trio (busy_time/rwnd_limited): cumulative time bookkeeping, read-only
    // with respect to every send/receive decision -- consulted only by TcpStatusInfo.
    // INET-native definitions (not a port of Linux's tcp_chrono_* state machine); see
    // enqueueSendCommandData()/processAckInEstabEtc() (busy) and sendData() (rwnd).
    // A *StartTime of -1 means "not currently in that state"; *Accumulated is the
    // running total up to the last state exit. sndbuf_limited has no INET analog
    // (the send queue never actually blocks a SEND on sendQueueLimit -- it's
    // notification-only, see notsentLowat/sendQueueLimit above) and is deliberately
    // not tracked here.
    simtime_t busyStartTime;          // set to -1 in configureStateVariables() -- see pmtudLastMssReduction below for the same pattern
    simtime_t busyTimeAccumulated = 0;
    simtime_t rwndLimitedStartTime;   // set to -1 in configureStateVariables()
    simtime_t rwndLimitedAccumulated = 0;

    // those counters would logically belong to TcpAlgorithm, but it's a lot easier to manage them here
    uint32_t dupacks = 0;    // current number of received consecutive duplicate ACKs
    uint32_t snd_sacks = 0;    // number of sent sacks
    uint32_t rcv_sacks = 0;    // number of received sacks
    uint32_t rcv_oooseg = 0;    // number of received out-of-order segments
    uint32_t rcv_naseg = 0;    // number of received not acceptable segments

    // receiver buffer / receiver queue related variables
    uint32_t maxRcvBuffer = 0;    // maximal amount of bytes in tcp receive queue
    uint32_t usedRcvBuffer = 0;    // current amount of used bytes in tcp receive queue
    uint32_t freeRcvBuffer = 0;    // current amount of free bytes in tcp receive queue
    uint32_t tcpRcvQueueDrops = 0;    // number of drops in tcp receive queue

    // ECN
    bool ecnEchoState = false; // indicates if connection is in echo mode (got CE indication from IP and didn't get CWR from sender yet)
    bool sndCwr = false; // set if ECE was handled
    bool gotEce = false; // set if packet with ECE arrived
    bool gotCeIndication = false; // set if CE was set in controlInfo from IP
    bool ect = false; // set if this connection is ECN Capable (ECT stands for ECN-Capable transport - rfc-3168)
    bool endPointIsWillingECN = false; // set if the other end-point is willing to use ECN
    bool ecnSynSent = false; // set if ECN-setup SYN packet was sent
    bool ecnWillingness = false; // set if current host is willing to use ECN. Kept in sync with ecnMode>=2 ("rfc3168") for backward compatibility -- every classic-ECN call site still keys off this field, unmodified since before AccECN (Workstream G)
    bool sndAck = false; // set if sending Ack packet, used to set relevant info in controlInfo.
    bool rexmit = false; // set if retransmitting data, used to send not-ECT codepoint (rfc3168, p. 20)
    simtime_t eceReactionTime; // records the time of the last ECE reaction

    // AccECN (draft-ietf-tcpm-accurate-ecn), Workstream G
    int ecnMode = 0; // from the tcpEcnMode parameter (or the deprecated ecnWillingness, mapped): 0=off, 1=passive, 2=rfc3168, 3=accecn, 4=accecn-passive
    bool accEcnNegotiated = false; // set once 3WHS negotiation (G3) determines this connection uses AccECN, not classic ECN or no-ECN
    bool aeSynSent = false; // set if an AccECN-requesting SYN (the flag-bit combination, G3) was sent
    uint32_t rcvCePkts = 0; // receiver-side count of CE-marked packets seen (mod-8 ACE counter input, G4/G5)
    uint32_t rcvCePktsReported = 0; // rcvCePkts value as of the last ACE delta the peer has acknowledged resolving (G5)
    uint32_t deliveredCePkts = 0; // sender-side resolved count of CE-marked packets the peer has reported via ACE/option (G5/G7)

    // AccECN TCP option (kind 172/174 E0B/E1B/CEB byte counters), Workstream G6/G7
    bool accEcnOptionEnabled = false; // cached from the accEcnOptionEnabled param at configureStateVariables() time
    uint32_t accEcnOptionBeaconAcks = 4; // cached from the accEcnOptionBeaconAcks param: emit the option on every Nth ACK-bearing segment
    uint32_t accEcnAckCount = 0; // counts ACK-bearing segments sent since AccECN negotiated, for the beacon cadence above
    bool accEcnOptionNextKindIsAccEcn1 = false; // alternates which of kind 172/174 is emitted each time the option is actually sent (G6.1)
    bool accEcnOptionKindAlternates = true; // cached from the accEcnOptionKindAlternates param: if false, always emit kind 174 (Linux behavior) instead of alternating
    uint32_t rcvEct0Bytes = 0; // receiver-side byte count of segments arriving marked ECT(0) since AccECN negotiated (option's E0B field content, G6.1)
    uint32_t rcvEct1Bytes = 0; // receiver-side byte count of segments arriving marked ECT(1) since AccECN negotiated (option's E1B field content, G6.1)
    uint32_t rcvCeBytes = 0; // receiver-side byte count of segments arriving marked CE since AccECN negotiated (option's CEB field content, G6.1)

    // AccECN TCP option read side (G7): peer-reported byte counts, decoded from the last
    // AccECN option this connection has received (offsets already un-applied). These are
    // PEER-reported feedback on bytes WE sent (opposite direction from rcvEct*Bytes/rcvCeBytes
    // above, which are what WE observed on bytes the peer sent us).
    uint32_t peerReportedEct0Bytes = 0;
    uint32_t peerReportedEct1Bytes = 0;
    // peerReportedCeBytes is the CEB baseline, advanced only at the point its delta is
    // actually consumed (processAckInEstabEtc()'s ACE block) -- NOT at decode time
    // (readHeaderOptions() below), because that block has an early-return path (an ACK
    // beyond snd_max) that would otherwise silently lose the delta: if the baseline had
    // already advanced but the block never ran to fold the delta into deliveredCeBytes,
    // those CE bytes would be gone for good, and the next real option would diff against
    // an already-advanced baseline as if they'd been accounted for. Same read-vs-mutate
    // separation as G6.1's write-side fix (dry-run calls there, an early return here).
    uint32_t peerReportedCeBytes = 0; // cumulative (peer's clock); 0 is a legitimate starting baseline, not a sentinel -- CEB itself starts at 0
    bool accEcnOptionCebDeltaValid = false; // true only for the segment just processed by readHeaderOptions(), if it carried a valid AccECN option
    uint32_t accEcnOptionRawCeBytes = 0; // valid only when accEcnOptionCebDeltaValid: the raw (offset-corrected, but not yet diffed) CEB value decoded from that segment's option
    uint32_t deliveredCeBytes = 0; // sender-side resolved CE byte count from option evidence only (G7); stays 0 if the peer never sends the option

    uint32_t dupthresh = 0; // used for TcpTahoe, TcpReno and SACK (RFC 3517)

    // Path MTU Discovery (RFC 1191, RFC 1981)
    bool pmtudEnabled = false;              // set if Path MTU Discovery is enabled
    uint32_t pmtudOriginalMss = 0;          // negotiated MSS before any PMTUD reduction
    simtime_t pmtudTimeout;                 // time after which the original MSS is restored to probe for increased path MTU
    simtime_t pmtudLastMssReduction;        // timestamp of last PMTUD-triggered MSS reduction (-1 = never reduced)

    // TCP Fast Open (RFC 7413)
    bool fastopenClientEnabled = false;      // from the fastopenClientEnabled parameter
    bool fastopenServerEnabled = false;      // from the fastopenServerEnabled parameter
    bool fastopenAcceptWithoutCookie = false; // server: accept SYN data carrying no cookie option at all (tcp_fastopen 0x200 / TFO_SERVER_COOKIE_NOT_REQD)
    bool fastopenLenientCookieValidation = true; // from the fastopenLenientCookieValidation parameter
    bool fastopenExpOptionEnabled = false;   // from the fastopenExpOptionEnabled parameter
    int fastopenCookieBytes = 8;             // from the fastopenCookieBytes parameter
    bool fastopenCookieRequested = false;    // server: peer sent an empty-cookie TFO option this SYN
    bool fastopenCookieValid = false;        // server: peer's cookie validated (accelerate this SYN)
    bool fastopenSendCookieOption = false;   // server: echo a (possibly fresh) cookie in the SYN-ACK
    FastOpenCookie fastopenCookieToSend;     // server: the cookie bytes to echo, if fastopenSendCookieOption
    bool fastopenSynDeferred = false;        // client: connect(fastOpen=true) is waiting for the app's first SEND (or the connEstabTimer safety net) before sending the SYN
    bool fastopenCookieRequestPending = false; // client: no cookie was cached, so the (immediate, dataless) SYN requests one
    uint32_t fastopenSynDataLen = 0;         // client: bytes of app data attached to the SYN (0 = none); fixed once set, reused verbatim across SYN-REXMIT
    bool fastopenRequested = false;          // client: this connection's own connect() call opted into Fast Open (per-connection, like Linux's MSG_FASTOPEN/TCP_FASTOPEN_CONNECT) -- gates the cookie append independently of the module-wide fastopenClientEnabled parameter, so a plain connect() to a destination with a cached cookie does NOT send it uninvited
    bool fastopenSynDataAccepted = false;    // server: a validated-cookie SYN carried data that was accepted and delivered ahead of the 3WHS (mirrors Linux's TCPI_OPT_SYN_DATA tcp_info flag)
    uint32_t pktsAckedEwma = 0;              // AccECN: EWMA of packets acked per ACK (<<6 fixed point), for the ACE-wrap-vs-ACK-compression heuristic on large ACKs
};

cplusplus(TcpStateVariables) {{
  public:
    virtual ~TcpStateVariables();
    virtual std::string str() const override;
    virtual std::string detailedInfo() const;
}}
