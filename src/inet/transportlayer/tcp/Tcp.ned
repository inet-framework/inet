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
//   -# The defaults follow the RFCs (see Standards below). ~LinuxTcp and
//      ~LegacyTcp extend this module and change only defaults: ~LinuxTcp sets
//      the defaults of Linux, and ~LegacyTcp the defaults that this module had
//      before it took those of the RFCs. Select one with `**.tcp.typename`.
//
//   -# The parameters come in groups, one for each part of the protocol: the
//      module, connection setup and header options, the receive window and
//      buffer, acknowledgments, sending data, congestion control, loss
//      detection and recovery, the retransmission timeout,
//      ECN, the path MTU, keepalive, TCP Fast Open, and the connection close
//      with the checks of arriving segments. In a group, the parameters of
//      the RFCs come first; the behaviors of Linux that no RFC defines follow
//      under "Linux behaviors (no RFC)". ~LinuxTcp sets most of them.
//
//   -# The `tcpAlgorithmClass` module parameter can be used to set the TCP flavor
//      to be used (`TcpNewReno`, `TcpReno`, `TcpTahoe`, etc.). Note that this setting
//      can be programmatically overridden on a per-connection basis. Also note
//      that the `TcpNoCongestionControl` and `DumbTcp` flavors are intentionally
//      simplified code that are only provided for testing/educational purposes,
//      and do not support many TCP features the more advanced ones do. Each
//      connection creates its algorithm as its submodule `algorithm`, of the
//      NED type of that name (see ~TcpAlgorithm). The parameters of an
//      algorithm, for example those of CUBIC in ~TcpCubic, are parameters of
//      its type: set them with a key such as `**.tcp.conn-*.algorithm.beta`.
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
//      (RFC 7323). Like SACK, it is only used if both endpoints support it.
//
//   -# The `tcpEcnMode` parameter enables Explicit Congestion Notification
//      (RFC 3168). ECN is negotiated during the three-way handshake and is
//      only active if one endpoint requests it ("rfc3168") and the other one
//      answers ("rfc3168" or "passive"). The modes "accecn" and
//      "accecn-passive" add Accurate ECN (RFC 9768), which feeds back the
//      number of CE marks instead of one bit for each round trip. The AccECN
//      option (`accEcnOptionEnabled`, `accEcnOptionPolicy`) gives the bytes of
//      each ECN codepoint. `ecnMarkAll` sets ECT also on the control segments
//      of an AccECN or DCTCP connection, as Linux does. With the `DcTcp`
//      algorithm, the `g` parameter of ~DcTcp controls the EWMA gain for
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
//    RFC 9293 but may not be what you'd expect.
//  - handling segments with SYN+FIN bits set (esp. with data too) is
//    inconsistent across TCPs, so check this one if it's of importance
//
// <b>Standards</b>
//
// The documents below are grouped by area, and ordered within each area by how
// central they are to a working connection. Unless an entry lists deviations, the
// feature follows its RFC; a listed deviation is deliberate, not a known defect.
//
// The defaults of ~Tcp follow the RFCs: a value that an RFC sets, a mechanism that
// an RFC recommends (SHOULD, RECOMMENDED), and no mechanism that an RFC only allows
// (MAY) or that only Linux has. Where Linux departs from an RFC, a parameter selects
// the behavior of Linux; such entries say "Linux, by parameter". ~LinuxTcp sets all
// of them, and the other defaults of Linux; ~LegacyTcp keeps the defaults that ~Tcp
// had before it took those of the RFCs.
//
// <b>1. Core protocol</b>
//
//  - RFC 9293 - Transmission Control Protocol (TCP)
//    The base specification: every state and transition, connection setup and
//    teardown, segment acceptability, sequence-number arithmetic, the
//    SYN-REXMIT, 2MSL and FIN-WAIT-2 timers, keepalive (`keepAliveEnabled`, off by
//    default, as MUST-25 asks), the delayed ACK (`delayedAckTimeout`,
//    `delayedAckFrameCount`), the Nagle algorithm, silly-window-syndrome avoidance
//    on both sides, and the socket command set. A SEND has no PUSH flag, so PSH is
//    set on the last buffered segment (MUST-61, `pushLastBufferedSegment`).
//    Urgent data (section 3.8.5): `TcpSocket::sendUrgent()` sets the urgent pointer
//    at the octet after the data (MUST-62); a receiver tells the application of a
//    new or advanced pointer (`TCP_I_URGENT`, MUST-32) and delivers the data
//    before the mark as `TCP_I_URGENT_DATA` with the urgent bytes that remain
//    (`TcpUrgentInd`, MUST-33).
//    Deviations:
//     - PSH is ignored on arrival: the receiver delivers data as early as it can,
//       as if PSH were set on each segment.
//     - Checksums are declared correct rather than computed, unless `checksumMode`
//       is "computed" (MUST-2 asks for computed checksums; gap 2 of the TCP
//       evidence).
//     - Timers are exact: the 200 ms and 500 ms ticks of BSD-derived stacks are not
//       simulated.
//     - Initial sequence numbers come from the simulation clock, or from
//       `initialSendSequenceNumber`. The hashed generator of RFC 6528 is not
//       implemented.
//    Linux, by parameter: the PSH of Linux (`pushSegmentsOnWriteBoundary`), the
//    adaptive delayed ACK with its quickack mode (`adaptiveDelayedAcks`), the Nagle
//    algorithm of Minshall (`minshallNagle`), the zero-window probe of Linux
//    (`zeroWindowProbe`), the reset of Linux at an abort (`abortRstWithAck`), the
//    close() of Linux (`abortOnDataAfterClose`), the checks of a received segment
//    of Linux (`dropBeyondWindowEdge`, `acceptBeyondBufferAtEmptyQueue`), the
//    receive window of Linux (`windowAutoTuning`, `windowUpdateAfterRead`), and the
//    out-of-band byte of urgent data (`urgentOutOfBand`, `TcpSocket::setOobInline()`).
//    The MSS that this host announces is that of the address family for an MTU of
//    1500 bytes, 1460 for IPv4 and 1440 for IPv6 (`mss = -1`); 536 and 1220 are
//    what a missing MSS option of the peer implies (MUST-15).
//
//  - RFC 6298 - Computing TCP's Retransmission Timer
//    Jacobson's and Karn's algorithms, the first measurement of section 2.2,
//    exponential backoff, an initial RTO of 1 s (`initialRto`), a minimum of 1 s
//    (`minRexmitTimeout`), and 3 s again after a SYN timeout (section 5.7).
//    Linux, by parameter: the RTO of Linux, in which the minimum bounds the
//    variance term (`rtoVarianceFloor`, with `minRexmitTimeout` = 200 ms), and an
//    estimator that starts from the round trip of the handshake
//    (`seedRttFromHandshake`). The maximum RTO is 240 s (`maxRexmitTimeout`);
//    section 2.5 asks for at least 60 s, and Linux uses 120 s.
//
//  - RFC 5961 - Improving TCP's Robustness to Blind In-Window Attacks
//    Section 5.2 only, by parameter (`rfc5961AckCheck`, as RFC 9293 MAY-12 allows
//    and Linux does): an ACK below SND.UNA - MAX.SND.WND is discarded and answered
//    with an ACK. The blind RST (section 3) and blind SYN (section 4) mitigations,
//    and the rate limit of the challenge ACKs, are not implemented.
//
//  - RFC 5461 - TCP's Reaction to Soft Errors
//    A soft ICMP error never aborts a connection (RFC 9293 MUST-56); a hard error
//    aborts a connection in setup, also in SYN_RCVD. Linux, by parameter
//    (`softIcmpErrorsAbortSetup`): an IPv4 net or host unreachable aborts a
//    connection in setup too, the widespread behavior of section 4.
//
//  - RFC 5927 - ICMP Attacks against TCP
//    Linux, by parameter (`icmpSequenceCheck`): an ICMP error whose quoted segment
//    has a sequence number outside SND.UNA to SND.NXT is ignored (section 4.1).
//    By default each ICMP error of a connection acts, as RFC 9293 section 3.9.2.2
//    says.
//
//  - RFC 2525 - Known TCP Implementation Problems
//    Section 2.5, "failure to retain above-sequence data": out-of-order segments
//    are buffered rather than discarded.
//
//  - RFC 793 - Transmission Control Protocol (obsoleted by RFC 9293)
//    The comments cite RFC 9293 where the two texts agree.
//
// <b>2. Header options and high-performance extensions</b>
//
//  - RFC 7323 - TCP Extensions for High Performance (obsoletes RFC 1323)
//    Window Scale and Timestamps, negotiated on the SYN exchange and on by default
//    (`windowScalingSupport`, `timestampSupport`), and PAWS with the idle limit of
//    24 days. The timestamps give a round-trip sample on each ACK.
//    Deviations: the timestamp clock is the simulation clock in milliseconds;
//    `windowScalingFactor` may pin the shift count instead of deriving it from
//    `advertisedWindow`.
//    Linux, by parameter: TS.Recent is taken only after the ACK check of the
//    segment (`tsRecentAfterAckCheck`).
//
//  - RFC 2018 - TCP Selective Acknowledgment Options
//    SACK-Permitted and SACK, on by default (`sackSupport`) and used when both ends
//    offer them. After a retransmission timeout the SACK marks are cleared
//    (section 8). Linux, by parameter: the marks stay (`retainSackAfterRto`), as
//    RFC 6675 section 5.1 allows.
//
//  - RFC 2883 - An Extension to the Selective Acknowledgement (SACK) Option for
//    TCP (D-SACK)
//    Reported on duplicate arrivals (`dsackEnabled`), and read by the undo of a
//    spurious retransmission below.
//
//    Options recognized on the wire: EOL, NOP, MSS, Window Scale, SACK-Permitted,
//    SACK, Timestamps, the AccECN options (kinds 172 and 174), and the Fast Open
//    option in its assigned (kind 34) and experimental (kind 254) forms.
//
// <b>3. Congestion control</b>
//
//  - RFC 5681 - TCP Congestion Control (obsoletes RFC 2581 and RFC 2001)
//    Slow start, congestion avoidance, fast retransmit and fast recovery, under
//    each algorithm class; the initial window counts the SMSS without the TCP
//    options. `TcpTahoe` implements fast retransmit only, `TcpReno` and
//    `TcpNewReno` add fast recovery. The classic classes grow the window by at
//    most one SMSS for each ACK.
//    Deviation: the congestion window is kept in bytes rather than in segments,
//    so a rule that counts segments divides the window by the effective MSS
//    first.
//
//  - RFC 3390 - Increasing TCP's Initial Window
//    The default initial window (`initialWindow = "rfc3390"`), the upper bound of
//    RFC 5681 section 3.1. `initialWindow = "rfc2001"` gives one segment; the
//    deprecated `increasedIWEnabled` selects "rfc3390".
//
//  - RFC 6928 - Increasing TCP's Initial Window (experimental)
//    IW10, which Linux uses (`initialWindow = "rfc6928"`). A route value can
//    replace the initial window (`initialCwnd`), as `initcwnd` does on Linux.
//
//  - RFC 6582 - The NewReno Modification to TCP's Fast Recovery Algorithm
//    The default algorithm (`tcpAlgorithmClass = "TcpNewReno"`): RFC 5681 section
//    4.3 recommends a recovery that copes with more than one loss in a window.
//    `TcpNewReno` and `TcpCubic` recover by RFC 6582 when SACK was not negotiated;
//    `TcpReno` recovers by RFC 5681 then.
//
//  - RFC 9438 - CUBIC for Fast and Long-Distance Networks (obsoletes RFC 8312)
//    `TcpCubic`, the algorithm of Linux. It grows by the number of segments that an
//    ACK acknowledges in slow start (RFC 3465, experimental). The hybrid slow start
//    of Linux (`hystartEnabled` of ~TcpCubic) is off by default: RFC 9438 section 4.10 asks for
//    HyStart++ (RFC 9406), which is not implemented, and calls the hybrid slow
//    start experimental.
//    Deviation: the TCP-friendliness estimator keeps the fixed scale of Linux,
//    from beta = 717/1024, so a change of `beta` of ~TcpCubic does not rescale it.
//
//  - RFC 8257 - Data Center TCP (DCTCP): TCP Congestion Control for Data Centers
//    ECN-driven congestion control, in the `DcTcp` algorithm class.
//
//  - RFC 3042 - Enhancing TCP's Loss Recovery Using Limited Transmit
//    On by default (`limitedTransmitEnabled`), as RFC 5681 recommends; in every
//    algorithm class except `DumbTcp`.
//
//    Also available, and not specified by an RFC: `TcpVegas` and `TcpWestwood`,
//    from the research literature, and `TcpNoCongestionControl` and `DumbTcp`,
//    which are simplified on purpose, for teaching and for tests.
//
// <b>4. Loss detection and recovery</b>
//
//  - RFC 8985 - The RACK-TLP Loss Detection Algorithm for TCP
//    The default (`lossDetectionMode = "rack"`, `tlpEnabled`): time-based loss
//    detection and tail loss probes. It needs SACK.
//    Linux, by parameter: the ticks of the reordering timer of a kernel
//    (`rackReoTimerGranularity`).
//
//  - RFC 6675 - A Conservative Loss Recovery Algorithm Based on Selective
//    Acknowledgment (SACK) for TCP (obsoletes RFC 3517)
//    The SACK recovery of `TcpCubic`, `TcpReno` and `TcpNewReno` when SACK was
//    negotiated (`TcpTahoe`, `TcpVegas`, `TcpWestwood`, `TcpNoCongestionControl`
//    and `DumbTcp` have none), and the loss detection of
//    `lossDetectionMode = "dupthresh"`, with a DupThresh of three (`dupthresh`).
//
//  - RFC 6937 - Proportional Rate Reduction for TCP
//    On by default (`prrEnabled`), as RFC 8985 recommends: the window comes down
//    over the recovery instead of at once. It needs SACK.
//
//  - RFC 5682 - Forward RTO-Recovery (F-RTO): An Algorithm for Detecting
//    Spurious Retransmission Timeouts with TCP
//    The SACK variant, by parameter (`frtoEnabled`), as RFC 9438 section 4.9.1
//    allows and Linux does (`tcp_frto = 2`).
//
//  - RFC 3522 - The Eifel Detection Algorithm for TCP (experimental)
//    By parameter (`lossUndoEnabled`), as Linux does: the timestamps and D-SACK
//    detect a spurious retransmission, and the window and threshold come back
//    (RFC 4015).
//
//  - RFC 4653 - Improving the Robustness of TCP to Non-Congestion Events
//    (experimental)
//    The adaptive reordering degree of Linux, by parameter
//    (`adaptiveReorderingEnabled`, `maxReordering`), not the full NCR algorithm:
//    the reordering degree grows when SACK shows reordering.
//
// <b>5. Explicit Congestion Notification</b>
//
//  - RFC 3168 - The Addition of Explicit Congestion Notification (ECN) to IP
//    ECN negotiation on the handshake, ECE and CWR, and CE processing. By default
//    a host answers an ECN request and requests nothing (`tcpEcnMode =
//    "passive"`, as Linux `tcp_ecn = 2`), as RFC 9293 SHLD-8 asks for an
//    implementation; "rfc3168" also requests ECN.
//
//  - RFC 9768 - More Accurate ECN Feedback in TCP (AccECN)
//    The AccECN handshake, the ACE counter, and the AccECN option
//    (`tcpEcnMode = "accecn"` or "accecn-passive", `accEcnOptionEnabled`, which
//    section 3.2.3 recommends). Linux, by parameter: the option as Linux sends it
//    (`accEcnOptionPolicy = "linux"`, `accEcnBareTimeWaitAck`), and ECT on each
//    segment of an AccECN or DCTCP connection (`ecnMarkAll`).
//
//  - RFC 8311 - Relaxing Restrictions on Explicit Congestion Notification (ECN)
//    Experimentation
//    What lets AccECN use the reserved bit.
//
//  - RFC 3540 - Robust Explicit Congestion Notification (ECN) Signaling with
//    Nonces (historic, obsoleted by RFC 8311)
//    Not implemented: the bit of the nonce sum is the AE bit of AccECN.
//
// <b>6. Path MTU discovery</b>
//
//  - RFC 1191 - Path MTU Discovery
//  - RFC 1981 - Path MTU Discovery for IP version 6
//    On by default (`pmtudEnabled`), as RFC 9293 section 3.7.2 strongly
//    recommends: TCP sets DF in each IPv4 segment, and an ICMPv4 Fragmentation
//    Needed or an ICMPv6 Packet Too Big lowers the MSS and sends the segment
//    again. In SYN_SENT the error lowers the MSS of the next SYN. After
//    `pmtudTimeout` the original MSS comes back, to probe for a larger path MTU.
//
//  - RFC 4821 - Packetization Layer Path MTU Discovery
//    By parameter (`mtuProbing`, off by default as in Linux): a search between
//    `baseMss` and the MSS of the peer, with one probe segment from queued data,
//    under the preconditions of Linux `tcp_mtu_probe()`. The route MTU bounds the
//    search (`pathMtu`, `TcpSocket::setPathMtu()`). A lost probe, the first
//    unacknowledged segment where a loss recovery would start, gets no congestion
//    response (section 7.6.2): the search continues below it, and the probe goes
//    again as segments of the MSS, as Linux `tcp_simple_retransmit()`. Any other
//    retransmission ends a probe without a result, and F-RTO does not start at a
//    timeout with a probe in flight, as Linux. A search that converged starts
//    again after `mtuProbeInterval` (section 7.2), as Linux
//    `tcp_mtu_check_reprobe()`.
//    A black hole (section 7.7), retransmissions at a timeout that last longer
//    than R1, starts the search (`mtuProbing = 1`) or halves its lower bound, and
//    the MSS falls to the lower bound, as Linux `tcp_mtu_probing()`.
//    Deviations:
//     - R1 is the time of 3 retransmissions at the minimum RTO
//       (`minRexmitTimeout`), as Linux `tcp_retries1` and `tcp_model_timeout()`;
//       RFC 1122 counts 3 retransmissions at the current RTO. At the first black
//       hole of a running search, the lower bound halves at once, as Linux;
//       section 7.7 first sets the MTU to the lower bound.
//     - ~TcpVegas and ~TcpWestwood, with their own recoveries, treat a lost probe
//       as congestion.
//
// <b>7. Connection setup acceleration</b>
//
//  - RFC 7413 - TCP Fast Open (experimental)
//    Client and server, by parameter (`fastopenClientEnabled`,
//    `fastopenServerEnabled`): the cookie request and validation, data on the SYN,
//    the fallback paths, and the experimental option of Appendix A
//    (`fastopenExpOptionEnabled`). RFC 7413 leaves the cookie to the server: with
//    `fastopenKey` set, the cookie is that of Linux (SipHash-2-4 over the address
//    pair); with it empty, the default, an internal hash makes it.
//
// <b>8. Recognized on the wire but not implemented</b>
//
// The option kinds of these documents are listed, commented out, in the header
// definitions (`TcpHeader.msg`), as a record of the numbers that are taken. Nothing
// generates them, and an arriving option of one of these kinds is decoded as an
// unknown option.
//
//  - RFC 2385 - Protection of BGP Sessions via the TCP MD5 Signature Option
//    (obsoleted by RFC 5925)
//  - RFC 5925 - The TCP Authentication Option
//  - RFC 5482 - TCP User Timeout Option
//  - RFC 4782 - Quick-Start for TCP and IP
//  - RFC 4727 - Experimental Values in IPv4, IPv6, ICMPv4, ICMPv6, UDP, and TCP
//    Headers, and RFC 3692 - Assigning Experimental and Testing Numbers
//    Considered Useful. These reserve the option kinds 253 and 254 for
//    experiments; kind 254 carries the experimental Fast Open option of section 7.
//  - RFC 1072 - TCP Extensions for Long-Delay Paths (Echo and Echo Reply)
//  - RFC 1146 - TCP Alternate Checksum Options
//  - RFC 1644 - T/TCP -- TCP Extensions for Transactions Functional Specification
//    (the CC, CC.NEW and CC.ECHO options)
//  - RFC 1693 - An Extension to TCP: Partial Order Service
//  - RFC 6247 - Moving the Undeployed TCP Extensions RFC 1072, RFC 1106, RFC 1110,
//    RFC 1145, RFC 1146, RFC 1379, RFC 1644, and RFC 1693 to Historic Status
//  - RFC 7805 - Moving Outdated TCP Extensions and TCP-Related Documents to
//    Historic or Informational Status
//
// <b>Remaining gaps</b>
//
//  - Flow control between TCP and the application is opt-in: a connection opened
//    in "autoread" mode delivers the received data to the application at once, so
//    its advertised window never closes. A connection opened in explicit-read mode
//    (`autoRead = false`, then READ commands) has a receive buffer that fills up.
//    The `tcpRcvQueueDrops` statistic counts a segment dropped because the buffer
//    was full.
//  - The receive buffer is counted at the granularity of the skb truesize of
//    Linux, but it bounds the advertised window only with `windowShrinkAllowed`,
//    or when an application locks its size (`TcpSocket::setReceiveBufferSize()`,
//    as SO_RCVBUF). Otherwise the window follows the unread bytes.
//  - HyStart++ (RFC 9406) is not implemented.
//
simple Tcp extends SimpleModule like ITcp
{
    parameters:
        @class(Tcp);

        // the module
        string checksumMode @enum("declared", "computed") = default("declared");
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        double stopOperationExtraTime @unit(s) = default(0s);    // Extra time after lifecycle stop operation finished

        // connection setup and header options
        volatile int initialSendSequenceNumber = default(-1); // the initial send sequence number (ISS) of each connection; volatile, so a distribution gives each connection its own value; -1: from the clock, one tick in 4 microseconds (RFC 9293 section 3.4.1)
        int mss = default(-1); // Maximum Segment Size (header option): the largest segment text that this host sends and announces. -1: the MSS of the address family for an MTU of 1500 bytes, 1460 for IPv4 and 1440 for IPv6, when the peer is known
        bool sendMssOption = default(true); // send the MSS option in the SYN and the SYN-ACK; false: leave it out, as some peers do (the other side then assumes the default send MSS of RFC 9293 MUST-15)
        bool alignOptions = default(true); // align each TCP option with NOP options before it, as before; false: no NOPs, and End of Option List bytes pad the option area (for peers that compare the exact option layout)
        bool windowScalingSupport = default(true); // Window Scale (RFC 7323) support (header option) (WS will be enabled for a connection if both endpoints support it)
        int windowScalingFactor = default(-1); // Window Scaling Factor given as a shift count. Valid values are 0..14, and -1 for automatic selection: the smallest shift count that represents the largest window that the connection can offer, advertisedWindow, or with windowAutoTuning and a receive buffer that SO_RCVBUF does not lock, receiveBufferMax (Linux tcp_select_initial_window() represents tcp_rmem[2])
        bool timestampSupport @mutable = default(true); // Timestamps (RFC 7323) support (header option) (TS will be enabled for a connection if both endpoints support it). Each connection reads it at its open, so a run can change it as the host-wide sysctl net.ipv4.tcp_timestamps
        bool sackSupport = default(true); // Selective Acknowledgment (RFC 2018, 2883, 3517) support (header option) (SACK will be enabled for a connection if both endpoints support it)
        int synRetries = default(12); // the retransmissions of a SYN or of a SYN-ACK, after which the open fails (Linux net.ipv4.tcp_syn_retries, its default: 6; Linux has tcp_synack_retries for the SYN-ACK); they alone limit the setup: with the defaults, an open fails after about 24 minutes, and RFC 9293 section 3.8.3 asks for at least 3 minutes (MUST-23)
        bool sendDataWithFirstAck = default(true); // an active open sends data that waits at the end of the handshake with the ACK of the SYN-ACK; false: a bare ACK first, and then the data
        // Linux behaviors (no RFC)
        int synLinearTimeouts = default(0); // the first retransmissions of a SYN that keep the initial timeout before it doubles, as Linux net.ipv4.tcp_syn_linear_timeouts (its default: 4): with 4 and an initial RTO of 1 s, the SYN timeouts are 1, 1, 1, 1, 1, 2, 4 s; 0: the timeout doubles at each expiry (RFC 6298 section 5.5)
        bool syncookiesAlways = default(false); // model Linux tcp_syncookies=2 (a SYN cookie on every passive open): the cookie quantizes the MSS of the peer down to the IPv4 msstab {536, 1300, 1440, 1460} of net/ipv4/syncookies.c; the other options stay exact
        bool dropSynData = default(false); // the data of a SYN that Fast Open does not accept is dropped, so the SYN-ACK acknowledges only the SYN and the client sends the data again, as Linux (tcp_conn_request() keeps no data of such a SYN); false: the data waits in the receive queue until the handshake ends (RFC 9293 section 3.10.7.2)

        // receive window and buffer
        int advertisedWindow = default(65535); // In bytes, corresponds with the maximal receiver buffer capacity (Note: normally, NIC queues should be at least this size)
        double receiveBufferSize @unit(B) = default(-1B); // the receive buffer, apart from the advertised window (Linux sk_rcvbuf): the unread data and the out-of-order data use it, and the offered window is the smaller of its free space and the advertised window less the unread data; -1B: the buffer is the advertised window, as before
        bool windowShrinkAllowed = default(false); // the offered window follows the free receive buffer and may move its right edge back, as Linux with tcp_shrink_window=1: the buffer is charged with the truesize of the skb of each segment, and the free buffer counts at the measured ratio of payload to truesize; the first offer is half the buffer. RFC 9293 says that a receiver SHOULD NOT shrink the window (SHLD-14)
        // Linux behaviors (no RFC)
        bool windowAutoTuning = default(false); // receive window auto-tuning, as Linux (tcp_grow_window()): the offered window starts at advertisedWindow and grows with each received data segment toward a clamp of half the receive buffer; needs receiveBufferSize
        double receiveBufferLowRtt @unit(s) = default(1ms); // windowAutoTuning: below this receive RTT, Dynamic Right-Sizing grows the receive buffer in proportion to the RTT; above it, the buffer may double each RTT, as for slow start (Linux net.ipv4.tcp_rcvbuf_low_rtt)
        double receiveBufferMax @unit(B) = default(32MiB); // windowAutoTuning: the largest receive buffer that auto-tuning reaches, by Dynamic Right-Sizing, out-of-order data or the overflow of its memory (Linux net.ipv4.tcp_rmem[2], 32 MiB on a usual host)
        bool windowUpdateAfterRead = default(false); // true: as Linux (__tcp_cleanup_rbuf()), a READ after which the receiver can offer at least twice the window that is still open sends that window at once in an ACK; false: the new window goes with the next ACK, as the receiver SWS avoidance and the delayed ACK of RFC 9293 (section 3.8.6.2.2) decide
        bool acceptBeyondBufferAtEmptyQueue = default(false); // accept a whole in-order segment that reaches beyond the receive buffer when the receive queue is empty, as Linux does (tcp_data_queue()); the window is then zero until the application reads. false: drop such a segment
        bool dropBeyondWindowEdge = default(false); // true: as Linux (tcp_sequence()), a segment whose data ends beyond the highest window edge offered is dropped whole, unless it starts within that edge and the in-order receive queue is empty; false: the window test of RFC 9293 section 3.10.7.4, after which only the receive buffer limits the data
        bool urgentOutOfBand = default(false); // true: as Linux without SO_OOBINLINE, the last urgent byte leaves the stream and comes to the application alone, as a packet of one byte with TCP_I_URGENT_DATA and TcpOutOfBandInd (MSG_OOB); the data before it comes as TCP_I_DATA, and a delivery ends before it; TcpSocket::setOobInline() keeps the byte in the stream for one connection. false: the urgent receive of RFC 9293, in which the data before the mark comes as TCP_I_URGENT_DATA

        // acknowledgments
        bool delayedAcksEnabled = default(true); // Delayed ACK algorithm (RFC 1122) enabled/disabled
        double delayedAckTimeout @unit(s) = default(200ms); // with delayedAcksEnabled, the longest delay of an ACK (RFC 9293 MUST-40: less than 0.5 s)
        int delayedAckFrameCount = default(2); // with delayedAcksEnabled, the ACK goes at once when this number of full-sized segments waits for it (RFC 9293 SHLD-19: an ACK for at least each second full-sized segment)
        bool dsackEnabled = default(this.sackSupport); // report duplicate segments by D-SACK blocks (RFC 2883), when SACK is in use
        // Linux behaviors (no RFC)
        bool adaptiveDelayedAcks = default(false); // with delayedAcksEnabled, the delayed ACK of Linux: a quickack budget (ACKs at once at the start of a connection, after out-of-order data and after a long pause), an adaptive delay (ATO: from 40 ms, after the gaps between arrivals, at most the SRTT and 200 ms) in place of delayedAckTimeout, and a pingpong mode, which delays the ACKs of interactive traffic; false: the delayed ACK of RFC 9293 section 3.8.6.3 (delayedAckTimeout, delayedAckFrameCount)

        // sending data
        bool nagleEnabled = default(true); // Nagle's algorithm (RFC 896) enabled/disabled
        bool pushLastBufferedSegment = default(true); // set PSH on the last buffered segment, after which no queued data waits (RFC 9293 section 3.9.1.2, MUST-61), because the SEND of INET has no PUSH flag; pushSegmentsOnWriteBoundary replaces this rule with that of Linux
        string zeroWindowProbe @enum("rfc","linux") = default("rfc"); // the probe of a zero window. "rfc": the first unacknowledged octet, as RFC 9293 section 3.8.6.1 says ("at least one octet of new data (if available), or retransmit"); "linux": a segment without data with SEG.SEQ = SND.UNA - 1, which the receiver answers with an ACK, as Linux (tcp_xmit_probe_skb())
        double maxPersistTimeout @unit(s) = default(60s); // the upper limit of the interval of the zero-window probes, which starts at the RTO and doubles at each probe (RFC 9293 SHLD-29, SHLD-30)
        // Linux behaviors (no RFC)
        bool minshallNagle = default(false); // with nagleEnabled, the variant of Minshall, as Linux (tcp_minshall_check()): a small segment waits only while an earlier small segment is not acknowledged, so a flight of full-sized segments does not hold the end of a write; false: the Nagle algorithm of RFC 9293 section 3.7.4, where a small segment waits while any data is not acknowledged
        bool pushSegmentsOnWriteBoundary = default(false); // set PSH as Linux: on the segment that ends a write without MSG_MORE (tcp_mark_push()), on the last segment of a GSO burst of new data (tcp_transmit_skb()), in a write longer than half the largest window (forced_push()), and on the segment that ends the data of a Fast Open SYN; false: PSH only on a partial segment that TCP_CORK or MSG_MORE held
        int notsentLowat @unit(B) = default(-1B); // TCP_NOTSENT_LOWAT (a Linux socket option, no RFC): a low-water mark on the unsent bytes: when they exceed it, TCP sends no TCP_I_SEND_MSG until they are at or below it again, and then it sends one; TcpSocket::setNotsentLowat() sets it for one connection; -1B: off

        // congestion control
        string tcpAlgorithmClass @examples("TcpCubic", "TcpVegas", "TcpWestwood", "DcTcp", "TcpNewReno", "TcpReno", "TcpTahoe", "TcpNoCongestionControl") = default("TcpNewReno"); // the NED type of the algorithm of each connection, its submodule algorithm: a type of the package inet.transportlayer.tcp.flavours, or a qualified NED type name; the open command of a socket can choose another one
        string initialWindow @enum("rfc2001","rfc3390","rfc6928") = default("rfc3390"); // the initial congestion window: "rfc2001" = 1 SMSS; "rfc3390" = min(4*SMSS, max(2*SMSS, 4380 bytes)), the upper bound of RFC 5681 section 3.1; "rfc6928" = min(10*SMSS, max(2*SMSS, 14600 bytes)), as Linux
        int initialCwnd = default(0); // the initial congestion window in segments, in place of initialWindow, as the initcwnd of a Linux route (dst_metric(RTAX_INITCWND)); 0: initialWindow
        bool increasedIWEnabled = default(false); // deprecated: the same as initialWindow = "rfc3390" (RFC 3390); an error with initialWindow = "rfc6928"
        int initialSsthresh = default(0xFFFFFFFF); // Initial value for Slow Start threshold used in TahoeRenoFamily. The initial value of ssthresh SHOULD be set arbitrarily high (e.g., to the size of the largest possible advertised window) Without user interaction there is no limit...
        bool limitedTransmitEnabled = default(true); // Limited Transmit algorithm (RFC 3042) enabled/disabled (can be used for TcpReno/TcpTahoe/TcpNewReno/TcpNoCongestionControl)

        // loss detection and recovery
        string lossDetectionMode @enum("dupthresh","rack") = default("rack"); // SACK loss detection: classic DupThresh (RFC 6675) or RACK time-based (RFC 8985). RACK requires sackSupport.
        int dupthresh = default(3); // Used for TcpTahoe, TcpReno, and SACK (RFC 3517) DO NOT change unless you really know what you are doing
        bool tlpEnabled = default(true); // Tail Loss Probe (RFC 8985 section 7): about two SRTTs after the last transmission or ACK of new data, send one probe, so that its ACK starts the repair of a lost tail before the RTO. Requires sackSupport.
        bool prrEnabled = default(true); // Proportional Rate Reduction (RFC 6937) during fast recovery instead of classic cwnd inflation/halving. Requires sackSupport.
        bool frtoEnabled = default(false); // F-RTO spurious-RTO detection (RFC 5682, SACK-enhanced; Linux tcp_frto=2). Requires sackSupport.
        bool lossUndoEnabled = default(false); // Undo a spurious cwnd/ssthresh reduction when D-SACK (RFC 2883) or timestamps (RFC 3522 Eifel) reveal the retransmission was unnecessary.
        // Linux behaviors (no RFC)
        double rackReoTimerGranularity @unit(s) = default(0s); // model the kernel's jiffy-quantized REO timeout arming (Linux usecs_to_jiffies(timeout)+1: delay rounded UP to this granularity plus one extra tick, e.g. 4ms for an HZ=250 kernel); a sub-tick RACK deadline then fires whole ticks later, letting a dupthresh-timed SACK enter recovery first, as real kernels do. 0 = exact (default INET behavior)
        bool adaptiveReorderingEnabled = default(false); // Grow the reordering degree (dynamic DupThresh) when SACK reveals reordering, so persistent reordering stops causing spurious fast retransmits. Requires sackSupport.
        int maxReordering = default(300); // Upper bound on the learned reordering degree in segments (Linux tcp_max_reordering).
        bool retainSackAfterRto = default(false); // false: a retransmission timeout clears the SACKed marks of the scoreboard (RFC 2018 section 8, RFC 5682 step 1); true: the marks survive the timeout, as Linux keeps them (RFC 6675 section 5.1 allows it)

        // retransmission timeout
        double initialRto @unit(s) = default(1s); // the retransmission timeout before the first RTT measurement, also the first timeout of a SYN and of a SYN-ACK (RFC 6298 section 2.1: 1 s; 3 s is the value of RFC 1122)
        double minRexmitTimeout @unit(s) = default(1s); // the lower limit of the retransmission timeout (RFC 6298 section 2.4: 1 s; Linux: 200 ms)
        double maxRexmitTimeout @unit(s) = default(240s); // the upper limit of the retransmission timeout (RFC 6298 section 2.5: at least 60 s)
        int maxRexmitCount = default(12); // the retransmissions of one segment, after which the connection aborts
        // Linux behaviors (no RFC)
        bool rtoVarianceFloor = default(false); // false: RTO = SRTT + 4*RTTVAR, rounded up to minRexmitTimeout (RFC 6298 sections 2.3 and 2.4); true: RTO = SRTT + max(4*RTTVAR, minRexmitTimeout), as Linux (tcp_set_rto()), whose minimum limits the variance term
        bool seedRttFromHandshake = default(false); // Seed the RTT estimator (srtt/rttvar/RTO) from the SYN<->SYN-ACK exchange, as Linux does. The first-flight RTO becomes RTT-scaled instead of the initial RTO.

        // Explicit Congestion Notification (ECN, AccECN)
        string tcpEcnMode @mutable @enum("off","passive","rfc3168","accecn","accecn-passive") = default("passive"); // ECN: "off": no ECN; "passive": answer an ECN-setup SYN (RFC 3168), but request no ECN in an active open (Linux tcp_ecn=2, its default); "rfc3168": request and answer ECN; "accecn": request Accurate ECN (RFC 9768), and answer it and ECN; "accecn-passive": answer Accurate ECN and ECN, but request nothing. Each connection reads it at its open
        bool ecnWillingness = default(false); // deprecated: true is tcpEcnMode="rfc3168", in place of "off" or "passive"
        bool accEcnOptionEnabled @mutable = default(true); // AccECN: send the AccECN option, which gives the byte counts of each ECN codepoint (RFC 9768 section 3.2.3: RECOMMENDED); false: only the ACE field (Linux tcp_ecn_option=0). Each connection reads it at its open
        // Linux behaviors (no RFC)
        string accEcnOptionPolicy @enum("rfc","linux") = default("rfc"); // AccECN: when the option goes out and which fields it has. "rfc": the recommended scheme of RFC 9768 section 3.2.3.3: after a byte count changed, with each field that ever changed, in the order that needs fewer fields, and with each field on the SYN-ACK and on the first segments of the client (section 3.2.3.2.1); "linux": as Linux tcp_ecn_option=2: on each segment, kind 174 with each field that fits, but only after the peer sent an option (section 3.2.3.2.3 asks to send it also then)
        bool accEcnBareTimeWaitAck = default(false); // AccECN: the ACKs of TIME_WAIT carry no ACE field, as in Linux, whose time-wait socket sends them (tcp_v4_send_ack()); false: they carry it, as each ACK after the handshake (RFC 9768 section 3.2.2.2: MUST)
        bool ecnMarkAll = default(false); // ECN: ECT also on the pure ACKs, FINs, retransmissions and window probes of an AccECN connection (RFC 9768) and of a DCTCP connection, as Linux does (tcp_ecn_send()); RFC 8257 section 4.1 recommends such a knob for DCTCP. false: ECT only on new data, as RFC 3168 sections 6.1.4 to 6.1.6 ask, which RFC 9768 section 4 keeps

        // path MTU
        bool pmtudEnabled = default(true); // Path MTU Discovery (RFC 1191, RFC 1981) enabled/disabled
        double pmtudTimeout @unit(s) = default(600s); // Time after which the original MSS is restored to probe for increased path MTU (RFC 1191, Section 6.3)
        int mtuProbing = default(0); // the path MTU search of RFC 4821, as Linux tcp_mtu_probing: 2 = from the start of each connection, the sender sends single segments larger than its MSS and raises the MSS when the peer acknowledges them; 1 = the search starts after a black hole, that is, retransmissions at a timeout that last longer than R1; 0 = no search
        int baseMss = default(1024); // with mtuProbing: the lower end of the path MTU search, the payload of a segment that the path surely carries (Linux tcp_base_mss)
        int pathMtu = default(0); // with mtuProbing: the MTU of the route, the largest MTU that the search can give the connection (Linux dst_mtu()); 0 = the MTU of the negotiated MSS
        double mtuProbeInterval @unit(s) = default(600s); // with mtuProbing: when the search converged and this time passed since it started, it starts again from the MTU of the MSS in use up to the MSS of the peer, as Linux tcp_probe_interval (RFC 4821 section 7.2: at least 5 minutes, 10 recommended)

        // keepalive
        bool keepAliveEnabled = default(false); // keepalive (RFC 9293 section 3.8.4): after keepAliveIdleTime without a received segment and without data in flight, send a probe without data (SEG.SEQ = SND.UNA - 1), again each keepAliveInterval, and abort the connection after keepAliveProbeCount probes without an answer; off by default (MUST-25); TcpSocket::setKeepAlive() turns it on or off for one connection (MUST-24)
        double keepAliveIdleTime @unit(s) = default(7200s); // the idle time before the first keepalive probe (RFC 9293 MUST-28: at least two hours; Linux tcp_keepalive_time)
        double keepAliveInterval @unit(s) = default(75s); // the time between two keepalive probes (Linux tcp_keepalive_intvl)
        int keepAliveProbeCount = default(9); // the keepalive probes without an answer, after which the connection aborts (Linux tcp_keepalive_probes; RFC 9293 MUST-29: one probe without an answer is not a dead connection)

        // TCP Fast Open
        bool fastopenServerEnabled @mutable = default(false); // TCP Fast Open (RFC 7413), server: give cookies, and deliver the data of a SYN with a valid cookie to the application before the handshake completes (~ Linux tcp_fastopen bit 0x2)
        bool fastopenClientEnabled @mutable = default(false); // TCP Fast Open (RFC 7413), client: a connect() with fastOpen requests a cookie from the server, and with a cached cookie it sends its first data on the SYN (~ Linux tcp_fastopen bit 0x1)
        bool fastopenAcceptWithoutCookie @mutable = default(false); // TCP Fast Open, server: accept the data of a SYN without a valid cookie, if the SYN has data or a Fast Open option, and give no cookie (Linux tcp_fastopen bit 0x200); RFC 7413 requires a valid cookie
        bool fastopenClientNoCookieRequired @mutable = default(false); // TCP Fast Open, client: send the data on the SYN without a cookie and without a Fast Open option (Linux tcp_fastopen bit 0x4), for a server that accepts it; RFC 7413 requires a cookie
        bool fastopenLenientCookieValidation = default(false); // TCP Fast Open, server: accept each cookie of a valid length without a check; false: accept only the cookie that this server gives the address of the client (RFC 7413 section 4.2.2)
        int fastopenCookieBytes = default(8); // TCP Fast Open, server: the length of the cookies that this server gives (RFC 7413: an even number from 4 to 16)
        int fastopenCookieCacheSize = default(1024); // TCP Fast Open, client: the number of servers whose cookie is kept; a new server then replaces an arbitrary entry
        string fastopenKey @mutable = default(""); // TCP Fast Open, server: the key of the cookies, in the format of Linux net.ipv4.tcp_fastopen_key ("xxxxxxxx-xxxxxxxx-xxxxxxxx-xxxxxxxx", the primary key); a cookie of 8 bytes is then the cookie of Linux with this key; empty: a key from the RNG
        int fastopenMaxPendingRequests = default(16); // TCP Fast Open, server: the largest number of Fast Open connections of a local port in SYN_RCVD (PendingFastOpenRequests, RFC 7413 section 4.2; Linux takes it from the TCP_FASTOPEN socket option of the listener); above it, a SYN gets no Fast Open: no cookie, and its data is not acknowledged
        // Linux behaviors (no RFC)
        double fastopenBlackholeTimeout @unit(s) = default(3600s); // TCP Fast Open, client: when a SYN with a Fast Open option or data is sent the third time (a negative response, RFC 7413 section 4.1.3.1), no connection uses Fast Open for this time, which doubles at each further blackhole, up to 64 times (as Linux); 0s: no detection (the Linux default since 5.14)
        bool fastopenExpOptionEnabled @mutable = default(false); // TCP Fast Open: also use the experimental option (kind 254 with the magic number 0xF989, RFC 7413 appendix A), as Linux: the server answers it in the same option, and the client tries it once when a cookie request had no answer; false: only kind 34, which RFC 7413 section 9 says implementations SHOULD use

        // connection close and the checks of arriving segments and ICMP errors
        int msl @unit(s) = default(120s);   // Maximum Segment Lifetime
        bool abortOnDataAfterClose = default(false); // model the full close of Linux close(): a CLOSE without halfClose also shuts the receive side down, and new data after it resets the connection (RFC 1122 section 4.2.2.13); false: RFC 9293 CLOSE, after which the application may still receive
        bool rfc5961AckCheck = default(false); // the ACK check of RFC 5961 section 5.2, which RFC 9293 allows (MAY-12) and Linux uses: a segment whose ACK lies below SND.UNA - MAX.SND.WND is dropped, and an ACK goes in reply; false: such an ACK counts as an old duplicate ACK
        bool icmpSequenceCheck = default(false); // true: an ICMP error whose quoted TCP segment has a sequence number outside SND.UNA to SND.MAX is ignored as stale or forged, as Linux (tcp_v4_err(), tcp_v6_err()) and the BSDs do and RFC 5927 section 4.1 describes; false: each ICMP error of the connection counts, as RFC 9293 section 3.9.2.2 says
        bool softIcmpErrorsAbortSetup = default(false); // true: a soft ICMP Destination Unreachable (IPv4 codes 0, 1 and 5, IPv6 codes 0 and 3) aborts a connection in SYN_SENT or SYN_RCVD, as Linux (tcp_v4_err(), tcp_v6_err()) and RFC 5461 section 4 describe; false: RFC 9293 MUST-56, a soft error never aborts a connection
        // Linux behaviors (no RFC)
        bool abortRstWithAck = default(false); // true: the RST of an abort carries the ACK bit and RCV.NXT, as Linux (tcp_send_active_reset()); false: <SEQ=SND.NXT><CTL=RST> of RFC 9293
        bool tsRecentAfterAckCheck = default(false); // false: TS.Recent takes the TSval of each acceptable segment before its ACK is processed (RFC 7323 section 5.3, R3); true: not from a segment that acknowledges data that was not sent, as Linux, which updates TS.Recent only after the ACK passes its check (tcp_replace_ts_recent() in tcp_ack())

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

