//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp;

import inet.common.SimpleModule;
import inet.transportlayer.contract.ITcp;

//
// Implements the TCP protocol. For other implementations, see ~ITcp.
//
// This module is compatible with both ~Ipv4 and ~Ipv6.
//
// A TCP header is represented by the class ~TcpHeader.
//
// <b>Usage</b>
//
// To allow arbitrary modules send and receive data over TCP, you can use the
// ~TcpClientSocketIo and ~TcpServerSocketIo modules. Two module types,
// ~TcpClientApp and ~TcpServerApp, are extensible compound modules that build
// on top of the above two `SocketIo` modules and allow assembling TCP
// applications from pre-existing queuing components. There are also ready-made
// application models that use TCP, such as ~TcpSessionApp, ~TcpEchoApp, and
// ~TcpBasicClientApp/~TcpGenericServerApp.
//
// From C++, use the `TcpSocket` class, or see the Tcp class documentation for
// details on how to use the Tcp without the `TcpSocket` utility class.
//
// <b>Configuration Hints</b>
//
// TCP can be configured via module parameters. The following notes provide
// additional information compared to the module parameter documentation.
//
//   -# The `tcpAlgorithmClass` module parameter can be used to set the TCP flavor
//      to be used (`TcpNewReno`, `TcpReno`, `TcpTahoe`, etc.). Note that this setting
//      can be programmatically overridden on a per-connection basis. Also note
//      that the `TcpNoCongestionControl` and `DumbTcp` flavors are intentionally
//      simplified code that are only provided for testing/educational purposes,
//      and do not support many TCP features the more advanced ones do.
//
//   -# The `limitedTransmitEnabled` parameter can be used to enable/disable the
//      Limited Transmit algorithm (RFC 3042).
//
//   -# The `increasedIWEnabled` parameter can be used to change the initial window
//      from one segment (RFC 2001) (based on MSS) to maximal four segments
//      (min(4*MSS, max (2*MSS, 4380 bytes))) (RFC 3390).
//
//   -# The `advertisedWindow` parameter defines the amount of data TCP can
//      receive without the socket being read by the local user. Note that
//      connections can be opened programmatically in one of two modes,
//      "autoread" and "explicit-read", and that in "autoread" mode the
//      advertised window never decreases so there is effectively no flow
//      control. Links with large bandwidth*delay product require large
//      advertised windows to operate efficiently. If the window size is larger
//      than 64K, window scaling support needs to be turned on (on both
//      endpoints) using the `windowScalingSupport` parameter, and the
//      `windowScalingFactor` set to the appropriate shift count.
//      windowScalingFactor=-1 sets the shift count automatically to a value
//      suitable for the `advertisedWindow` setting.
//
//   -# The `sackSupport` parameter enables SACK (RFC 2018). SACK is only
//      used on a connection if both endpoints have it enabled.
//
//   -# The `timestampSupport` parameter enables the Timestamps option
//      (RFC 1323). Like SACK, it is only used if both endpoints support it.
//
//   -# The `ecnWillingness` parameter enables Explicit Congestion Notification
//      (RFC 3168). ECN is negotiated during the three-way handshake and is
//      only active if both endpoints are willing.  When using the `DcTcp`
//      algorithm, the `dctcpGamma` parameter controls the EWMA gain for
//      estimating the congestion level (RFC 8257).
//
//   -# The `pmtudEnabled` parameter enables Path MTU Discovery (RFC 1191,
//      RFC 1981). When enabled, IPv4 segments are sent with the DF bit set.
//      The `pmtudTimeout` parameter controls how long the reduced MSS is
//      kept before probing with the original MSS again.
//
// Notes:
//  - if you do active OPEN, then send data and close before the connection
//    has reached ESTABLISHED, the connection will go from SYN_SENT to CLOSED
//    without actually sending the buffered data. This is consistent with
//    RFC 793 but may not be what you'd expect.
//  - handling segments with SYN+FIN bits set (esp. with data too) is
//    inconsistent across TCPs, so check this one if it's of importance
//
// <b>Standards</b>
//
// Implementation is based on the following RFCs:
//   - RFC  793 - Transmission Control Protocol
//   - RFC  896 - Congestion Control in IP/TCP Internetworks
//   - RFC 1122 - Requirements for Internet Hosts -- Communication Layers
//   - RFC 1323 - TCP Extensions for High Performance
//   - RFC 2018 - TCP Selective Acknowledgment Options
//   - RFC 2581 - TCP Congestion Control
//   - RFC 2883 - An Extension to the Selective Acknowledgement (SACK) Option for TCP
//   - RFC 3042 - Enhancing TCP's Loss Recovery Using Limited Transmit
//   - RFC 3390 - Increasing TCP's Initial Window
//   - RFC 3517 - A Conservative Selective Acknowledgment (SACK)-based Loss Recovery
//                Algorithm for TCP
//   - RFC 3782 - The `NewReno` Modification to TCP's Fast Recovery Algorithm
//   - RFC 1191 - Path MTU Discovery
//   - RFC 1981 - Path MTU Discovery for IP version 6
//   - RFC 3168 - The Addition of Explicit Congestion Notification (ECN) to IP
//   - RFC 8257 - Data Center TCP (DCTCP): ECN Marking at the Data Sender
//
// Implemented features include the following:
//  - all RFC 793 TCP states and state transitions
//  - connection setup and teardown as in RFC 793
//  - generally, RFC 793-compliant segment processing
//  - all socket commands and indications
//  - receive buffer to cache above-sequence data and data not yet forwarded
//    to the user
//  - CONN-ESTAB timer, SYN-REXMIT timer, 2MSL timer, FIN-WAIT-2 timer
//  - selective acknowledgements aka. SACK (RFC 2018 and RFC 2883)
//  - RFC 3517 - SACK-based Loss Recovery algorithm which is a conservative
//    replacement of the fast recovery algorithm (RFC2581) integrated into
//    `TcpReno` but not into `TcpNewReno`, `TcpTahoe`, `TcpNoCongestionControl`, and `DumbTcp`.
//  - changes from RFC 2001 to RFC 2581:
//      - ACK generation (ack_now = true) RFC 2581, page 6: "(...) a Tcp receiver SHOULD send an immediate ACK
//        when the incoming segment fills in all or part of a gap in the sequence space."
//  - TCP header options:
//      - EOL: End of option list.
//      - NOP: Padding bytes, currently needed for SACK_PERMITTED and SACK.
//      - MSS: The value of snd_mss (SMSS) is set to the minimum of snd_mss
//        (local parameter) and the value specified in the MSS option
//        received during connection startup. Based on [RFC 2581, page 1].
//      - WS: Window Scale option, based on RFC 1323.
//      - SACK_PERMITTED: SACK can only be used if both nodes sent SACK_-
//        PERMITTED during connection startup.
//      - SACK: SACK option, based on RFC 2018, RFC 2883, and RFC 3517.
//      - TS: Timestamps option, based on RFC 1323.
//  - flow control: finite receive buffer size (initiated by the parameter
//    `advertisedWindow`). If the receive buffer is exhausted (by out-of-order
//    segments) and the payload length of a newly received segment
//    is higher than free receiver buffer, the new segment will be dropped.
//    Such drops are recorded in `tcpRcvQueueDropsVector`.
//  - Path MTU Discovery (RFC 1191, RFC 1981): when enabled, the DF bit is set
//    on IPv4 segments. Upon receiving an ICMPv4 Fragmentation Needed or ICMPv6
//    Packet Too Big error, the MSS is reduced accordingly and the offending
//    segment is retransmitted. After a configurable timeout (`pmtudTimeout`),
//    the original MSS is restored to probe for an increased path MTU.
//  - Explicit Congestion Notification (RFC 3168): ECN-setup during the
//    three-way handshake, ECE/CWR flag handling, and CE codepoint processing.
//  - Data Center TCP (RFC 8257): DCTCP congestion control based on ECN
//    feedback, implemented in the `DcTcp` algorithm class.
//
// The `TcpNewReno`, `TcpReno`, and `TcpTahoe` algorithms implement:
//  - RFC 1122 - delayed ACK algorithm (optional) with 200ms timeout
//  - RFC 896 - Nagle's algorithm (optional)
//  - Jacobson's and Karn's algorithms for round-trip time measurement and
//    adaptive retransmission
//  - `TcpTahoe` (Fast Retransmit), `TcpReno` (Fast Retransmit and Fast Recovery),
//    `TcpNewReno` (Fast Retransmit and Fast Recovery)
//  - RFC 3390 - Increased Initial Window (optional) integrated into `TcpBaseAlg`
//    (can be used for `TcpNewReno`, `TcpReno`, `TcpTahoe`, and `TcpNoCongestionControl` but not
//    for `DumbTcp`).
//  - RFC 3042 - Limited Transmit algorithm (optional) integrated into `TcpBaseAlg`
//    (can be used for `TcpNewReno`, `TcpReno`, `TcpTahoe`, and `TcpNoCongestionControl` but not
//    for `DumbTcp`).
//
// Additional congestion control algorithms: `TcpVegas`, `TcpWestwood`, `DcTcp`.
//
// Missing bits:
//  - URG and PSH bits not handled. Receiver always acts as if PSH was set
//    on all segments: always forwards data to the app as soon as possible.
//  - no RECEIVE command. Received data is always forwarded to the app as
//    soon as possible, as if the app issued a very large RECEIVE request
//    at the beginning. This means there's currently no flow control
//    between Tcp and the app.
//  - all timeouts are precisely calculated: timer granularity (which is caused
//    by "slow" and "fast" i.e. 500ms and 200ms timers found in many *nix Tcp
//    implementations) is not simulated
//
// `TcpNewReno`, `TcpReno`, and `TcpTahoe` issues and missing features:
//  - KEEP-ALIVE not implemented (idle connections never time out)
//  - Nagle's algorithm (RFC 896) possibly not precisely implemented
//
// The above problems should be relatively easy to fix.
//
simple Tcp extends SimpleModule like ITcp
{
    parameters:
        @class(Tcp);
        string checksumMode @enum("declared", "computed") = default("declared");
        int advertisedWindow @mutable = default(14 * this.mss); // In bytes, corresponds with the maximal receiver buffer capacity (Note: normally, NIC queues should be at least this size). @mutable so a harness/app can model setsockopt(SO_RCVBUF) by updating it before a connection is configured.
        bool delayedAcksEnabled = default(false); // Delayed ACK algorithm (RFC 1122) enabled/disabled
        bool nagleEnabled = default(true); // Nagle's algorithm (RFC 896) enabled/disabled
        bool limitedTransmitEnabled = default(false); // Limited Transmit algorithm (RFC 3042) enabled/disabled (can be used for TcpReno/TcpTahoe/TcpNewReno/TcpNoCongestionControl)
        bool increasedIWEnabled = default(false); // Increased Initial Window (RFC 3390) enabled/disabled
        bool sackSupport = default(false); // Selective Acknowledgment (RFC 2018, 2883, 3517) support (header option) (SACK will be enabled for a connection if both endpoints support it)
        bool windowScalingSupport = default(false); // Window Scale (RFC 1323) support (header option) (WS will be enabled for a connection if both endpoints support it)
        int windowScalingFactor @mutable = default(-1); // Window Scaling Factor given as a shift count. Valid values are 0..14, and -1 for automatic selection (it chooses the smallest shift count that makes advertisedWindow representable in 16 bits). @mutable so a harness/app modeling setsockopt(SO_RCVBUF) can switch to auto (-1) at runtime.
        bool timestampSupport @mutable = default(false); // Timestamps (RFC 1323) support (header option) (TS will be enabled for a connection if both endpoints support it)
        int mss = default(536); // Maximum Segment Size (header option). Default 536 per RFC 1122.
        int msl @unit(s) = default(120s);   // Maximum Segment Lifetime
        string tcpAlgorithmClass @examples("TcpVegas", "TcpWestwood", "DcTcp", "TcpNewReno", "TcpReno", "TcpTahoe", "TcpNoCongestionControl") = default("TcpReno");
        int dupthresh = default(3); // Used for TcpTahoe, TcpReno, and SACK (RFC 3517) DO NOT change unless you really know what you are doing
        int initialSsthresh = default(0xFFFFFFFF); // Initial value for Slow Start threshold used in TahoeRenoFamily. The initial value of ssthresh SHOULD be set arbitrarily high (e.g., to the size of the largest possible advertised window) Without user interaction there is no limit...
        double stopOperationExtraTime @unit(s) = default(0s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        bool ecnWillingness = default(false); // True if willing to use ECN
        double dctcpGamma = default(0.0625); // A fixed estimation gain for calculating dctcp_alpha (RFC 8257 4.2)
        bool pmtudEnabled = default(false); // Path MTU Discovery (RFC 1191, RFC 1981) enabled/disabled
        double pmtudTimeout @unit(s) = default(600s); // Time after which the original MSS is restored to probe for increased path MTU (RFC 1191, Section 6.3)
        @display("i=block/wheelbarrow");
        @signal[tcpConnectionAdded];
        @signal[tcpConnectionRemoved];
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetSentToLower](type=cPacket);
        @signal[packetDropped](type=inet::Packet);
        // parameters of: tcp: add TcpCubic (RFC 9438) with HyStart
        double cubicBeta = default(0.7); // multiplicative-decrease factor applied to cwnd on loss. Note that the TCP-friendliness estimator keeps the kernel's fixed scale (derived from beta=717/1024), so changing this only alters the decrease itself
        double cubicC = default(0.4); // scaling constant C of the cubic window-growth curve
        int cubicCntClamp = default(20); // cap on the per-increment ACK count while no W_max is known yet (before the first loss)
        double cubicDelta @unit(s) = default(10ms); // settling time after a window reduction, during which RTT samples are ignored
        bool cubicFastConvergence = default(true); // on a loss below the previous W_max, release extra window so a newly arrived competing flow can grow (RFC 9438 section 4.7)
        bool cubicTcpFriendliness = default(true); // never grow slower than an AIMD(1, beta) Reno flow would (RFC 9438 section 4.3)
        double hystartAckDelta @unit(s) = default(2ms); // largest ACK spacing still counted as part of an ACK train
        double hystartDelayMax @unit(s) = default(1s); // upper clamp on the HyStart delay-increase threshold
        double hystartDelayMin @unit(s) = default(4ms); // lower clamp on the HyStart delay-increase threshold
        int hystartDetect = default(3); // HyStart detectors, as a bitmask: 1 = ACK train, 2 = delay increase, 3 = both
        bool hystartEnabled = default(true); // HyStart: leave slow start when the path appears full, instead of overshooting into loss
        int hystartLowWindow = default(16); // smallest cwnd (in segments) at which HyStart starts looking for the exit point
        // parameters of: tcp: keepalive probing (RFC 1122 4.2.3.6)
        bool keepAliveEnabled = default(false); // TCP keepalive (RFC 1122 4.2.3.6). Off by default, like SO_KEEPALIVE on Linux
        double keepAliveIdleTime @unit(s) = default(7200s); // idle time before the first keepalive probe (Linux tcp_keepalive_time)
        double keepAliveInterval @unit(s) = default(75s); // interval between keepalive probes (Linux tcp_keepalive_intvl)
        int keepAliveProbeCount = default(9); // number of unanswered probes before the connection is aborted (Linux tcp_keepalive_probes)
        // parameters of: tcp: size segments against the space options actually leave
        int accEcnOptionBeaconAcks = default(4); // Send the AccECN option at least once every N ACKs even when not otherwise demanded (simplified beaconing policy -- Linux's full demand/beacon/DSACK-triggered state machine is not ported)
        bool accEcnOptionEnabled @mutable = default(false); // Send/parse the AccECN TCP option (E0B/E1B/CEB byte counters). Requires tcpEcnMode in {"accecn","accecn-passive"}
        bool accEcnOptionKindAlternates = default(true); // If true, alternate the AccECN option between kind 172 (E0B,CEB,E1B) and 174 (E1B,CEB,E0B) on successive sends (draft middlebox-robustness). If false, always emit kind 174 -- Linux's observed behavior.
        bool adaptiveDelayedAcks = default(false); // Linux-shaped receiver ACK dynamics on top of delayedAcksEnabled: a quickack budget (immediate ACKs at connection start, after out-of-order data, and after long idle), an ADAPTIVE delayed-ACK timeout (ATO: 40ms floor, tracks the inter-segment arrival gap, bounded by srtt and 200ms) instead of the fixed 200ms timer, and an interactive "pingpong" mode. Off = the classic fixed-timeout RFC 1122 behavior
        bool adaptiveReorderingEnabled = default(true); // Grow the reordering degree (dynamic DupThresh) when SACK reveals reordering, so persistent reordering stops causing spurious fast retransmits. Requires sackSupport.
        int delayedAckFrameCount = default(2); // number of frames after delayed acks are sent
        bool dsackEnabled = default(this.sackSupport); // Selective Acknowledgment (RFC 2018, 2883, 6675) support (header option) (SACK will be enabled for a connection if both endpoints support it)
        bool fastopenAcceptWithoutCookie @mutable = default(false); // Server role: accept SYN-with-data even when the SYN carries no Fast Open cookie option at all (RFC 7413 section 4.1.3's cookie-less mode). ~ sysctl tcp_fastopen bit 9 (0x200, TFO_SERVER_COOKIE_NOT_REQD). Only meaningful with fastopenServerEnabled.
        bool fastopenClientEnabled @mutable = default(false); // TCP Fast Open (RFC 7413) client role: attempt SYN-with-data when a cookie is cached. ~ sysctl tcp_fastopen bit 0 (TFO_CLIENT_ENABLE)
        bool fastopenClientNoCookieRequired @mutable = default(false); // Linux net.ipv4.tcp_fastopen bit 0x4 (TFO_CLIENT_NO_COOKIE): the client sends SYN+data immediately with NO cookie (and no cookie-request) at all -- for networks where the server is known to accept cookie-less TFO
        int fastopenCookieBytes = default(8); // Length in bytes of the locally generated Fast Open cookie (RFC 7413 allows 4-16)
        bool fastopenExpOptionEnabled @mutable = default(false); // Accept the legacy kind-253/254+magic-0xF989 experimental Fast Open option on receive (never emitted); inert unless fastopenServerEnabled or fastopenClientEnabled is also true
        bool fastopenLenientCookieValidation = default(true); // Accept any syntactically valid (4-16 byte) Fast Open cookie as valid without a keyed-hash check -- simulation-appropriate (no adversarial clients on the wire); set false to require an exact match against the locally generated cookie
        bool fastopenServerEnabled @mutable = default(false); // TCP Fast Open (RFC 7413) server role: accept and accelerate a validated-cookie SYN-with-data. ~ sysctl tcp_fastopen bit 1 (TFO_SERVER_ENABLE)
        bool frtoEnabled = default(true); // F-RTO spurious-RTO detection (RFC 5682, SACK-enhanced variant): after a retransmission timeout, an ACK/SACK covering data that was never retransmitted proves the original flight arrived and the timeout was spurious -- the pre-RTO cwnd/ssthresh are restored. ~ sysctl tcp_frto=2 (the Linux default)
        volatile int initialSendSequenceNumber = default(-1); // -1 means the value is calculated from the current simulation time
        string initialWindow @enum("rfc2001","rfc3390","rfc6928") = default("rfc6928"); // initial congestion window: rfc2001=1 SMSS, rfc3390=min(4*MSS,max(2*MSS,4380)), rfc6928=IW10=min(10*MSS,max(2*MSS,14600))
        string lossDetectionMode @enum("dupthresh","rack") = default("rack"); // SACK loss detection: classic DupThresh (RFC 6675) or RACK time-based (RFC 8985). RACK requires sackSupport.
        bool lossUndoEnabled = default(true); // Undo a spurious cwnd/ssthresh reduction when D-SACK (RFC 2883) or timestamps (RFC 3522 Eifel) reveal the retransmission was unnecessary.
        int maxReordering = default(300); // Upper bound on the learned reordering degree in segments (Linux tcp_max_reordering).
        bool prrEnabled = default(true); // Proportional Rate Reduction (RFC 6937) during fast recovery instead of classic cwnd inflation/halving. Requires sackSupport.
        double receiveBufferSize @unit(B) = default(-1B) @mutable; // receive-BUFFER capacity, decoupled from the advertised window (Linux sk_rcvbuf, initialized from tcp_rmem[1] / SO_RCVBUF): out-of-order data is buffered up to this limit even when it exceeds the advertised window. -1 = same as the advertised window (historical INET behavior)
        bool seedRttFromHandshake = default(true); // Seed the RTT estimator (srtt/rttvar/RTO) from the SYN<->SYN-ACK exchange, as Linux does. The first-flight RTO and TLP probe timeout become RTT-scaled instead of initialRto.
        bool syncookiesAlways = default(false); // model Linux sysctl tcp_syncookies=2 (cookies on every passive open): the server's connection is rebuilt from the cookie, whose 2-bit MSS field quantizes the peer's advertised MSS DOWN to the IPv4 msstab {536, 1300, 1440, 1460} (net/ipv4/syncookies.c); other option state (wscale/SACK/TS) survives via the timestamp encoding and stays exact
        string tcpEcnMode @mutable @enum("off","passive","rfc3168","accecn","accecn-passive") = default("off"); // @mutable so a harness can model a mid-run "sysctl net.ipv4.tcp_ecn=N": the value is read per connection at open time, exactly as Linux samples it
        bool tlpEnabled = default(true); // Tail Loss Probe (RFC 8985 section 7.2): probe ~2*SRTT after the last transmission so a lost tail is repaired without waiting for the RTO; requires SACK. ~ Linux sysctl tcp_early_retrans=3 (its default)
        bool windowAutoTuning = default(false); // receiver window auto-tuning (Linux tcp_grow_window): the offered window starts at advertisedWindow and grows with received data toward a clamp derived from receiveBufferSize. Requires receiveBufferSize to be set.
        bool windowShrinkAllowed = default(false); // model Linux sysctl tcp_shrink_window=1: the advertised window follows the receive buffer's genuine free space (__tcp_select_window's shrink branch) and may move the right edge DOWN as unread data accumulates -- occupancy is accounted at Linux's skb-truesize granularity and scaled by the measured payload/truesize ratio (tcp_scaling_ratio), so unread bytes cost what they cost a real socket buffer. Needs receiveBufferSize and an application that leaves data unread (e.g. a packetdrill explicit-read run) to differ from the default no-shrink policy
        // parameters of: tcp: judge loss by transmission time (RFC 8985)
        double initialRto @unit(s) = default(1s); // initial retransmission timeout, used until the first RTT measurement completes (RFC 6298 section 2.1 specifies 1s; 3s is the historical RFC 1122 value)
        double rackReoTimerGranularity @unit(s) = default(0s); // model the kernel's jiffy-quantized REO timeout arming (Linux usecs_to_jiffies(timeout)+1: delay rounded UP to this granularity plus one extra tick, e.g. 4ms for an HZ=250 kernel); a sub-tick RACK deadline then fires whole ticks later, letting a dupthresh-timed SACK enter recovery first, as real kernels do. 0 = exact (default INET behavior)
        int synRetries @mutable = default(-1); // Linux net.ipv4.tcp_syn_retries / per-socket TCP_SYNCNT: maximum number of SYN (and SYN-ACK) retransmissions before the connection attempt is aborted; -1 = INET's historical MAX_SYN_REXMIT_COUNT (12). @mutable so a test harness can inject the per-socket sockopt at runtime.
        // parameters of: tcp: TCP Fast Open (RFC 7413)
        bool alignOptions = default(true);  // enable/disable per-option predefined alignment
        double fastopenBlackholeTimeout @unit(s) = default(0s); // Client-side active-TFO blackhole detection: base disable duration after a suspected middlebox interference event (RTO or out-of-order RST on a data-carrying TFO SYN), doubling (capped at 64x) on repeated events. 0s (default) disables the detector entirely, matching Linux's own shipped default -- the local corpus's defaults.sh override to a nonzero value is a *test* configuration, not upstream's
        int fastopenCookieCacheSize = default(1024); // Upper bound on the number of distinct destinations the client-role Fast Open cookie cache retains; evicts an arbitrary (not LRU) entry once full
        string fastopenKey @mutable = default(""); // Server-role Fast Open cookie key in Linux net.ipv4.tcp_fastopen_key format ("xxxxxxxx-xxxxxxxx-xxxxxxxx-xxxxxxxx", primary key only). When set, the 8-byte cookie is derived exactly as Linux does (SipHash-2-4 of source||destination IPv4 address under this key), so cookies are deterministic and interoperable with captures; when empty (default), a simulation-local keyed hash is used instead
        bool sendMssOption = default(true);  // enable/disable MSS option sending
        // parameters of: tcp: adapt the acknowledgement delay to the connection
        double delayedAckTimeout @unit(s) = default(200ms); // maximum time to wait before sending a delayed ACK
        double maxPersistTimeout @unit(s) = default(60s); // maximum persist timeout
        int maxRexmitCount = default(12); // maximum retransmission count
        double maxRexmitTimeout @unit(s) = default(120s); // maximum retransmission timeout
        double minPersistTimeout @unit(s) = default(5s); // minimum persist timeout
        double minRexmitTimeout @unit(s) = default(0.2s); // minimum retransmission timeout
        bool sendDataWithFirstAck = default(true); // send data in 3rd packet in SYN - SYN+ACK - ACK sequence
        // parameters of: tcp: retransmission, persist and handshake timing
        int synLinearTimeouts = default(4); // Linux net.ipv4.tcp_syn_linear_timeouts: number of initial CLIENT SYN retransmits fired at the (non-backed-off) initial RTO before exponential backoff begins; 0 = double from the first retransmit
    gates:
        input appIn @labels(TcpCommand/down) @messageKinds(inet::TcpCommandCode);
        input ipIn @labels(TcpHeader,Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output appOut @labels(TcpCommand/up) @messageKinds(inet::TcpStatusInd);
        output ipOut @labels(TcpHeader,Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}

