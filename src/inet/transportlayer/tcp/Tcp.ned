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
//  - CUBIC (RFC 9438): the `TcpCubic` algorithm class implements CUBIC window
//    growth with fast convergence, TCP friendliness, and the HyStart hybrid
//    slow-start exit (parameters `cubic*` and `hystart*`). It reuses `TcpReno`'s
//    fast retransmit and RFC 3517 SACK recovery.
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
        int advertisedWindow = default(14 * this.mss); // In bytes, corresponds with the maximal receiver buffer capacity (Note: normally, NIC queues should be at least this size)
        bool delayedAcksEnabled = default(false); // Delayed ACK algorithm (RFC 1122) enabled/disabled
        bool nagleEnabled = default(true); // Nagle's algorithm (RFC 896) enabled/disabled
        bool limitedTransmitEnabled = default(false); // Limited Transmit algorithm (RFC 3042) enabled/disabled (can be used for TcpReno/TcpTahoe/TcpNewReno/TcpNoCongestionControl)
        bool increasedIWEnabled = default(false); // DEPRECATED: use initialWindow="rfc3390" instead. Increased Initial Window (RFC 3390) enabled/disabled
        string initialWindow @enum("rfc2001","rfc3390","rfc6928") = default("rfc2001"); // initial congestion window: rfc2001=1 SMSS, rfc3390=min(4*MSS,max(2*MSS,4380)), rfc6928=IW10=min(10*MSS,max(2*MSS,14600))
        bool sackSupport = default(false); // Selective Acknowledgment (RFC 2018, 2883, 3517) support (header option) (SACK will be enabled for a connection if both endpoints support it)
        bool windowScalingSupport = default(false); // Window Scale (RFC 1323) support (header option) (WS will be enabled for a connection if both endpoints support it)
        int windowScalingFactor = default(-1); // Window Scaling Factor given as a shift count. Valid values are 0..14, and -1 for automatic selection (it chooses the smallest shift count that makes advertisedWindow representable in 16 bits)
        bool timestampSupport = default(false); // Timestamps (RFC 1323) support (header option) (TS will be enabled for a connection if both endpoints support it)
        int mss = default(536); // Maximum Segment Size (header option). Default 536 per RFC 1122.
        int msl @unit(s) = default(120s);   // Maximum Segment Lifetime
        string tcpAlgorithmClass @examples("TcpCubic", "TcpVegas", "TcpWestwood", "DcTcp", "TcpNewReno", "TcpReno", "TcpTahoe", "TcpNoCongestionControl") = default("TcpReno");
        int dupthresh = default(3); // Used for TcpTahoe, TcpReno, and SACK (RFC 3517) DO NOT change unless you really know what you are doing
        string lossDetectionMode @enum("dupthresh","rack") = default("dupthresh"); // SACK loss detection: classic DupThresh (RFC 3517) or RACK time-based (RFC 8985). RACK requires sackSupport.
        bool prrEnabled = default(false); // Proportional Rate Reduction (RFC 6937) during fast recovery instead of classic cwnd inflation/halving. Requires sackSupport. Currently applied by TcpReno.
        bool lossUndoEnabled = default(false); // Undo a spurious cwnd/ssthresh reduction when D-SACK (RFC 2883) or timestamps (RFC 3522 Eifel) reveal the retransmission was unnecessary. Currently applied by TcpReno.
        bool adaptiveReorderingEnabled = default(false); // Grow the reordering degree (dynamic DupThresh) when SACK reveals reordering, so persistent reordering stops causing spurious fast retransmits. Requires sackSupport. Currently applied by TcpReno.
        int maxReordering = default(300); // Upper bound on the learned reordering degree in segments (Linux tcp_max_reordering).
        int initialSsthresh = default(0xFFFFFFFF); // Initial value for Slow Start threshold used in TahoeRenoFamily. The initial value of ssthresh SHOULD be set arbitrarily high (e.g., to the size of the largest possible advertised window) Without user interaction there is no limit...
        double stopOperationExtraTime @unit(s) = default(0s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        bool ecnWillingness = default(false); // True if willing to use ECN
        double dctcpGamma = default(0.0625); // A fixed estimation gain for calculating dctcp_alpha (RFC 8257 4.2)
        bool pmtudEnabled = default(false); // Path MTU Discovery (RFC 1191, RFC 1981) enabled/disabled
        double pmtudTimeout @unit(s) = default(600s); // Time after which the original MSS is restored to probe for increased path MTU (RFC 1191, Section 6.3)
        bool keepAliveEnabled = default(false); // TCP keepalive (RFC 1122 4.2.3.6). Off by default, like SO_KEEPALIVE on Linux
        double keepAliveIdleTime @unit(s) = default(7200s); // idle time before the first keepalive probe (Linux tcp_keepalive_time)
        double keepAliveInterval @unit(s) = default(75s); // interval between keepalive probes (Linux tcp_keepalive_intvl)
        int keepAliveProbeCount = default(9); // number of unanswered probes before the connection is aborted (Linux tcp_keepalive_probes)
        double initialRto @unit(s) = default(3s); // initial retransmission timeout, used until the first RTT sample (RFC 6298 recommends 1s; Linux TCP_TIMEOUT_INIT=1s)
        double minRto @unit(s) = default(1s); // lower bound on the retransmission timeout (Linux TCP_RTO_MIN=200ms)
        double maxRto @unit(s) = default(240s); // upper bound on the retransmission timeout (Linux rto_max default 120s)
        double cubicBeta = default(0.700195); // TcpCubic multiplicative-decrease factor (717/1024)
        double cubicC = default(0.4); // TcpCubic scaling constant C
        bool cubicFastConvergence = default(true); // TcpCubic fast convergence
        bool cubicTcpFriendliness = default(true); // TcpCubic Reno-emulation (TCP friendliness)
        bool hystartEnabled = default(true); // TcpCubic HyStart hybrid slow start
        int hystartDetect = default(3); // HyStart detectors: 1=ACK-train, 2=delay, 3=both
        int hystartLowWindow = default(16); // minimum cwnd (in segments) for HyStart to act
        double hystartAckDelta @unit(s) = default(2ms); // HyStart ACK-train inter-arrival threshold
        @display("i=block/wheelbarrow");
        @signal[tcpConnectionAdded];
        @signal[tcpConnectionRemoved];
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetDropped](type=inet::Packet);
    gates:
        input appIn @labels(TcpCommand/down) @messageKinds(inet::TcpCommandCode);
        input ipIn @labels(TcpHeader,Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output appOut @labels(TcpCommand/up) @messageKinds(inet::TcpStatusInd);
        output ipOut @labels(TcpHeader,Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}

