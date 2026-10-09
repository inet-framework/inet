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
// RFC 9293, section 3.3.2:
//"
// LISTEN - represents waiting for a connection request from any remote TCP
//    peer and port.
// SYN-SENT - represents waiting for a matching connection request after having
//    sent a connection request.
// SYN-RECEIVED - represents waiting for a confirming connection request
//    acknowledgment after having both received and sent a connection request.
// ESTABLISHED - represents an open connection, data received can be delivered
//    to the user. The normal state for the data transfer phase of the
//    connection.
// FIN-WAIT-1 - represents waiting for a connection termination request from
//    the remote TCP peer, or an acknowledgment of the connection termination
//    request previously sent.
// FIN-WAIT-2 - represents waiting for a connection termination request from
//    the remote TCP peer.
// CLOSE-WAIT - represents waiting for a connection termination request from
//    the local user.
// CLOSING - represents waiting for a connection termination request
//    acknowledgment from the remote TCP peer.
// LAST-ACK - represents waiting for an acknowledgment of the connection
//    termination request previously sent to the remote TCP peer (this
//    termination request sent to the remote TCP peer already included an
//    acknowledgment of the termination request sent from the remote TCP peer).
// TIME-WAIT - represents waiting for enough time to pass to be sure the remote
//    TCP peer received the acknowledgment of its connection termination request
//    and to avoid new connections being impacted by delayed segments from
//    previous connections.
// CLOSED - represents no connection state at all.
//"
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
    bool forked = false;            // this connection was forked off a listener (it stays set after ACCEPT, unlike listeningSocketId): an RST or a timeout in SYN_RCVD closes it, because the listener still exists

    uint32_t snd_mss = 0;           // sender maximum segment size (without headers, i.e. only segment text); see RFC 5681.
                                    // This will be set to the minimum of the local smss parameter and the value specified in the
                                    // MSS option received during connection setup.
    uint32_t advertisedMss = 0;     // the MSS that this side announces in its SYN or SYN-ACK: its own receive limit
                                    // (RFC 9293 section 3.7.1), not clamped to the MSS of the peer as snd_mss is

    // send sequence number variables (see RFC 793, "3.2. Terminology")
    uint32_t snd_una = 0;           // send unacknowledged
    uint32_t snd_nxt = 0;           // send next (drops back on retransmission)
    uint32_t snd_max = 0;           // max seq number sent (needed because snd_nxt is re-set on retransmission)
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

    bool rcvShutdown = false;       // set by a CLOSE without halfClose when abortOnDataAfterClose is on (the full close of Linux): the application will not read again, so new data in FIN_WAIT_1/2 resets the connection (RFC 1122 section 4.2.2.13)
    bool fin_rcvd = false;          // whether FIN received or not
    uint32_t rcv_fin_seq = 0;       // if fin_rcvd: sequence number of received FIN

    bool nagle_enabled = false;                 // set if Nagle's algorithm (RFC 896) is enabled
    bool delayed_acks_enabled = false;          // set if delayed ACK algorithm (RFC 1122) is enabled
    bool limited_transmit_enabled = false;      // set if Limited Transmit algorithm (RFC 3042) is enabled
    bool increased_IW_enabled = false;          // set if Increased Initial Window (RFC 3390) is enabled

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
    bool ws_support = false;                    // set if the host supports Window Scale (header option) (RFC 7323)
    bool ws_enabled = false;                    // set if the connection uses Window Scale (header option)
    int  ws_manual_scale = -1;                  // the value of scale parameter if it was set manually (-1 otherwise)
    bool snd_ws = false;                        // set if initial WINDOW_SCALE has been sent
    bool rcv_ws = false;                        // set if initial WINDOW_SCALE has been received
    unsigned int rcv_wnd_scale = 0;             // RFC 7323: Rcv.Wind.Shift, the receive window scale exponent
    unsigned int snd_wnd_scale = 0;             // RFC 7323: Snd.Wind.Shift, the send window scale exponent

    // TIMESTAMP related variables
    bool ts_support = false;        // set if the host supports Timestamps (header option) (RFC 7323)
    bool ts_enabled = false;        // set if the connection uses Window Scale (header option)
    bool snd_initial_ts = false;    // set if initial TIMESTAMP has been sent
    bool rcv_initial_ts = false;    // set if initial TIMESTAMP has been received
    uint32_t ts_recent = 0;         // RFC 7323: TS.Recent, the latest received timestamp
    uint32_t last_ack_sent = 0;     // RFC 7323: Last.ACK.sent, the last ACK field sent
    simtime_t time_last_data_sent;  // time at which the last data segment was sent (needed to compute the IDLE time for PAWS)

    // SACK related variables
    bool sack_support = false;      // set if the host supports selective acknowledgment (header option) (RFC 2018, 2883, 6675)
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

    // queue management
    uint32_t sendQueueLimit = 0;
    bool queueUpdate = true;

    // those counters would logically belong to TcpAlgorithm, but it's a lot easier to manage them here
    uint32_t dupacks = 0;    // current number of received consecutive duplicate ACKs
    uint32_t limitedTransmitBytes = 0;    // new data that Limited Transmit sent on the current duplicate ACKs (RFC 5681 section 3.2 step 2 leaves it out of FlightSize)
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
    bool ect = false; // set if this connection is ECN Capable (ECT stands for ECN-Capable transport, RFC 3168)
    bool endPointIsWillingECN = false; // set if the other end-point is willing to use ECN
    bool ecnSynSent = false; // set if ECN-setup SYN packet was sent
    bool ecnWillingness = false; // set if current host is willing to use ECN
    bool sndAck = false; // set if sending Ack packet, used to set relevant info in controlInfo.
    bool rexmit = false; // set if retransmitting data, used to send not-ECT codepoint (RFC 3168, page 20)
    simtime_t eceReactionTime; // records the time of the last ECE reaction

    uint32_t dupthresh = 0; // used for TcpTahoe, TcpReno and SACK (RFC 6675)

    // Path MTU Discovery (RFC 1191, RFC 1981)
    bool pmtudEnabled = false;              // set if Path MTU Discovery is enabled
    uint32_t pmtudOriginalMss = 0;          // negotiated MSS before any PMTUD reduction
    simtime_t pmtudTimeout;                 // time after which the original MSS is restored to probe for increased path MTU
    simtime_t pmtudLastMssReduction;        // timestamp of last PMTUD-triggered MSS reduction (-1 = never reduced)
    // declared here for the code above that already uses it:
    uint32_t maxPacketsOut = 0;    // peak segments in flight in the current window of data (Linux max_packets_out); drives the cwnd-limited slow-start gate
    uint32_t cwndUsageSeq = 0;     // snd_nxt when the current window of data started (Linux cwnd_usage_seq)
    uint32_t snd_effmss = 0;        // effective sender maximum segment size without TCP/IP header options
    bool accEcnNegotiated = false; // set once 3WHS negotiation determines this connection uses AccECN, not classic ECN or no-ECN
    uint8_t accEcnOptMinFields = 0;
    bool accEcnOptionEnabled = false; // cached from the accEcnOptionEnabled param at configureStateVariables() time
    uint64_t deliveredBytes = 0;   // cumulative newly-delivered (cumulatively acked + selectively acked) bytes
    uint32_t lastRcvdTSecr = 0;    // most recent TSecr echoed back by the peer (Eifel input)
    simtime_t minRtt = 0;         // minimum RTT observed on the connection (0 = unset)
    int lossDetectionMode = 0;    // 0 = classic DupThresh (RFC 6675), 1 = RACK time-based
    simtime_t rackXmitTime = 0;   // send time of the most recently delivered (SACKed/ACKed) segment
    uint32_t rackEndSeq = 0;      // end sequence number of that segment
    simtime_t rackRtt = 0;        // RTT of the ACK that last advanced the RACK point
    bool rackReordSeen = false;   // whether reordering has been observed (widens the reordering window)
    bool adaptiveReorderingEnabled = false; // set from the adaptiveReorderingEnabled parameter
    bool dsack_enabled = false;    // report duplicate segments by D-SACK blocks (RFC 2883), set from the dsackEnabled parameter
    uint32_t reordering = 3;       // learned reordering degree in segments; initialized to dupthresh
    uint32_t maxReordering = 300;  // upper bound (set from the maxReordering parameter)

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
    uint32_t dsackEndSeq = 0;      // end of the D-SACK block seen on the current segment

    // F-RTO spurious-RTO detection (RFC 5682, SACK-enhanced; Linux tcp_frto=2)
    bool frtoEnabled = false;      // set from the frtoEnabled parameter
    bool frtoActive = false;       // an F-RTO episode is open (since the last RTO)
    uint32_t rtoRecoveryPoint = 0; // snd_max at the last RTO (RFC 5682 RecoveryPoint); 0 = the recovery of the RTO is complete
    bool frtoOrigAcked = false;    // never-retransmitted data was acked/SACKed during the episode

    // Tail Loss Probe (RFC 8985 section 7, Linux tcp_send_loss_probe())
    bool tlpEnabled = false;       // set from the tlpEnabled parameter
    uint32_t tlpHighSeq = 0;       // snd_max after the probe (RFC 8985 TLP.end_seq); 0 = no probe outstanding
    bool tlpRetrans = false;       // the outstanding probe retransmitted data (RFC 8985 TLP.is_retrans)

    // TCP_INFO time counters (busy_time/rwnd_limited): cumulative time bookkeeping, read-only
    // with respect to every send/receive decision -- consulted only by TcpStatusInfo.
    // INET-native definitions (not a port of Linux's tcp_chrono_* state machine); see
    // enqueueSendCommandData()/processAckInEstabEtc() (busy) and sendData() (rwnd).
    // A *StartTime of -1 means "not currently in that state"; *Accumulated is the
    // running total up to the last state exit.
    simtime_t busyStartTime;          // set to -1 in configureStateVariables()
    simtime_t busyTimeAccumulated = 0;
    simtime_t rwndLimitedStartTime;   // set to -1 in configureStateVariables()
    simtime_t rwndLimitedAccumulated = 0;

    bool seedRttFromHandshake = false; // seed srtt/rttvar/RTO from the SYN<->SYN-ACK exchange, as Linux does
    simtime_t handshakeSentTime = -1; // when our SYN (active) / SYN-ACK (passive) was last sent; seeds the RTT estimator from the handshake RTT on reaching ESTABLISHED (skipped per Karn if the handshake segment was retransmitted)
    simtime_t firstSynSentTime = -1; // when the first SYN was sent (Linux tp->retrans_stamp); a retransmitted SYN does not change it
    uint64_t prrDeliveredMark = 0; // snapshot of deliveredBytes at the start of the current segment (per-ACK delta = newly acked+sacked)
    bool sawAccEcnOpt = false;

    // TCP Fast Open (RFC 7413)
    bool fastopenClientEnabled = false;      // from the fastopenClientEnabled parameter
    bool fastopenServerEnabled = false;      // from the fastopenServerEnabled parameter
    bool fastopenLenientCookieValidation = false; // from the fastopenLenientCookieValidation parameter
    bool fastopenExpOptionEnabled = false;   // from the fastopenExpOptionEnabled parameter
    bool fastopenAcceptWithoutCookie = false; // from the fastopenAcceptWithoutCookie parameter
    bool fastopenClientNoCookieRequired = false; // from the fastopenClientNoCookieRequired parameter
    bool fastopenPeerUsedExpOption = false;  // the Fast Open option of the peer was the experimental option (kind 254); the server answers in it, and the client caches its form
    int fastopenCookieBytes = 8;             // from the fastopenCookieBytes parameter
    bool fastopenCookieRequested = false;    // server: the SYN requests a cookie (an empty Fast Open option)
    bool fastopenCookieValid = false;        // server: the cookie of the SYN is valid
    bool fastopenSendCookieOption = false;   // server: the SYN-ACK gives a cookie (fastopenCookieToSend)
    FastOpenCookie fastopenCookieToSend;     // server: the cookie for the SYN-ACK
    bool fastopenAccelerated = false;        // server: a SYN with a valid cookie made this connection, also without data; the application gets it at once and can send in SYN_RCVD
    bool fastopenRequested = false;          // client: the OPEN of this connection asked for Fast Open (Linux MSG_FASTOPEN or TCP_FASTOPEN_CONNECT); a cached cookie goes only on the SYN of such a connection
    bool fastopenSynDeferred = false;        // client: the SYN waits for the first SEND (or for the connection establishment timer), to carry its data
    bool fastopenCookieRequestPending = false; // client: no cookie is cached, so the SYN requests one
    bool fastopenSynCarriedOption = false;   // client: the SYN carried a Fast Open option (Linux tp->syn_fastopen); a cookie of the SYN-ACK is cached only then
    uint32_t fastopenSynDataLen = 0;         // client: the number of bytes on the SYN; a retransmitted SYN leaves them out, and they stay in flight
    bool fastopenSynDataAccepted = false;    // server: the data of the SYN went to the application before the handshake completed; client: the SYN-ACK acknowledged all the data of the SYN (Linux TCPI_OPT_SYN_DATA)
    uint32_t peerAdvertisedMss = 0;          // the value of the MSS option of the peer, before the local limit; the client caches it for the data of its next Fast Open SYN
};

cplusplus(TcpStateVariables) {{
  public:
    virtual ~TcpStateVariables();
    virtual std::string str() const override;
    virtual std::string detailedInfo() const;
}}
