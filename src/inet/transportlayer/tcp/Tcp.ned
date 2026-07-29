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
//   -# The `initialWindow` parameter selects the initial congestion window:
//      one segment (RFC 2001), min(4*MSS, max(2*MSS, 4380 bytes)) (RFC 3390), or
//      IW10 (RFC 6928), which is the default. The older `increasedIWEnabled`
//      boolean does the same as "rfc3390" and is deprecated.
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
//   -# The `tcpEcnMode` parameter selects Explicit Congestion Notification:
//      classic ECN (RFC 3168) or AccECN (RFC 9768), requested or accept-only.
//      Either way it is negotiated during the three-way handshake and is only
//      active if both endpoints agree. The older `ecnWillingness` boolean is
//      deprecated in favor of it. When using the `DcTcp` algorithm, the
//      `dctcpGamma` parameter controls the EWMA gain for estimating the
//      congestion level (RFC 8257).
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
// The documents below are grouped by area and ordered, within each area, by how
// central they are to a working connection. Unless an entry lists deviations, the
// feature is meant to follow its RFC; where deviations are listed they are
// deliberate, not known bugs.
//
// Two recurring reasons for a deviation are worth stating once. First, the module
// is continuously validated against real Linux behavior (the differential
// packetdrill oracle in the `inet-gpl` package's `tests/oracle`), so where Linux
// and an RFC disagree on something observable on the wire, the tie is broken in
// favor of Linux; such entries are marked "Linux parity". Second, several
// Linux-parity behaviors are gated behind default-off parameters so that existing
// simulation results do not shift under users unannounced; those name the
// parameter that turns them on.
//
// <b>1. Core protocol</b>
//
//  - RFC 9293 - Transmission Control Protocol (TCP)
//    The base specification: every state and transition, connection setup and
//    teardown, segment acceptability, sequence-number arithmetic, the CONN-ESTAB,
//    SYN-REXMIT, 2MSL and FIN-WAIT-2 timers, and the socket command set.
//    Deviations:
//     - The urgent pointer is carried and tracked, but urgent data is never
//       delivered out of band: `TCP_I_URGENT_DATA` is never produced, and a
//       receiver treats URG-marked bytes as ordinary data.
//     - PSH is set on outgoing segments but ignored on arrival: the receiver
//       always delivers as early as it can, as if PSH were set on everything.
//       (What drives PSH on the sending side is Linux's rule, not the RFC's --
//       see `pushSegmentsOnWriteBoundary`.)
//     - Checksums are declared correct rather than computed, unless
//       `checksumMode` is set to "computed".
//     - Timers are exact. The 200 ms / 500 ms "fast and slow tick" quantization
//       that BSD-derived stacks impose on every timeout is not simulated.
//     - Initial sequence numbers come from the simulation clock, or from
//       `initialSendSequenceNumber`. RFC 6528, "Defending against Sequence
//       Number Attacks", specifies a hashed generator; it is not implemented.
//     - The MSS this host announces defaults to an MTU-derived 1460 (IPv4) or
//       1440 (IPv6) rather than RFC 1122's conservative 536/1220. Those RFC
//       values are still what a *missing* MSS option from the peer implies.
//
//  - RFC 1122 - Requirements for Internet Hosts -- Communication Layers
//    Delayed ACKs, Nagle's algorithm, silly-window-syndrome avoidance on both
//    sides, and keepalive (`keepAliveEnabled`, off by default as on Linux).
//    Deviations:
//     - The delayed-ACK timer is a fixed 200 ms and also fires after
//       `delayedAckFrameCount` segments; RFC 1122 permits up to 500 ms.
//       `adaptiveDelayedAcks` replaces the fixed timer with Linux's adaptive ATO
//       plus its quickack budget and interactive "pingpong" detection.
//       (Linux parity, opt-in.)
//     - Nagle follows Minshall's variant -- a trailing partial segment is held
//       only while an earlier SMALL segment is still unacknowledged -- rather
//       than the plain "any unacknowledged data holds it" rule. (Linux parity.)
//     - Receiver-side SWS avoidance follows the Stevens/FreeBSD formulation and
//       ordering rather than the pseudo-code in RFC 9293.
//
//  - RFC 6298 - Computing TCP's Retransmission Timer
//    Jacobson's and Karn's algorithms, exponential backoff, `initialRto` = 1 s.
//    Deviations:
//     - `minRexmitTimeout` defaults to 200 ms, Linux's floor, where section 2.4
//       recommends 1 s (while explicitly allowing a lower value). (Linux parity.)
//     - With `seedRttFromHandshake` the estimator starts from the SYN/SYN-ACK
//       round trip rather than from the first data ACK. RFC 6298 does not
//       describe this; Linux does it. (Linux parity, on by default.)
//
//  - RFC 5961 - Improving TCP's Robustness to Blind In-Window Attacks
//    Section 5.2 only: an ACK below SND.UNA - MAX.SND.WND is discarded and
//    answered with a challenge ACK.
//    Deviations: the blind RST (section 3) and blind SYN (section 4)
//    mitigations, and the global challenge-ACK rate limit, are not implemented.
//
//  - RFC 5461 - TCP's Reaction to Soft Errors
//    A soft ICMP error never aborts an established connection; hard errors abort
//    a connection that is still being set up.
//
//  - RFC 5927 - ICMP Attacks against TCP
//    The sequence number quoted inside an ICMP error is validated against the
//    connection before the error is acted on.
//
//  - RFC 2525 - Known TCP Implementation Problems
//    Section 2.5, "failure to retain above-sequence data": out-of-order segments
//    are buffered rather than discarded.
//
//  - RFC 793 - Transmission Control Protocol (historic, obsoleted by RFC 9293)
//    Cited in the code only where the two texts differ in wording. RFC 9293
//    governs.
//
// <b>2. Header options and high-performance extensions</b>
//
//  - RFC 7323 - TCP Extensions for High Performance
//    Window Scale and Timestamps, both negotiated on the SYN exchange, plus PAWS
//    (with the specified 24-day idle threshold). Timestamps are also what drives
//    RTT measurement and the Eifel and RACK machinery below.
//    Deviations: the timestamp clock is the simulation clock; `windowScalingFactor`
//    may pin the shift count instead of deriving it from `advertisedWindow`.
//    Obsoletes RFC 1323, "TCP Extensions for High Performance", which older
//    comments and parameter descriptions still refer to.
//
//  - RFC 2018 - TCP Selective Acknowledgment Options
//    SACK-Permitted and SACK, used only when both endpoints offer them.
//
//  - RFC 2883 - An Extension to the Selective Acknowledgement (SACK) Option for
//    TCP (D-SACK)
//    Reported on duplicate arrivals and consumed by the spurious-retransmission
//    undo below (`dsackEnabled`).
//
//    Options recognized on the wire: EOL, NOP, MSS, Window Scale,
//    SACK-Permitted, SACK, Timestamps, the AccECN options of section 5 (kinds
//    172 and 174), and the Fast Open option in both its assigned (kind 34) and
//    experimental (kind 254) forms.
//
// <b>3. Congestion control</b>
//
//  - RFC 5681 - TCP Congestion Control
//    Slow start, congestion avoidance, fast retransmit and fast recovery; the
//    common substrate under every algorithm class. `TcpTahoe` implements fast
//    retransmit only, `TcpReno` and `TcpNewReno` add fast recovery.
//    Deviations:
//     - The congestion window is maintained in bytes rather than segments. This
//       is a representation choice, but it does surface: where the reference
//       counts packets (for example when deciding how large a burst may be), the
//       byte-valued window has to be divided by the effective MSS first.
//     - The default initial window is RFC 6928's IW10, not RFC 5681's 2-4
//       segments (see `initialWindow`).
//    Predecessors cited in the code: RFC 2001, "TCP Slow Start, Congestion
//    Avoidance, Fast Retransmit, and Fast Recovery Algorithms" (also the name of
//    `initialWindow = "rfc2001"`, one segment), and RFC 2581, "TCP Congestion
//    Control"; both are obsoleted by RFC 5681.
//
//  - RFC 6928 - Increasing TCP's Initial Window
//    IW10, the default (`initialWindow = "rfc6928"`). A route-supplied initial
//    window may override it (`initialCwnd`), as `initcwnd` does on Linux.
//
//  - RFC 3390 - Increasing TCP's Initial Window
//    Selectable as `initialWindow = "rfc3390"`. The older `increasedIWEnabled`
//    boolean is deprecated in favor of it.
//
//  - RFC 9438 - CUBIC for Fast and Long-Distance Networks (obsoletes RFC 8312,
//    "CUBIC for Fast Long-Distance Networks")
//    The default algorithm (`tcpAlgorithmClass = "TcpCubic"`), including the
//    HyStart slow-start exit, which RFC 9438 describes as optional.
//    Deviations: the TCP-friendliness estimator keeps the reference
//    implementation's fixed scale, derived from beta = 717/1024, so changing
//    `cubicBeta` alters the multiplicative decrease without rescaling the
//    friendliness term.
//
//  - RFC 8257 - Data Center TCP (DCTCP): TCP Congestion Control for Data Centers
//    ECN-feedback-driven congestion control, in the `DcTcp` algorithm class.
//
//  - RFC 3042 - Enhancing TCP's Loss Recovery Using Limited Transmit
//    On by default (`limitedTransmitEnabled`); available to every algorithm
//    class except `DumbTcp`.
//
//  - RFC 3465 - TCP Congestion Control with Appropriate Byte Counting
//    (experimental)
//    Applied in `TcpCubic`'s slow start, so that a delayed-ACK receiver does not
//    halve the growth rate. Deliberately NOT applied in the classic
//    `TcpTahoe`/`TcpReno`/`TcpNewReno`/`TcpWestwood` flavours, which grow by one
//    SMSS per ACK as RFC 5681 specifies.
//
//    Also available, and not RFC-specified: `TcpVegas` and `TcpWestwood` (both
//    from the research literature), `TcpNoCongestionControl`, and `DumbTcp` --
//    the last two intentionally simplified, for teaching and for isolating other
//    behavior in tests.
//
// <b>4. Loss detection and recovery</b>
//
//  - RFC 8985 - The RACK-TLP Loss Detection Algorithm for TCP
//    The default (`lossDetectionMode = "rack"`, `tlpEnabled`): time-based loss
//    detection plus tail loss probes. Requires SACK.
//
//  - RFC 6675 - A Conservative Loss Recovery Algorithm Based on Selective
//    Acknowledgment (SACK) for TCP
//    The SACK recovery engine, selected when SACK was negotiated by an algorithm
//    class that has a recovery engine at all -- `TcpCubic`, `TcpReno` and
//    `TcpNewReno`. (`TcpTahoe`, `TcpVegas`, `TcpWestwood`, `TcpNoCongestionControl`
//    and `DumbTcp` have none.) Also reachable as the loss detector, via
//    `lossDetectionMode = "dupthresh"`.
//    Deviations: DupThresh is dynamic by default rather than fixed at three --
//    see RFC 4653 below.
//
//  - RFC 6582 - The NewReno Modification to TCP's Fast Recovery Algorithm
//    The recovery engine `TcpNewReno` and `TcpCubic` fall back to when SACK was
//    not negotiated. `TcpReno` falls back to plain RFC 5681 recovery instead.
//
//  - RFC 6937 - Proportional Rate Reduction for TCP
//    The default pacing of the window down during recovery (`prrEnabled`),
//    replacing RFC 5681's cwnd inflation and halving. Requires SACK.
//
//  - RFC 5682 - Forward RTO-Recovery (F-RTO): An Algorithm for Detecting
//    Spurious Retransmission Timeouts with TCP
//    SACK-enhanced spurious-timeout detection (`frtoEnabled`, on by default,
//    matching Linux's `tcp_frto = 2`).
//
//  - RFC 3522 - The Eifel Detection Algorithm for TCP
//    Timestamp-based detection of a spurious retransmission; together with
//    D-SACK it drives the cwnd/ssthresh undo (`lossUndoEnabled`).
//
//  - RFC 4653 - Improving the Robustness of TCP to Non-Congestion Events
//    The adaptive-reordering idea, not the full NCR algorithm: the reordering
//    degree grows when SACK reveals reordering, so persistent reordering stops
//    causing spurious fast retransmits (`adaptiveReorderingEnabled`,
//    `maxReordering`).
//
//  - RFC 3517 - A Conservative Selective Acknowledgment (SACK)-based Loss
//    Recovery Algorithm for TCP (historic, obsoleted by RFC 6675)
//    Referenced in the code for the classic DupThresh formulation.
//
// <b>5. Explicit Congestion Notification</b>
//
//  - RFC 3168 - The Addition of Explicit Congestion Notification (ECN) to IP
//    ECN negotiation on the handshake, ECE/CWR handling, and CE processing
//    (`tcpEcnMode = "rfc3168"`, or "passive" to accept but not request).
//
//  - RFC 9768 - More Accurate ECN Feedback in TCP (AccECN)
//    The AccECN handshake, the three-bit ACE counter, and the AccECN option
//    (`tcpEcnMode = "accecn"` / "accecn-passive", `accEcnOptionEnabled`).
//
//  - RFC 8311 - Relaxing Restrictions on Explicit Congestion Notification (ECN)
//    Experimentation
//    What permits the reserved-bit reuse AccECN depends on.
//
//  - RFC 3540 - Robust Explicit Congestion Notification (ECN) Signaling with
//    Nonces (historic, obsoleted by RFC 8311)
//    NOT implemented: the nonce-sum bit it defines is instead the AE bit of
//    AccECN's counter.
//
// <b>6. Path MTU discovery</b>
//
//  - RFC 1191 - Path MTU Discovery
//  - RFC 1981 - Path MTU Discovery for IP version 6
//    `pmtudEnabled` sets DF on IPv4 segments; an ICMPv4 Fragmentation Needed or
//    ICMPv6 Packet Too Big reduces the MSS and retransmits the offending
//    segment. After `pmtudTimeout` the original MSS is restored, to probe for a
//    path that has grown.
//
//  - RFC 4821 - Packetization Layer Path MTU Discovery
//    `mtuProbing` arms an upward binary search between `baseMss` and the peer's
//    MSS: one oversized probe segment is built from data already queued, and the
//    lower bound moves up when that segment is acknowledged.
//    Deviations:
//     - Upward search only. There is no black-hole detector, so the "enable
//       after a failure is detected" setting (`mtuProbing = 1`) behaves as off;
//       only `mtuProbing = 2` arms the search.
//     - A lost probe is recognized from the retransmission that covers it, and
//       lowers the upper bound. Linux reaches the same verdict from its loss
//       estimator instead (`tcp_fastretrans_alert`), so the two can disagree about
//       exactly WHEN a probe is written off, though not about the outcome.
//     - There is no reprobe timer: once the search has narrowed, it stays
//       narrowed for the life of the connection.
//     - Sending a probe does not decrement the congestion window. The reference
//       decrements it because its window counts packets and the probe replaces
//       several of them with one; a byte-valued window already charges the probe
//       exactly what it carries.
//
// <b>7. Connection setup acceleration</b>
//
//  - RFC 7413 - TCP Fast Open
//    Client and server, cookie request and validation, data on the SYN, the
//    fallback paths, and the Appendix A experimental option (kind 254 with the
//    0xF989 magic) alongside the assigned kind 34.
//    Note, not a deviation: RFC 7413 leaves cookie construction to the server.
//    Set `fastopenKey` and the cookie is derived exactly as Linux derives it
//    (SipHash-2-4 over the address pair), so a simulated server and a real client
//    agree; leave it empty, the default, and an internal hash is used instead.
//
// <b>8. Recognized on the wire but not implemented</b>
//
// The option kinds these documents define are listed -- commented out -- in the
// header definitions (`TcpHeader.msg`), as a record of which kind numbers are
// taken. Nothing generates them, and an arriving option of one of these kinds is
// decoded as an unknown option rather than being recognized.
//
//  - RFC 2385 - Protection of BGP Sessions via the TCP MD5 Signature Option
//    (obsoleted by RFC 5925)
//  - RFC 5925 - The TCP Authentication Option
//  - RFC 5482 - TCP User Timeout Option
//  - RFC 4782 - Quick-Start for TCP and IP
//  - RFC 4727 - Experimental Values in IPv4, IPv6, ICMPv4, ICMPv6, UDP, and TCP
//    Headers, and RFC 3692 - Assigning Experimental and Testing Numbers
//    Considered Useful. These reserve option kinds 253 and 254 for experiments.
//    Kind 253 is unused here; kind 254 is NOT -- it carries the experimental Fast
//    Open option of section 7, and is the one live enum member in this list.
//  - RFC 1072 - TCP Extensions for Long-Delay Paths (Echo / Echo Reply)
//  - RFC 1146 - TCP Alternate Checksum Options
//  - RFC 1644 - T/TCP -- TCP Extensions for Transactions Functional Specification
//    (the CC, CC.NEW and CC.ECHO options)
//  - RFC 1693 - An Extension to TCP: Partial Order Service
//  - RFC 6247 - Moving the Undeployed TCP Extensions RFC 1072, RFC 1106, RFC 1110,
//    RFC 1145, RFC 1146, RFC 1379, RFC 1644, and RFC 1693 to Historic Status
//    (the document that retired four of the entries above: RFC 1072, 1146,
//    1644 and 1693)
//
//  - RFC 7805 - Moving Outdated TCP Extensions and TCP-Related Documents to
//    Historic or Informational Status (the companion to RFC 6247 above; between
//    them they explain why most of this list is historic rather than merely
//    unimplemented)
//
// <b>Remaining gaps</b>
//
//  - Urgent data, as described under RFC 9293 above.
//  - Flow control between TCP and the application exists but is opt-in: a
//    connection opened in "autoread" mode drains the receive queue to the
//    application immediately, so its advertised window never closes. Open it in
//    explicit-read mode (`autoRead = false` on the OPEN command, then READ
//    commands) for a receive buffer that fills up and applies back pressure.
//    A segment dropped because the buffer was exhausted is counted by the
//    `tcpRcvQueueDrops` statistic.
//  - Receive-buffer occupancy is always tracked at skb-truesize granularity, but
//    it bounds the advertised window only under `windowShrinkAllowed`, or when the
//    buffer is pinned (`receiveBufferLocked`, i.e. an application that issued
//    SO_RCVBUF). Otherwise the window follows unread bytes, because an unpinned
//    buffer grows on demand and its free space is not the binding constraint.
//
simple Tcp extends SimpleModule like ITcp
{
    parameters:
        @class(Tcp);
        string checksumMode @enum("declared", "computed") = default("declared");
        int advertisedWindow @mutable = default(65535); // In bytes, corresponds with the maximal receiver buffer capacity (Note: normally, NIC queues should be at least this size). @mutable so a harness/app can model setsockopt(SO_RCVBUF) by updating it before a connection is configured.
        bool delayedAcksEnabled = default(true); // Delayed ACK algorithm (RFC 1122) enabled/disabled
        int delayedAckFrameCount = default(2); // number of frames after delayed acks are sent
        bool nagleEnabled = default(true); // Nagle's algorithm (RFC 1122) enabled/disabled
        bool limitedTransmitEnabled = default(true); // Limited Transmit algorithm (RFC 3042) enabled/disabled (can be used for TcpReno/TcpTahoe/TcpNewReno/TcpNoCongestionControl)
        bool adaptiveDelayedAcks = default(false); // Linux-shaped receiver ACK dynamics on top of delayedAcksEnabled: a quickack budget (immediate ACKs at connection start, after out-of-order data, and after long idle), an ADAPTIVE delayed-ACK timeout (ATO: 40ms floor, tracks the inter-segment arrival gap, bounded by srtt and 200ms) instead of the fixed 200ms timer, and an interactive "pingpong" mode. Off = the classic fixed-timeout RFC 1122 behavior
        bool pushSegmentsOnWriteBoundary = default(false); // Linux parity: set the PSH flag on the last segment carrying the final buffered byte of the current send (Linux always does this; PSH has no effect on INET's own receiver, so this is a pure wire-realism option). Default off pending a maintainer-gated default flip, like the other Linux-default parity switches.
        bool increasedIWEnabled = default(false); // DEPRECATED: use initialWindow="rfc3390" instead. Increased Initial Window (RFC 3390) enabled/disabled
        string initialWindow @enum("rfc2001","rfc3390","rfc6928") = default("rfc6928"); // initial congestion window: rfc2001=1 SMSS, rfc3390=min(4*MSS,max(2*MSS,4380)), rfc6928=IW10=min(10*MSS,max(2*MSS,14600))
        bool sackSupport = default(true); // Selective Acknowledgment (RFC 2018, 2883, 6675) support (header option) (SACK will be enabled for a connection if both endpoints support it)
        bool dsackEnabled = default(this.sackSupport); // Selective Acknowledgment (RFC 2018, 2883, 6675) support (header option) (SACK will be enabled for a connection if both endpoints support it)
        bool windowScalingSupport = default(true); // Window Scale (RFC 7323) support (header option) (WS will be enabled for a connection if both endpoints support it)
        int windowScalingFactor @mutable = default(-1); // Window Scaling Factor given as a shift count. Valid values are 0..14, and -1 for automatic selection (it chooses the smallest shift count that makes advertisedWindow representable in 16 bits). @mutable so a harness/app modeling setsockopt(SO_RCVBUF) can switch to auto (-1) at runtime.
        bool timestampSupport @mutable = default(true); // Timestamps (RFC 7323) support (header option) (TS will be enabled for a connection if both endpoints support it)
        int mss = default(-1); // Maximum Segment Size (RFC 9293) (header option), i.e. the largest segment text this host is willing to receive. -1 derives it from the address family once the peer is known: 1460 for IPv4, 1440 for IPv6. RFC 1122's conservative 536/1220 is what a MISSING MSS option from the peer implies, not this default
        int mtuProbing = default(0); // Packetized Path MTU Discovery (RFC 4821), ~ Linux sysctl tcp_mtu_probing: 0 = off, 2 = search for a larger usable MTU from the start of every connection. 1 (Linux "only after a black hole is detected") behaves as 0 here, since INET has no black-hole detector
        int baseMss = default(1024); // lower end of the RFC 4821 search, ~ Linux sysctl tcp_base_mss: the MSS assumed to work without probing. Effective only with mtuProbing
        int pathMtu = default(0); // the route's MTU, the ceiling the RFC 4821 search may reach (Linux dst_mtu / icsk_pmtu_cookie). 0 = derive from the negotiated MSS. Effective only with mtuProbing
        int initialCwnd = default(0); // initial congestion window in SEGMENTS, overriding initialWindow -- models a route's `initcwnd` attribute. 0 = derive from initialWindow
        int notsentLowat @unit(B) = default(-1B); // TCP_NOTSENT_LOWAT (Linux sockopt, no RFC): write-readiness low-water mark on queued-but-not-yet-transmitted bytes (sendQueue bytes ahead of snd_nxt). Once exceeded, no further TCP_I_SEND_MSG indication is sent until it drops back at or below this value. -1B = disabled (no notsentLowat-driven signaling; independent of the runtime-settable sendQueueLimit/TCP_C_QUEUE_BYTES_LIMIT mechanism, which tracks total outstanding+unsent bytes instead).
        int msl @unit(s) = default(120s);   // Maximum Segment Lifetime
        string tcpAlgorithmClass @examples("TcpCubic", "TcpVegas", "TcpWestwood", "DcTcp", "TcpNewReno", "TcpReno", "TcpTahoe", "TcpNoCongestionControl") = default("TcpCubic");
        int dupthresh = default(3); // Used for TcpTahoe, TcpReno, and SACK (RFC 6675) DO NOT change unless you really know what you are doing
        bool frtoEnabled = default(true); // F-RTO spurious-RTO detection (RFC 5682, SACK-enhanced variant): after a retransmission timeout, an ACK/SACK covering data that was never retransmitted proves the original flight arrived and the timeout was spurious -- the pre-RTO cwnd/ssthresh are restored. ~ sysctl tcp_frto=2 (the Linux default)
        bool tlpEnabled = default(true); // Tail Loss Probe (RFC 8985 section 7.2): probe ~2*SRTT after the last transmission so a lost tail is repaired without waiting for the RTO; requires SACK. ~ Linux sysctl tcp_early_retrans=3 (its default)
        bool seedRttFromHandshake = default(true); // Seed the RTT estimator (srtt/rttvar/RTO) from the SYN<->SYN-ACK exchange, as Linux does. The first-flight RTO and TLP probe timeout become RTT-scaled instead of initialRto.
        bool adaptiveReorderingEnabled = default(true); // Grow the reordering degree (dynamic DupThresh) when SACK reveals reordering, so persistent reordering stops causing spurious fast retransmits. Requires sackSupport.
        int maxReordering = default(300); // Upper bound on the learned reordering degree in segments (Linux tcp_max_reordering).
        bool lossUndoEnabled = default(true); // Undo a spurious cwnd/ssthresh reduction when D-SACK (RFC 2883) or timestamps (RFC 3522 Eifel) reveal the retransmission was unnecessary.
        bool prrEnabled = default(true); // Proportional Rate Reduction (RFC 6937) during fast recovery instead of classic cwnd inflation/halving. Requires sackSupport.
        string lossDetectionMode @enum("dupthresh","rack") = default("rack"); // SACK loss detection: classic DupThresh (RFC 6675) or RACK time-based (RFC 8985). RACK requires sackSupport.
        double receiveBufferSize @unit(B) = default(-1B) @mutable; // receive-BUFFER capacity, decoupled from the advertised window (Linux sk_rcvbuf, initialized from tcp_rmem[1] / SO_RCVBUF): out-of-order data is buffered up to this limit even when it exceeds the advertised window. -1 = same as the advertised window (historical INET behavior)
        bool receiveBufferLocked @mutable = default(false); // the application pinned receiveBufferSize with SO_RCVBUF (Linux SOCK_RCVBUF_LOCK): the buffer no longer grows under receive pressure, so its free space -- not the unread byte count -- is what bounds the advertised window, and an arrival that no longer fits is dropped. @mutable so a harness/app modeling setsockopt(SO_RCVBUF) can set it before a connection is configured
        bool windowAutoTuning = default(false); // receiver window auto-tuning (Linux tcp_grow_window): the offered window starts at advertisedWindow and grows with received data toward a clamp derived from receiveBufferSize. Requires receiveBufferSize to be set.
        double rackReoTimerGranularity @unit(s) = default(0s); // model the kernel's jiffy-quantized REO timeout arming (Linux usecs_to_jiffies(timeout)+1: delay rounded UP to this granularity plus one extra tick, e.g. 4ms for an HZ=250 kernel); a sub-tick RACK deadline then fires whole ticks later, letting a dupthresh-timed SACK enter recovery first, as real kernels do. 0 = exact (default INET behavior)
        bool syncookiesAlways = default(false); // model Linux sysctl tcp_syncookies=2 (cookies on every passive open): the server's connection is rebuilt from the cookie, whose 2-bit MSS field quantizes the peer's advertised MSS DOWN to the IPv4 msstab {536, 1300, 1440, 1460} (net/ipv4/syncookies.c); other option state (wscale/SACK/TS) survives via the timestamp encoding and stays exact
        bool windowShrinkAllowed = default(false); // model Linux sysctl tcp_shrink_window=1: the advertised window follows the receive buffer's genuine free space (__tcp_select_window's shrink branch) and may move the right edge DOWN as unread data accumulates -- occupancy is accounted at Linux's skb-truesize granularity and scaled by the measured payload/truesize ratio (tcp_scaling_ratio), so unread bytes cost what they cost a real socket buffer. Needs receiveBufferSize and an application that leaves data unread (e.g. a packetdrill explicit-read run) to differ from the default no-shrink policy
        int initialSsthresh = default(0xFFFFFFFF); // Initial value for Slow Start threshold used in TahoeRenoFamily. The initial value of ssthresh SHOULD be set arbitrarily high (e.g., to the size of the largest possible advertised window) Without user interaction there is no limit...
        // TcpCubic (RFC 9438) tuning; effective only with tcpAlgorithmClass=TcpCubic
        double cubicBeta = default(0.7); // multiplicative-decrease factor applied to cwnd on loss. Note that the TCP-friendliness estimator keeps the kernel's fixed scale (derived from beta=717/1024), so changing this only alters the decrease itself
        double cubicC = default(0.4); // scaling constant C of the cubic window-growth curve
        bool cubicFastConvergence = default(true); // on a loss below the previous W_max, release extra window so a newly arrived competing flow can grow (RFC 9438 section 4.7)
        bool cubicTcpFriendliness = default(true); // never grow slower than an AIMD(1, beta) Reno flow would (RFC 9438 section 4.3)
        double cubicDelta @unit(s) = default(10ms); // settling time after a window reduction, during which RTT samples are ignored
        int cubicCntClamp = default(20); // cap on the per-increment ACK count while no W_max is known yet (before the first loss)
        bool hystartEnabled = default(true); // HyStart: leave slow start when the path appears full, instead of overshooting into loss
        int hystartDetect = default(3); // HyStart detectors, as a bitmask: 1 = ACK train, 2 = delay increase, 3 = both
        int hystartLowWindow = default(16); // smallest cwnd (in segments) at which HyStart starts looking for the exit point
        double hystartAckDelta @unit(s) = default(2ms); // largest ACK spacing still counted as part of an ACK train
        double hystartDelayMin @unit(s) = default(4ms); // lower clamp on the HyStart delay-increase threshold (an eighth of the minimum RTT)
        double hystartDelayMax @unit(s) = default(16ms); // upper clamp on the same threshold (Linux HYSTART_DELAY_MAX)
        double stopOperationExtraTime @unit(s) = default(0s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        bool ecnWillingness = default(false); // DEPRECATED -- use tcpEcnMode. true maps to tcpEcnMode="rfc3168" unless tcpEcnMode is ALSO explicitly set to something other than "off" (then a cRuntimeError is thrown -- same precedence rule as increasedIWEnabled vs initialWindow)
        string tcpEcnMode @mutable @enum("off","passive","rfc3168","accecn","accecn-passive") = default("off"); // @mutable so a harness can model a mid-run "sysctl net.ipv4.tcp_ecn=N": the value is read per connection at open time, exactly as Linux samples it
            // Value semantics modeled on Linux net.ipv4.tcp_ecn (0/1/2/3/4/5; AccECN owns 3/4/5):
            //   off            ~ tcp_ecn=0: never request or accept any form of ECN (today's default)
            //   passive        ~ tcp_ecn=2: accept a peer-initiated classic ECN-setup SYN, never
            //                    initiate ECN on active OPEN. Linux's real out-of-box default.
            //   rfc3168        ~ tcp_ecn=1/4: always send a classic ECN-setup SYN on active OPEN;
            //                    accept passively when listening. Equivalent to ecnWillingness=true.
            //   accecn         ~ tcp_ecn=3: attempt AccECN negotiation on active OPEN and accept it
            //                    when listening; fall back to rfc3168 or off per negotiation outcome.
            //   accecn-passive ~ tcp_ecn=5: accept AccECN when listening, never initiate it on
            //                    active OPEN (mirrors "passive" but for AccECN).
        bool accEcnOptionEnabled @mutable = default(false); // Send/parse the AccECN TCP option (E0B/E1B/CEB byte counters). Requires tcpEcnMode in {"accecn","accecn-passive"}
        int accEcnOptionBeaconAcks = default(4); // Send the AccECN option at least once every N ACKs even when not otherwise demanded (simplified beaconing policy -- Linux's full demand/beacon/DSACK-triggered state machine is not ported)
        bool accEcnOptionKindAlternates = default(true); // If true, alternate the AccECN option between kind 172 (E0B,CEB,E1B) and 174 (E1B,CEB,E0B) on successive sends (draft middlebox-robustness). If false, always emit kind 174 -- Linux's observed behavior.
        double dctcpGamma = default(0.0625); // A fixed estimation gain for calculating dctcp_alpha (RFC 8257 4.2)
        bool pmtudEnabled = default(false); // Path MTU Discovery (RFC 1191, RFC 1981) enabled/disabled
        double pmtudTimeout @unit(s) = default(600s); // Time after which the original MSS is restored to probe for increased path MTU (RFC 1191, Section 6.3)
        volatile int initialSendSequenceNumber = default(-1); // -1 means the value is calculated from the current simulation time
        double initialRto @unit(s) = default(1s); // initial retransmission timeout, used until the first RTT measurement completes (RFC 6298 section 2.1 specifies 1s; 3s is the historical RFC 1122 value)
        int synLinearTimeouts = default(4); // Linux net.ipv4.tcp_syn_linear_timeouts: number of initial CLIENT SYN retransmits fired at the (non-backed-off) initial RTO before exponential backoff begins; 0 = double from the first retransmit
        int synRetries @mutable = default(-1); // Linux net.ipv4.tcp_syn_retries / per-socket TCP_SYNCNT: maximum number of SYN (and SYN-ACK) retransmissions before the connection attempt is aborted; -1 = INET's historical MAX_SYN_REXMIT_COUNT (12). @mutable so a test harness can inject the per-socket sockopt at runtime.
        double minRexmitTimeout @unit(s) = default(0.2s); // minimum retransmission timeout
        double maxRexmitTimeout @unit(s) = default(120s); // maximum retransmission timeout
        int maxRexmitCount = default(12); // maximum retransmission count
        double minPersistTimeout @unit(s) = default(5s); // minimum persist timeout
        double maxPersistTimeout @unit(s) = default(60s); // maximum persist timeout
        double delayedAckTimeout @unit(s) = default(200ms); // maximum time to wait before sending a delayed ACK
        bool sendDataWithFirstAck = default(true); // send data in 3rd packet in SYN - SYN+ACK - ACK sequence
        bool alignOptions = default(true);  // enable/disable per-option predefined alignment
        bool sendMssOption = default(true);  // enable/disable MSS option sending
        bool keepAliveEnabled = default(false); // TCP keepalive (RFC 1122 4.2.3.6). Off by default, like SO_KEEPALIVE on Linux
        double keepAliveIdleTime @unit(s) = default(7200s); // idle time before the first keepalive probe (Linux tcp_keepalive_time)
        double keepAliveInterval @unit(s) = default(75s); // interval between keepalive probes (Linux tcp_keepalive_intvl)
        int keepAliveProbeCount = default(9); // number of unanswered probes before the connection is aborted (Linux tcp_keepalive_probes)
        bool fastopenClientEnabled @mutable = default(false); // TCP Fast Open (RFC 7413) client role: attempt SYN-with-data when a cookie is cached. ~ sysctl tcp_fastopen bit 0 (TFO_CLIENT_ENABLE)
        bool fastopenServerEnabled @mutable = default(false); // TCP Fast Open (RFC 7413) server role: accept and accelerate a validated-cookie SYN-with-data. ~ sysctl tcp_fastopen bit 1 (TFO_SERVER_ENABLE)
        bool fastopenAcceptWithoutCookie @mutable = default(false); // Server role: accept SYN-with-data even when the SYN carries no Fast Open cookie option at all (RFC 7413 section 4.1.3's cookie-less mode). ~ sysctl tcp_fastopen bit 9 (0x200, TFO_SERVER_COOKIE_NOT_REQD). Only meaningful with fastopenServerEnabled.
        bool fastopenExpOptionEnabled @mutable = default(false); // Understand the pre-standardization kind-254 + magic-0xF989 Fast Open option (RFC 7413 Appendix A). Emitted only in reply to a peer that used it -- a server echoes the client's form in its SYN-ACK, and a client that got no answer to a kind-34 request retries once with this one, as Linux does. Inert unless fastopenServerEnabled or fastopenClientEnabled is also true
        bool fastopenLenientCookieValidation = default(true); // Accept any syntactically valid (4-16 byte) Fast Open cookie as valid without a keyed-hash check -- simulation-appropriate (no adversarial clients on the wire); set false to require an exact match against the locally generated cookie
        int fastopenCookieBytes = default(8); // Length in bytes of the locally generated Fast Open cookie (RFC 7413 allows 4-16)
        double fastopenBlackholeTimeout @unit(s) = default(0s); // Client-side active-TFO blackhole detection: base disable duration after a suspected middlebox interference event (RTO or out-of-order RST on a data-carrying TFO SYN), doubling (capped at 64x) on repeated events. 0s (default) disables the detector entirely, matching Linux's own shipped default -- the local corpus's defaults.sh override to a nonzero value is a *test* configuration, not upstream's
        bool fastopenClientNoCookieRequired @mutable = default(false); // Linux net.ipv4.tcp_fastopen bit 0x4 (TFO_CLIENT_NO_COOKIE): the client sends SYN+data immediately with NO cookie (and no cookie-request) at all -- for networks where the server is known to accept cookie-less TFO
        string fastopenKey @mutable = default(""); // Server-role Fast Open cookie key in Linux net.ipv4.tcp_fastopen_key format ("xxxxxxxx-xxxxxxxx-xxxxxxxx-xxxxxxxx", primary key only). When set, the 8-byte cookie is derived exactly as Linux does (SipHash-2-4 of source||destination IPv4 address under this key), so cookies are deterministic and interoperable with captures; when empty (default), a simulation-local keyed hash is used instead
        int fastopenCookieCacheSize = default(1024); // Upper bound on the number of distinct destinations the client-role Fast Open cookie cache retains; evicts an arbitrary (not LRU) entry once full
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

