//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp;

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
// To allow abitrary modules send and receive data over TCP, you can use the
// ~TcpClientSocketIo and ~TcpServerSocketIo modules. Two module types,
// ~TcpClientApp and ~TcpServerApp, are extensible compound modules that build
// on top of the above two SocketIo modules and allow assembling TCP
// applications from pre-existing queueing components. There are also ready-made
// application models that use TCP, such as ~TcpSessionApp, ~TcpEchoApp, and
// ~TcpBasicClientApp/~TcpGenericServerApp.
//
// From C++, use the TcpSocket class, or see the Tcp class documentation for
// details on how to use the Tcp without the TcpSocket utility class.
//
// <b>Configuration Hints</b>
//
// TCP can be configured via module parameters. The following notes provide
// additional information compared to the module parameter documentation.
//
//   -# The tcpAlgorithmClass module parameter can be used to set the TCP flavor
//      to be used (TcpNewReno, TcpReno, TcpTahoe, etc.). Note that this setting
//      can be programmatically overridden on a per-connection basis. Also note
//      that the TcpNoCongestionControl and DumbTcp flavors are intentionally
//      simplified code that are only provided for testing/educational purposes,
//      and do not support many TCP features the more advances ones do.
//
//   -# The limitedTransmitEnabled parameter can be used to enable/disable the
//      Limited Transmit algorithm (RFC 3042).
//
//   -# The increasedIWEnabled parameter can be used to change the initial window
//      from one segment (RFC 2001) (based on MSS) to maximal four segments
//      (min(4*MSS, max (2*MSS, 4380 bytes))) (RFC 3390).
//
//   -# The advertisedWindow parameter defines the amount of data TCP can
//      receive without the socket being read by the local user. Note that
//      connections can be opened programmatically in one of two modes,
//      "autoread" and "explicit-read", and that in "autoread" mode the
//      advertised window never decreases so there is effectively no flow
//      control. Links with large bandwidth*delay product require large
//      advertised windows to operate efficiently. If the window size is larger
//      than 64K, window scaling support needs to be turned on (on both
//      endppoints) using the windowScalingSupport parameter, and the
//      windowScalingFactor set to the appropriate shift count.
//      windowScalingFactor=-1 sets the shift count automatically to a value
//      suitable for the advertisedWindow setting.
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
//   - RFC 3782 - The NewReno Modification to TCP's Fast Recovery Algorithm
//
// Implemented features include the following:
//  - all RFC 793 TCP states and state transitions
//  - connection setup and teardown as in RFC 793
//  - generally, RFC 793 compliant segment processing
//  - all socked commands and indications
//  - receive buffer to cache above-sequence data and data not yet forwarded
//    to the user
//  - CONN-ESTAB timer, SYN-REXMIT timer, 2MSL timer, FIN-WAIT-2 timer
//  - selective acknowledgements aka. SACK (RFC 2018 and RFC 2883)
//  - RFC 3517 - SACK-based Loss Recovery algorithm which is a conservative
//    replacement of the fast recovery algorithm (RFC2581) integrated to
//    TcpReno but not to TcpNewReno, TcpTahoe, TcpNoCongestionControl and DumbTcp.
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
//      - SACK: SACK option, based on RFC 2018, RFC 2883 and RFC 3517.
//      - TS: Timestamps option, based on RFC 1323.
//  - flow control: finite receive buffer size (initiated by parameter
//    advertisedWindow). If receive buffer is exhausted (by out-of-order
//    segments) and the payload length of a new received segment
//    is higher than free receiver buffer, the new segment will be dropped.
//    Such drops are recorded in tcpRcvQueueDropsVector.
//
// The TcpNewReno, TcpReno and TcpTahoe algorithms implement:
//  - RFC 1122 - delayed ACK algorithm (optional) with 200ms timeout
//  - RFC 896 - Nagle's algorithm (optional)
//  - Jacobson's and Karn's algorithms for round-trip time measurement and
//    adaptive retransmission
//  - TcpTahoe (Fast Retransmit), TcpReno (Fast Retransmit and Fast Recovery),
//    TcpNewReno (Fast Retransmit and Fast Recovery)
//  - RFC 3390 - Increased Initial Window (optional) integrated to TcpBaseAlg
//    (can be used for TcpNewReno, TcpReno, TcpTahoe and TcpNoCongestionControl but not
//    for DumbTcp).
//  - RFC 3042 - Limited Transmit algorithm (optional) integrated to TcpBaseAlg
//    (can be used for TcpNewReno, TcpReno, TcpTahoe and TcpNoCongestionControl but not
//    for DumbTcp).
//
// Missing bits:
//  - URG and PSH bits not handled. Receiver always acts as if PSH was set
//    on all segments: always forwards data to the app as soon as possible.
//  - no RECEIVE command. Received data are always forwarded to the app as
//    soon as possible, as if the app issued a very large RECEIVE request
//    at the beginning. This means there's currently no flow control
//    between Tcp and the app.
//  - all timeouts are precisely calculated: timer granularity (which is caused
//    by "slow" and "fast" i.e. 500ms and 200ms timers found in many *nix Tcp
//    implementations) is not simulated
//  - new ECN flags (CWR and ECE). Need to be added to header by [RFC 3168].
//
// TcpNewReno/TcpReno/TcpTahoe issues and missing features:
//  - KEEP-ALIVE not implemented (idle connections never time out)
//  - Nagle's algorithm (RFC 896) possibly not precisely implemented
//
// The above problems should be relatively easy to fix.
//
simple Tcp like ITcp
{
    parameters:
        string crcMode @enum("declared", "computed") = default("declared");
        int advertisedWindow = default(14*this.mss); // in bytes, corresponds with the maximal receiver buffer capacity (Note: normally, NIC queues should be at least this size)
        bool delayedAcksEnabled = default(false); // delayed ACK algorithm (RFC 1122) enabled/disabled
        bool nagleEnabled = default(true); // Nagle's algorithm (RFC 896) enabled/disabled
        bool limitedTransmitEnabled = default(false); // Limited Transmit algorithm (RFC 3042) enabled/disabled (can be used for TcpReno/TcpTahoe/TcpNewReno/TcpNoCongestionControl)
        bool increasedIWEnabled = default(false); // Increased Initial Window (RFC 3390) enabled/disabled
        bool sackSupport = default(false); // Selective Acknowledgment (RFC 2018, 2883, 3517) support (header option) (SACK will be enabled for a connection if both endpoints support it)
        bool windowScalingSupport = default(false); // Window Scale (RFC 1323) support (header option) (WS will be enabled for a connection if both endpoints support it)
        int windowScalingFactor = default(-1); // Window Scaling Factor given as a shift count. Valid values are 0..14, and -1 for automatic selection (it chooses the smallest shift count that makes advertisedWindow representable in 16 bits)
        bool timestampSupport = default(false); // Timestamps (RFC 1323) support (header option) (TS will be enabled for a connection if both endpoints support it)
        int mss = default(536); // Maximum Segment Size (RFC 793) (header option)
        int msl @unit(s) = default(120s);   // Maximum Segment Lifetime
        string tcpAlgorithmClass @enum("TcpVegas", "TcpWestwood", "DcTcp", "TcpNewReno", "TcpReno", "TcpTahoe", "TcpNoCongestionControl") = default("TcpReno");
        int dupthresh = default(3); // used for TcpTahoe, TcpReno and SACK (RFC 3517) DO NOT change unless you really know what you are doing
        int initialSsthresh = default(0xFFFFFFFF); // initial value for Slow Start threshold used in TahoeRenoFamily. The initial value of ssthresh SHOULD be set arbitrarily high (e.g.,to the size of the largest possible advertised window) Without user interaction there is no limit...
        double stopOperationExtraTime @unit(s) = default(0s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        bool ecnWillingness = default(false); // true if willing to use ECN
        double dctcpGamma = default(0.0625); // A fixed estimation gain for calculating dctcp_alpha (RFC 8257 4.2)
        volatile int initialSendSequenceNumber = default(-1); // -1 means the value is calculated from the current simulation time
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

