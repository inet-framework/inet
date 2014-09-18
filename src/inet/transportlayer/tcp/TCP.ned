//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2009-2010 Thomas Reschka
// Copyright (C) 2010 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.transportlayer.tcp;

import inet.transportlayer.ITCP;


//
// TCP protocol implementation.
// See the ~ITCP for the TCP layer general informations.
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
// This module is compatible with both ~IPv4 and ~IPv6.
//
// A TCP segment is represented by the class ~TCPSegment.
//
// <b>Communication with clients</b>
//
// For communication between client applications and TCP, the ~TcpCommandCode
// and ~TcpStatusInd enums are used as message kinds, and ~TCPCommand
// and its subclasses are used as control info.
//
// To open a connection from a client app, send a cMessage to TCP with
// TCP_C_OPEN_ACTIVE as message kind and a ~TCPOpenCommand object filled in
// and attached to it as control info. (The peer TCP will have to be LISTENing;
// the server app can achieve this with a similar cMessage but TCP_C_OPEN_PASSIVE
// message kind.) With passive open, there's a possibility to cause the connection
// "fork" on an incoming connection, leaving the original connection LISTENing
// on the port (see the fork field in ~TCPOpenCommand).
//
// The client app can send data by assigning the TCP_C_SEND message kind
// and attaching a ~TCPSendCommand control info object to the data packet,
// and sending it to TCP. The server app will receive data as messages
// with the TCP_I_DATA message kind and ~TCPSendCommand control info.
//
// To close, the client sends a cMessage to TCP with the TCP_C_CLOSE message kind
// and ~TCPCommand control info.
//
// TCP sends notifications to the application whenever there's a significant
// change in the state of the connection: established, remote TCP closed,
// closed, timed out, connection refused, connection reset, etc. These
// notifications are also cMessages with message kind TCP_I_xxx
// (TCP_I_ESTABLISHED, etc.) and ~TCPCommand as control info.
//
// One TCP module can serve several application modules, and several
// connections per application. The <i>k</i>th application connects to TCP's
// appIn[k] and appOut[k] ports. When talking to applications, a
// connection is identified by the (application port index, connId) pair,
// where connId is assigned by the application in the OPEN call.
//
// <b>Sockets</b>
//
// The TCPSocket C++ class is provided to simplify managing TCP connections
// from applications. TCPSocket handles the job of assembling and sending
// command messages (OPEN, CLOSE, etc) to TCP, and it also simplifies
// the task of dealing with packets and notification messages coming from TCP.
//
// <b>Communication with the IP layer</b>
//
// The TCP model relies on sending and receiving ~IPv4ControlInfo objects
// attached to TCP segment objects as control info
// (see cMessage::setControlInfo()).
//
// <b>Configuring TCP</b>
//
// The obsoleted sendQueueClass and receiveQueueClass parameters are removed!
// You must set the dataTransferMode in ~TCPOpenCommand message.
//
//   -# use the module parameter (limitedTransmitEnabled) to enabled/disabled
//      Limited Transmit algorithm (RFC 3042) integrated to TCPBaseAlg
//      (can be used for TCPNewReno, TCPReno, TCPTahoe and TCPNoCongestionControl but not
//      for DumbTCP).
//
//   -# use the module parameter (increasedIWEnabled) to change Initial Window
//      from one segment (RFC 2001) (based on MSS) to maximal four segments
//      (min(4*MSS, max (2*MSS, 4380 bytes))) (RFC 3390) integrated to
//      TCPBaseAlg (can be used for TCPNewReno, TCPReno, TCPTahoe and TCPNoCongestionControl
//      but not for DumbTCP).
//
// The TCP flavour supported depends on the value of the tcpAlgorithmClass
// module parameter, e.g. "TCPTahoe" or "TCPReno". In the future, other
// classes can be written which implement Vegas, LinuxTCP (which
// differs from others) or other variants.
//
// Note that ~TCPOpenCommand allows tcpAlgorithmClass to be chosen per-connection.
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
//    SACK, although currently only TCPReno makes sense.
//  - RFC 3517 - (SACK)-based Loss Recovery algorithm which is a conservative
//    replacement of the fast recovery algorithm (RFC2581) integrated to
//    TCPReno but not to TCPNewReno, TCPTahoe, TCPNoCongestionControl and DumbTCP.
//  - changes from RFC 2001 to RFC 2581:
//      - ACK generation (ack_now = true) RFC 2581, page 6: "(...) a TCP receiver SHOULD send an immediate ACK
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
// The TCPNewReno, TCPReno and TCPTahoe algorithms implement:
//  - RFC 1122 - delayed ACK algorithm (optional) with 200ms timeout
//  - RFC 896 - Nagle's algorithm (optional)
//  - Jacobson's and Karn's algorithms for round-trip time measurement and
//    adaptive retransmission
//  - TCPTahoe (Fast Retransmit), TCPReno (Fast Retransmit and Fast Recovery), TCPNewReno (Fast Retransmit and Fast Recovery)
//  - RFC 3390 - Increased Initial Window (optional) integrated to TCPBaseAlg
//    (can be used for TCPNewReno, TCPReno, TCPTahoe and TCPNoCongestionControl but not
//    for DumbTCP).
//  - RFC 3042 - Limited Transmit algorithm (optional) integrated to TCPBaseAlg
//    (can be used for TCPNewReno, TCPReno, TCPTahoe and TCPNoCongestionControl but not
//    for DumbTCP).
//
// Missing bits:
//  - URG and PSH bits not handled. Receiver always acts as if PSH was set
//    on all segments: always forwards data to the app as soon as possible.
//  - no RECEIVE command. Received data are always forwarded to the app as
//    soon as possible, as if the app issued a very large RECEIVE request
//    at the beginning. This means there's currently no flow control
//    between TCP and the app.
//  - all timeouts are precisely calculated: timer granularity (which is caused
//    by "slow" and "fast" i.e. 500ms and 200ms timers found in many *nix TCP
//    implementations) is not simulated
//  - new ECN flags (CWR and ECE). Need to be added to header by [RFC 3168].
//
// TCPNewReno/TCPReno/TCPTahoe issues and missing features:
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
simple TCP like ITCP
{
    parameters:
        int advertisedWindow = default(14*this.mss); // in bytes, corresponds with the maximal receiver buffer capacity (Note: normally, NIC queues should be at least this size)
        bool delayedAcksEnabled = default(false); // delayed ACK algorithm (RFC 1122) enabled/disabled
        bool nagleEnabled = default(true); // Nagle's algorithm (RFC 896) enabled/disabled
        bool limitedTransmitEnabled = default(false); // Limited Transmit algorithm (RFC 3042) enabled/disabled (can be used for TCPReno/TCPTahoe/TCPNewReno/TCPNoCongestionControl)
        bool increasedIWEnabled = default(false); // Increased Initial Window (RFC 3390) enabled/disabled
        bool sackSupport = default(false); // Selective Acknowledgment (RFC 2018, 2883, 3517) support (header option) (SACK will be enabled for a connection if both endpoints support it)
        bool windowScalingSupport = default(false); // Window Scale (RFC 1323) support (header option) (WS will be enabled for a connection if both endpoints support it)
        bool timestampSupport = default(false); // Timestamps (RFC 1323) support (header option) (TS will be enabled for a connection if both endpoints support it)
        int mss = default(536); // Maximum Segment Size (RFC 793) (header option)
        string tcpAlgorithmClass = default("TCPReno"); // TCPReno/TCPTahoe/TCPNewReno/TCPNoCongestionControl/DumbTCP
        bool recordStats = default(true); // recording of seqNum etc. into output vectors enabled/disabled
        string sendQueueClass = default("");    // Obsolete!!!
        string receiveQueueClass = default(""); // Obsolete!!!
        @display("i=block/wheelbarrow");
    gates:
        input appIn[] @labels(TCPCommand/down);
        input ipIn @labels(TCPSegment,IPv4ControlInfo/up,IPv6ControlInfo/up);
        output appOut[] @labels(TCPCommand/up);
        output ipOut @labels(TCPSegment,IPv4ControlInfo/down,IPv6ControlInfo/down);
}
