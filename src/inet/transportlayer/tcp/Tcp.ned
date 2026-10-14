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
//   -# The `initialWindow` parameter selects the initial congestion window: one
//      segment (`rfc2001`), the upper bound of RFC 5681 section 3.1, which is
//      that of RFC 3390 (`rfc3390`, min(4*SMSS, max(2*SMSS, 4380 bytes))), or the
//      ten segments of RFC 6928 (`rfc6928`, min(10*SMSS, max(2*SMSS, 14600
//      bytes))), which Linux uses. The `initialCwnd` parameter gives a number of
//      segments instead, as the `initcwnd` of a Linux route. The deprecated
//      `increasedIWEnabled` parameter selects `rfc3390`.
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
//   -# The `tcpEcnMode` parameter enables Explicit Congestion Notification
//      (RFC 3168). ECN is negotiated during the three-way handshake and is
//      only active if one endpoint requests it ("rfc3168") and the other one
//      answers ("rfc3168" or "passive"). The modes "accecn" and
//      "accecn-passive" add Accurate ECN (RFC 9768), which feeds back the
//      number of CE marks instead of one bit for each round trip. The AccECN
//      option (`accEcnOptionEnabled`, `accEcnOptionPolicy`) gives the bytes of
//      each ECN codepoint. `ecnMarkAll` sets ECT also on the control segments
//      of an AccECN or DCTCP connection, as Linux does. When using the `DcTcp`
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
//   - RFC 8311 - Relaxing Restrictions on Explicit Congestion Notification (ECN)
//                Experimentation
//   - RFC 9768 - More Accurate Explicit Congestion Notification (AccECN) Feedback
//                in TCP
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
//      - AccECN0, AccECN1: the AccECN option, based on RFC 9768.
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
//  - Accurate ECN (RFC 9768): the AccECN handshake, the ACE field, which counts
//    the CE-marked packets, and the AccECN option, which counts the bytes of
//    each codepoint; with the fallback of a retransmitted SYN and SYN-ACK.
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
        int advertisedWindow = default(65535); // In bytes, corresponds with the maximal receiver buffer capacity (Note: normally, NIC queues should be at least this size)
        double receiveBufferSize @unit(B) = default(-1B); // the receive buffer, apart from the advertised window (Linux sk_rcvbuf): the unread data and the out-of-order data use it, and the offered window is the smaller of its free space and the advertised window less the unread data; -1B: the buffer is the advertised window, as before
        bool acceptBeyondBufferAtEmptyQueue = default(false); // accept a whole in-order segment that reaches beyond the receive buffer when the receive queue is empty, as Linux does (tcp_data_queue()); the window is then zero until the application reads. false: drop such a segment
        bool windowAutoTuning = default(false); // receive window auto-tuning, as Linux (tcp_grow_window()): the offered window starts at advertisedWindow and grows with each received data segment toward a clamp of half the receive buffer; needs receiveBufferSize
        bool windowShrinkAllowed = default(false); // the offered window follows the free receive buffer and may move its right edge back, as Linux with tcp_shrink_window=1: the buffer is charged with the truesize of the skb of each segment, and the free buffer counts at the measured ratio of payload to truesize; the first offer is half the buffer. RFC 9293 says that a receiver SHOULD NOT shrink the window (SHLD-14)
        bool windowUpdateAfterRead = default(false); // true: as Linux (__tcp_cleanup_rbuf()), a READ after which the receiver can offer at least twice the window that is still open sends that window at once in an ACK; false: the new window goes with the next ACK, as the receiver SWS avoidance and the delayed ACK of RFC 9293 (section 3.8.6.2.2) decide
        int notsentLowat @unit(B) = default(-1B); // TCP_NOTSENT_LOWAT (a Linux socket option, no RFC): a low-water mark on the unsent bytes: when they exceed it, TCP sends no TCP_I_SEND_MSG until they are at or below it again, and then it sends one; TcpSocket::setNotsentLowat() sets it for one connection; -1B: off
        bool delayedAcksEnabled = default(true); // Delayed ACK algorithm (RFC 1122) enabled/disabled
        bool nagleEnabled = default(true); // Nagle's algorithm (RFC 896) enabled/disabled
        bool limitedTransmitEnabled = default(true); // Limited Transmit algorithm (RFC 3042) enabled/disabled (can be used for TcpReno/TcpTahoe/TcpNewReno/TcpNoCongestionControl)
        bool increasedIWEnabled = default(false); // deprecated: the same as initialWindow = "rfc3390" (RFC 3390); an error with initialWindow = "rfc6928"
        volatile int initialSendSequenceNumber = default(-1); // the initial send sequence number (ISS) of each connection; volatile, so a distribution gives each connection its own value; -1: from the clock, one tick in 4 microseconds (RFC 9293 section 3.4.1)
        string initialWindow @enum("rfc2001","rfc3390","rfc6928") = default("rfc3390"); // the initial congestion window: "rfc2001" = 1 SMSS; "rfc3390" = min(4*SMSS, max(2*SMSS, 4380 bytes)), the upper bound of RFC 5681 section 3.1; "rfc6928" = min(10*SMSS, max(2*SMSS, 14600 bytes)), as Linux
        int initialCwnd = default(0); // the initial congestion window in segments, in place of initialWindow, as the initcwnd of a Linux route (dst_metric(RTAX_INITCWND)); 0: initialWindow
        bool sackSupport = default(true); // Selective Acknowledgment (RFC 2018, 2883, 3517) support (header option) (SACK will be enabled for a connection if both endpoints support it)
        bool windowScalingSupport = default(true); // Window Scale (RFC 1323) support (header option) (WS will be enabled for a connection if both endpoints support it)
        int windowScalingFactor = default(-1); // Window Scaling Factor given as a shift count. Valid values are 0..14, and -1 for automatic selection (it chooses the smallest shift count that makes advertisedWindow representable in 16 bits)
        bool timestampSupport @mutable = default(true); // Timestamps (RFC 1323) support (header option) (TS will be enabled for a connection if both endpoints support it). Each connection reads it at its open, so a run can change it as the host-wide sysctl net.ipv4.tcp_timestamps
        int mss = default(-1); // Maximum Segment Size (header option): the largest segment text that this host sends and announces. -1: the MSS of the address family for an MTU of 1500 bytes, 1460 for IPv4 and 1440 for IPv6, when the peer is known
        int msl @unit(s) = default(120s);   // Maximum Segment Lifetime
        string tcpAlgorithmClass @examples("TcpCubic", "TcpVegas", "TcpWestwood", "DcTcp", "TcpNewReno", "TcpReno", "TcpTahoe", "TcpNoCongestionControl") = default("TcpNewReno");
        int dupthresh = default(3); // Used for TcpTahoe, TcpReno, and SACK (RFC 3517) DO NOT change unless you really know what you are doing
        int initialSsthresh = default(0xFFFFFFFF); // Initial value for Slow Start threshold used in TahoeRenoFamily. The initial value of ssthresh SHOULD be set arbitrarily high (e.g., to the size of the largest possible advertised window) Without user interaction there is no limit...
        double stopOperationExtraTime @unit(s) = default(0s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        string tcpEcnMode @mutable @enum("off","passive","rfc3168","accecn","accecn-passive") = default("off"); // ECN: "off": no ECN; "passive": answer an ECN-setup SYN (RFC 3168), but request no ECN in an active open (Linux tcp_ecn=2, its default); "rfc3168": request and answer ECN; "accecn": request Accurate ECN (RFC 9768), and answer it and ECN; "accecn-passive": answer Accurate ECN and ECN, but request nothing. Each connection reads it at its open
        bool ecnWillingness = default(false); // deprecated: true is tcpEcnMode="rfc3168"
        bool ecnMarkAll = default(false); // ECN: ECT also on the pure ACKs, FINs, retransmissions and window probes of an AccECN connection (RFC 9768) and of a DCTCP connection, as Linux does (tcp_ecn_send()); RFC 8257 section 4.1 recommends such a knob for DCTCP. false: ECT only on new data, as RFC 3168 sections 6.1.4 to 6.1.6 ask, which RFC 9768 section 4 keeps
        double dctcpGamma = default(0.0625); // A fixed estimation gain for calculating dctcp_alpha (RFC 8257 4.2)
        bool pmtudEnabled = default(false); // Path MTU Discovery (RFC 1191, RFC 1981) enabled/disabled
        double pmtudTimeout @unit(s) = default(600s); // Time after which the original MSS is restored to probe for increased path MTU (RFC 1191, Section 6.3)
        double cubicBeta = default(0.7); // multiplicative-decrease factor applied to cwnd on loss. Note that the TCP-friendliness estimator keeps the kernel's fixed scale (derived from beta=717/1024), so changing this only alters the decrease itself
        double cubicC = default(0.4); // scaling constant C of the cubic window-growth curve
        int cubicCntClamp = default(20); // cap on the per-increment ACK count while no W_max is known yet (before the first loss)
        double cubicDelta @unit(s) = default(10ms); // settling time after a window reduction, during which RTT samples are ignored
        bool cubicFastConvergence = default(true); // on a loss below the previous W_max, release extra window so a newly arrived competing flow can grow (RFC 9438 section 4.7)
        bool cubicTcpFriendliness = default(true); // never grow slower than an AIMD(1, beta) Reno flow would (RFC 9438 section 4.3)
        double hystartAckDelta @unit(s) = default(2ms); // largest ACK spacing still counted as part of an ACK train
        double hystartDelayMax @unit(s) = default(16ms); // upper clamp on the same threshold (Linux HYSTART_DELAY_MAX)
        double hystartDelayMin @unit(s) = default(4ms); // lower clamp on the HyStart delay-increase threshold (an eighth of the minimum RTT)
        int hystartDetect = default(3); // HyStart detectors, as a bitmask: 1 = ACK train, 2 = delay increase, 3 = both
        bool hystartEnabled = default(true); // HyStart: leave slow start when the path appears full, instead of overshooting into loss
        int hystartLowWindow = default(16); // smallest cwnd (in segments) at which HyStart starts looking for the exit point
        bool adaptiveReorderingEnabled = default(false); // Grow the reordering degree (dynamic DupThresh) when SACK reveals reordering, so persistent reordering stops causing spurious fast retransmits. Requires sackSupport.
        bool dsackEnabled = default(this.sackSupport); // report duplicate segments by D-SACK blocks (RFC 2883), when SACK is in use
        string lossDetectionMode @enum("dupthresh","rack") = default("dupthresh"); // SACK loss detection: classic DupThresh (RFC 6675) or RACK time-based (RFC 8985). RACK requires sackSupport.
        bool frtoEnabled = default(false); // F-RTO spurious-RTO detection (RFC 5682, SACK-enhanced; Linux tcp_frto=2). Requires sackSupport.
        bool lossUndoEnabled = default(false); // Undo a spurious cwnd/ssthresh reduction when D-SACK (RFC 2883) or timestamps (RFC 3522 Eifel) reveal the retransmission was unnecessary.
        int maxReordering = default(300); // Upper bound on the learned reordering degree in segments (Linux tcp_max_reordering).
        bool prrEnabled = default(false); // Proportional Rate Reduction (RFC 6937) during fast recovery instead of classic cwnd inflation/halving. Requires sackSupport.
        bool abortOnDataAfterClose = default(false); // model the full close of Linux close(): a CLOSE without halfClose also shuts the receive side down, and new data after it resets the connection (RFC 1122 section 4.2.2.13); false: RFC 9293 CLOSE, after which the application may still receive
        bool accEcnBareTimeWaitAck = default(false); // AccECN: the ACKs of TIME_WAIT carry no ACE field, as in Linux, whose time-wait socket sends them (tcp_v4_send_ack()); false: they carry it, as each ACK after the handshake (RFC 9768 section 3.2.2.2: MUST)
        bool accEcnOptionEnabled @mutable = default(true); // AccECN: send the AccECN option, which gives the byte counts of each ECN codepoint (RFC 9768 section 3.2.3: RECOMMENDED); false: only the ACE field (Linux tcp_ecn_option=0). Each connection reads it at its open
        string accEcnOptionPolicy @enum("rfc","linux") = default("rfc"); // AccECN: when the option goes out and which fields it has. "rfc": the recommended scheme of RFC 9768 section 3.2.3.3: after a byte count changed, with each field that ever changed, in the order that needs fewer fields, and with each field on the SYN-ACK and on the first segments of the client (section 3.2.3.2.1); "linux": as Linux tcp_ecn_option=2: on each segment, kind 174 with each field that fits, but only after the peer sent an option (section 3.2.3.2.3 asks to send it also then)
        bool adaptiveDelayedAcks = default(false); // with delayedAcksEnabled, the delayed ACK of Linux: a quickack budget (ACKs at once at the start of a connection, after out-of-order data and after a long pause), an adaptive delay (ATO: from 40 ms, after the gaps between arrivals, at most the SRTT and 200 ms) in place of delayedAckTimeout, and a pingpong mode, which delays the ACKs of interactive traffic; false: the delayed ACK of RFC 9293 section 3.8.6.3 (delayedAckTimeout, delayedAckFrameCount)
        bool abortRstWithAck = default(false); // true: the RST of an abort carries the ACK bit and RCV.NXT, as Linux (tcp_send_active_reset()); false: <SEQ=SND.NXT><CTL=RST> of RFC 9293
        bool alignOptions = default(true); // align each TCP option with NOP options before it, as before; false: no NOPs, and End of Option List bytes pad the option area (for peers that compare the exact option layout)
        int baseMss = default(1024); // with mtuProbing: the lower end of the path MTU search, the payload of a segment that the path surely carries (Linux tcp_base_mss)
        int delayedAckFrameCount = default(2); // with delayedAcksEnabled, the ACK goes at once when this number of full-sized segments waits for it (RFC 9293 SHLD-19: an ACK for at least each second full-sized segment)
        double delayedAckTimeout @unit(s) = default(200ms); // with delayedAcksEnabled, the longest delay of an ACK (RFC 9293 MUST-40: less than 0.5 s)
        bool dropBeyondWindowEdge = default(false); // true: as Linux (tcp_sequence()), a segment whose data ends beyond the highest window edge offered is dropped whole, unless it starts within that edge and the in-order receive queue is empty; false: the window test of RFC 9293 section 3.10.7.4, after which only the receive buffer limits the data
        bool fastopenAcceptWithoutCookie @mutable = default(false); // TCP Fast Open, server: accept the data of a SYN without a valid cookie, if the SYN has data or a Fast Open option, and give no cookie (Linux tcp_fastopen bit 0x200); RFC 7413 requires a valid cookie
        double fastopenBlackholeTimeout @unit(s) = default(3600s); // TCP Fast Open, client: when a SYN with a Fast Open option or data is sent the third time (a negative response, RFC 7413 section 4.1.3.1), no connection uses Fast Open for this time, which doubles at each further blackhole, up to 64 times (as Linux); 0s: no detection (the Linux default since 5.14)
        bool fastopenClientEnabled @mutable = default(false); // TCP Fast Open (RFC 7413), client: a connect() with fastOpen requests a cookie from the server, and with a cached cookie it sends its first data on the SYN (~ Linux tcp_fastopen bit 0x1)
        bool fastopenClientNoCookieRequired @mutable = default(false); // TCP Fast Open, client: send the data on the SYN without a cookie and without a Fast Open option (Linux tcp_fastopen bit 0x4), for a server that accepts it; RFC 7413 requires a cookie
        int fastopenCookieBytes = default(8); // TCP Fast Open, server: the length of the cookies that this server gives (RFC 7413: an even number from 4 to 16)
        int fastopenCookieCacheSize = default(1024); // TCP Fast Open, client: the number of servers whose cookie is kept; a new server then replaces an arbitrary entry
        bool fastopenExpOptionEnabled @mutable = default(false); // TCP Fast Open: also use the experimental option (kind 254 with the magic number 0xF989, RFC 7413 appendix A), as Linux: the server answers it in the same option, and the client tries it once when a cookie request had no answer; false: only kind 34, which RFC 7413 section 9 says implementations SHOULD use
        string fastopenKey @mutable = default(""); // TCP Fast Open, server: the key of the cookies, in the format of Linux net.ipv4.tcp_fastopen_key ("xxxxxxxx-xxxxxxxx-xxxxxxxx-xxxxxxxx", the primary key); a cookie of 8 bytes is then the cookie of Linux with this key; empty: a key from the RNG
        bool fastopenLenientCookieValidation = default(false); // TCP Fast Open, server: accept each cookie of a valid length without a check; false: accept only the cookie that this server gives the address of the client (RFC 7413 section 4.2.2)
        int fastopenMaxPendingRequests = default(16); // TCP Fast Open, server: the largest number of Fast Open connections of a local port in SYN_RCVD (PendingFastOpenRequests, RFC 7413 section 4.2; Linux takes it from the TCP_FASTOPEN socket option of the listener); above it, a SYN gets no Fast Open: no cookie, and its data is not acknowledged
        bool fastopenServerEnabled @mutable = default(false); // TCP Fast Open (RFC 7413), server: give cookies, and deliver the data of a SYN with a valid cookie to the application before the handshake completes (~ Linux tcp_fastopen bit 0x2)
        bool icmpSequenceCheck = default(false); // true: an ICMP error whose quoted TCP segment has a sequence number outside SND.UNA to SND.MAX is ignored as stale or forged, as Linux (tcp_v4_err(), tcp_v6_err()) and the BSDs do and RFC 5927 section 4.1 describes; false: each ICMP error of the connection counts, as RFC 9293 section 3.9.2.2 says
        double initialRto @unit(s) = default(3s); // the retransmission timeout before the first RTT measurement, also the first timeout of a SYN and of a SYN-ACK (RFC 6298 section 2.1: 1 s; 3 s is the value of RFC 1122)
        bool keepAliveEnabled = default(false); // keepalive (RFC 9293 section 3.8.4): after keepAliveIdleTime without a received segment and without data in flight, send a probe without data (SEG.SEQ = SND.UNA - 1), again each keepAliveInterval, and abort the connection after keepAliveProbeCount probes without an answer; off by default (MUST-25); TcpSocket::setKeepAlive() turns it on or off for one connection (MUST-24)
        double keepAliveIdleTime @unit(s) = default(7200s); // the idle time before the first keepalive probe (RFC 9293 MUST-28: at least two hours; Linux tcp_keepalive_time)
        double keepAliveInterval @unit(s) = default(75s); // the time between two keepalive probes (Linux tcp_keepalive_intvl)
        int keepAliveProbeCount = default(9); // the keepalive probes without an answer, after which the connection aborts (Linux tcp_keepalive_probes; RFC 9293 MUST-29: one probe without an answer is not a dead connection)
        double maxPersistTimeout @unit(s) = default(60s); // the upper limit of the interval of the zero-window probes, which starts at the RTO and doubles at each probe (RFC 9293 SHLD-29, SHLD-30)
        int maxRexmitCount = default(12); // the retransmissions of one segment, after which the connection aborts
        double maxRexmitTimeout @unit(s) = default(240s); // the upper limit of the retransmission timeout (RFC 6298 section 2.5: at least 60 s)
        double minRexmitTimeout @unit(s) = default(1s); // the lower limit of the retransmission timeout (RFC 6298 section 2.4: 1 s; Linux: 200 ms)
        bool minshallNagle = default(false); // with nagleEnabled, the variant of Minshall, as Linux (tcp_minshall_check()): a small segment waits only while an earlier small segment is not acknowledged, so a flight of full-sized segments does not hold the end of a write; false: the Nagle algorithm of RFC 9293 section 3.7.4, where a small segment waits while any data is not acknowledged
        int mtuProbing = default(0); // the path MTU search of RFC 4821, as Linux tcp_mtu_probing: 2 = from the start of each connection, the sender sends single segments larger than its MSS and raises the MSS when the peer acknowledges them; 0 and 1 = no search (1, the search after a black hole, needs a detector that INET does not have)
        int pathMtu = default(0); // with mtuProbing: the MTU of the route, the largest MTU that the search can give the connection (Linux dst_mtu()); 0 = the MTU of the negotiated MSS
        bool pushLastBufferedSegment = default(false); // set PSH on the last buffered segment, after which no queued data waits (RFC 9293 section 3.9.1.2, MUST-61), because the SEND of INET has no PUSH flag; pushSegmentsOnWriteBoundary replaces this rule with that of Linux
        bool pushSegmentsOnWriteBoundary = default(false); // set PSH as Linux: on the segment that ends a write without MSG_MORE (tcp_mark_push()), on the last segment of a GSO burst of new data (tcp_transmit_skb()), in a write longer than half the largest window (forced_push()), and on the segment that ends the data of a Fast Open SYN; false: PSH only on a partial segment that TCP_CORK or MSG_MORE held
        bool retainSackAfterRto = default(false); // false: a retransmission timeout clears the SACKed marks of the scoreboard (RFC 2018 section 8, RFC 5682 step 1); true: the marks survive the timeout, as Linux keeps them (RFC 6675 section 5.1 allows it)
        bool rfc5961AckCheck = default(false); // the ACK check of RFC 5961 section 5.2, which RFC 9293 allows (MAY-12) and Linux uses: a segment whose ACK lies below SND.UNA - MAX.SND.WND is dropped, and an ACK goes in reply; false: such an ACK counts as an old duplicate ACK
        bool rtoVarianceFloor = default(false); // false: RTO = SRTT + 4*RTTVAR, rounded up to minRexmitTimeout (RFC 6298 sections 2.3 and 2.4); true: RTO = SRTT + max(4*RTTVAR, minRexmitTimeout), as Linux (tcp_set_rto()), whose minimum limits the variance term
        bool seedRttFromHandshake = default(false); // Seed the RTT estimator (srtt/rttvar/RTO) from the SYN<->SYN-ACK exchange, as Linux does. The first-flight RTO becomes RTT-scaled instead of the initial RTO.
        bool sendDataWithFirstAck = default(true); // an active open sends data that waits at the end of the handshake with the ACK of the SYN-ACK; false: a bare ACK first, and then the data
        bool sendMssOption = default(true); // send the MSS option in the SYN and the SYN-ACK; false: leave it out, as some peers do (the other side then assumes the default send MSS of RFC 9293 MUST-15)
        int synLinearTimeouts = default(0); // the first retransmissions of a SYN that keep the initial timeout before it doubles, as Linux net.ipv4.tcp_syn_linear_timeouts (its default: 4): with 4 and an initial RTO of 1 s, the SYN timeouts are 1, 1, 1, 1, 1, 2, 4 s; 0: the timeout doubles at each expiry (RFC 6298 section 5.5)
        int synRetries = default(12); // the retransmissions of a SYN or of a SYN-ACK, after which the open fails (Linux net.ipv4.tcp_syn_retries, its default: 6; Linux has tcp_synack_retries for the SYN-ACK); an active open also fails after 75 s
        bool softIcmpErrorsAbortSetup = default(false); // true: a soft ICMP Destination Unreachable (IPv4 codes 0, 1 and 5, IPv6 codes 0 and 3) aborts a connection in SYN_SENT or SYN_RCVD, as Linux (tcp_v4_err(), tcp_v6_err()) and RFC 5461 section 4 describe; false: RFC 9293 MUST-56, a soft error never aborts a connection
        bool syncookiesAlways = default(false); // model Linux tcp_syncookies=2 (a SYN cookie on every passive open): the cookie quantizes the MSS of the peer down to the IPv4 msstab {536, 1300, 1440, 1460} of net/ipv4/syncookies.c; the other options stay exact
        bool tlpEnabled = default(false); // Tail Loss Probe (RFC 8985 section 7): about two SRTTs after the last transmission or ACK of new data, send one probe, so that its ACK starts the repair of a lost tail before the RTO. Requires sackSupport.
        bool tsRecentAfterAckCheck = default(false); // false: TS.Recent takes the TSval of each acceptable segment before its ACK is processed (RFC 7323 section 5.3, R3); true: not from a segment that acknowledges data that was not sent, as Linux, which updates TS.Recent only after the ACK passes its check (tcp_replace_ts_recent() in tcp_ack())
        double rackReoTimerGranularity @unit(s) = default(0s); // model the kernel's jiffy-quantized REO timeout arming (Linux usecs_to_jiffies(timeout)+1: delay rounded UP to this granularity plus one extra tick, e.g. 4ms for an HZ=250 kernel); a sub-tick RACK deadline then fires whole ticks later, letting a dupthresh-timed SACK enter recovery first, as real kernels do. 0 = exact (default INET behavior)
        string zeroWindowProbe @enum("rfc","linux") = default("rfc"); // the probe of a zero window. "rfc": the first unacknowledged octet, as RFC 9293 section 3.8.6.1 says ("at least one octet of new data (if available), or retransmit"); "linux": a segment without data with SEG.SEQ = SND.UNA - 1, which the receiver answers with an ACK, as Linux (tcp_xmit_probe_skb())
        @display("i=block/wheelbarrow");
        @signal[tcpConnectionAdded];
        @signal[tcpConnectionRemoved];
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetSentToLower](type=cPacket);
        @signal[packetDropped](type=inet::Packet);
    gates:
        input appIn @labels(TcpCommand/down) @messageKinds(inet::TcpCommandCode);
        input ipIn @labels(TcpHeader,Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output appOut @labels(TcpCommand/up) @messageKinds(inet::TcpStatusInd);
        output ipOut @labels(TcpHeader,Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}

