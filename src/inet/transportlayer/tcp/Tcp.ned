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
// TCP protocol implementation.
// See the ~ITcp for the Tcp layer general informations.
//
// This implementation supports:
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
// This module is compatible with both ~Ipv4 and ~Ipv6.
//
// A TCP segment is represented by the class ~TcpHeader.
//
// <b>Communication with clients</b>
//
// For communication between client applications and TCP, the ~TcpCommandCode
// and ~TcpStatusInd enums are used as message kinds, and ~TcpCommand
// and its subclasses are used as control info.
//
// To open a connection from a client app, send a cMessage to TCP with
// TCP_C_OPEN_ACTIVE as message kind and a ~TcpOpenCommand object filled in
// and attached to it as control info. (The peer Tcp will have to be LISTENing;
// the server app can achieve this with a similar cMessage but TCP_C_OPEN_PASSIVE
// message kind.) With passive open, there's a possibility to cause the connection
// "fork" on an incoming connection, leaving the original connection LISTENing
// on the port (see the fork field in ~TcpOpenCommand).
//
// The client app can send data by assigning the TCP_C_SEND message kind to the data packet,
// and sending it to TCP. The server app will receive data as messages
// with the TCP_I_DATA message kind.
//
// To close, the client sends a cMessage to TCP with the TCP_C_CLOSE message kind
// and ~TcpCommand control info.
//
// Tcp sends notifications to the application whenever there's a significant
// change in the state of the connection: established, remote TCP closed,
// closed, timed out, connection refused, connection reset, etc. These
// notifications are also cMessages with message kind TCP_I_xxx
// (TCP_I_ESTABLISHED, etc.) and ~TcpCommand as control info.
//
// One TCP module can serve several application modules, and several
// connections per application. When talking to applications, a
// connection is identified by the socketId that is assigned by the application in
// the OPEN call.
//
// <b>Sockets</b>
//
// The TcpSocket C++ class is provided to simplify managing Tcp connections
// from applications. TcpSocket handles the job of assembling and sending
// command messages (OPEN, CLOSE, etc) to TCP, and it also simplifies
// the task of dealing with packets and notification messages coming from Tcp.
//
// <b>Communication with the IP layer</b>
//
// The TCP model relies on sending and receiving ~L3AddressReq/~L3AddressInd tags
// attached to TCP segment packets.
//
// <b>Configuring TCP</b>
//
//   -# use the module parameter (limitedTransmitEnabled) to enabled/disabled
//      Limited Transmit algorithm (RFC 3042) integrated to TcpBaseAlg
//      (can be used for TcpNewReno, TcpReno, TcpTahoe and TcpNoCongestionControl but not
//      for DumbTcp).
//
//   -# use the module parameter (increasedIWEnabled) to change Initial Window
//      from one segment (RFC 2001) (based on MSS) to maximal four segments
//      (min(4*MSS, max (2*MSS, 4380 bytes))) (RFC 3390) integrated to
//      TcpBaseAlg (can be used for TcpNewReno, TcpReno, TcpTahoe and TcpNoCongestionControl
//      but not for DumbTcp).
//
// The TCP flavour supported depends on the value of the tcpAlgorithmClass
// module parameter, e.g. "TcpTahoe" or "TcpReno". In the future, other
// classes can be written which implement Vegas, LinuxTcp (which
// differs from others) or other variants.
//
// Note that ~TcpOpenCommand allows tcpAlgorithmClass to be chosen per-connection.
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
// The TCP module itself implements the following:
//  - all RFC 793 TCP states and state transitions
//  - connection setup and teardown as in RFC 793
//  - generally, RFC 793 compliant segment processing
//  - all socked commands (except RECEIVE) and indications
//  - receive buffer to cache above-sequence data and data not yet forwarded
//    to the user
//  - CONN-ESTAB timer, SYN-REXMIT timer, 2MSL timer, FIN-WAIT-2 timer
//  - The basic SACK implementation (RFC 2018 and RFC 2883) is located in
//    TCP main (and not in flavours).
//    This means that all existing TCP algorithm classes may be used with
//    SACK, although currently only TcpReno makes sense.
//  - RFC 3517 - (SACK)-based Loss Recovery algorithm which is a conservative
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
//  - TcpTahoe (Fast Retransmit), TcpReno (Fast Retransmit and Fast Recovery), TcpNewReno (Fast Retransmit and Fast Recovery)
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
// The above problems are relatively easy to fix, and will be resolved in the
// next iteration. Also, other TCPAlgorithms will be added.
//
// <b>Tests</b>
//
// There are automated test cases (*.test files) for TCP -- see the <i>tests</i>
// directory in the source distribution.
//
// Please also see ChangeLog.
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
        int windowScalingFactor = default(-1); // Window Scaling Factor to the power of 2. -1 indicates that manual window scaling is turned off.
        bool timestampSupport = default(false); // Timestamps (RFC 1323) support (header option) (TS will be enabled for a connection if both endpoints support it)
        int mss = default(536); // Maximum Segment Size (RFC 793) (header option)
        int msl @unit(s) = default(120s);   // Maximum Segment Lifetime
        string tcpAlgorithmClass = default("TcpReno"); // TODO: use the following @enum or similar, but it must allow new values for user defined algorithms (omnetpp-6.1 checks @enum strictly): @enum("TcpVegas", "TcpWestwood", "TcpNewReno", "TcpReno", "TcpTahoe", "TcpNoCongestionControl")
        bool useDataNotification = default(false); // turn the notifications for arrived data on or off
        int dupthresh = default(3); // used for TcpTahoe, TcpReno and SACK (RFC 3517) DO NOT change unless you really know what you are doing
        int initialSsthresh = default(0xFFFFFFFF); // initial value for Slow Start threshold used in TahoeRenoFamily. The initial value of ssthresh SHOULD be set arbitrarily high (e.g.,to the size of the largest possible advertised window) Without user interaction there is no limit...
        double stopOperationExtraTime @unit(s) = default(0s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        bool ecnWillingness = default(false); // true if willing to use ECN
        double dctcpGamma = default(0.0625); // A fixed estimation gain for calculating dctcp_alpha (RFC 8257 4.2)
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

