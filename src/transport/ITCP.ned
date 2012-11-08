//
// Copyright (C) 2004 Andras Varga
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


package inet.transport;

//
// Interface for TCP protocol implementations. All TCP implementations should
// implement this (i.e. declared as: TCP like ITCP)
// The existing implementations are these: ~TCP, ~TCP_NSC and ~TCP_lwIP.
//
// <b>Communication with applications</b>
//
// For communication between applications and TCP, the ~TcpCommandCode
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
// In the ~TCPOpenCommand you must choose a data transfer mode
// (see the dataTransferMode field in ~TCPOpenCommand).
// Note: Not implemented all ~TCPDataTransferMode in all TCP implementations.
//
// The app can send data by assigning the TCP_C_SEND message kind
// and attaching a ~TCPSendCommand control info object to the data packet,
// and sending it to TCP. The app will receive data as messages
// with the TCP_I_DATA message kind and ~TCPSendCommand control info.
// The data transmission mode is based on selected ~TCPDataTransferMode:
//   - TCP_TRANSFER_BYTECOUNT:
//      This mode manages "virtual bytes", that is, only byte counts are
//      transmitted over the TCP connection and no actual data. cPacket
//      contents, and even message boundaries are not preserved with these
//      classes: for example, if the client sends a single cPacket with
//      length = 1 megabyte over TCP, the receiver-side client will see a
//      sequence of MSS-sized packets.
//   - TCP_TRANSFER_OBJECT:
//      transmit cPacket objects (and subclasses) over a TCP connection. The
//      same message object sequence that was sent by the client to the
//      sender-side TCP entity will be reproduced on the receiver side.
//      If a client sends a cMessage with length = 1 megabyte, the
//      receiver-side client will receive the same message object (or a clone)
//      after the TCP entities have completed simulating the transmission
//      of 1 megabyte over the connection. This is a different behaviour
//      from TCPVirtualDataSendQueue/RcvQueue.
//   - TCP_TRANSFER_BYTESTREAM:
//      This mode manages "raw bytes", that is, bytes of ~ByteArrayMessage are
//      transmitted over the TCP connection.
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
// A TCP segment is represented by the class ~TCPSegment. The payload content
// of TCPSegment is depends on the selected ~TCPDataTransferMode in ~TCPOpenCommand.
//
// The TCP model relies on sending and receiving ~IPv4ControlInfo objects
// attached to TCP segment objects as control info
// (see cMessage::setControlInfo()).
//
moduleinterface ITCP
{
    @display("i=block/wheelbarrow");
    gates:
        input appIn[] @labels(TCPCommand/down);
        input ipIn @labels(TCPSegment,IPv4ControlInfo/up);
        input ipv6In @labels(TCPSegment,IPv6ControlInfo/up);
        output appOut[] @labels(TCPCommand/up);
        output ipOut @labels(TCPSegment,IPv4ControlInfo/down);
        output ipv6Out @labels(TCPSegment,IPv6ControlInfo/down);
}

simple TCP_None like ITCP
{
    parameters:
        @display("i=old/x_noentry");
        @class("UnimplementedModule");
    gates:
        input appIn[] @labels(TCPCommand/down);
        input ipIn @labels(TCPSegment,IPv4ControlInfo/up);
        input ipv6In @labels(TCPSegment,IPv6ControlInfo/up);
        output appOut[] @labels(TCPCommand/up);
        output ipOut @labels(TCPSegment,IPv4ControlInfo/down);
        output ipv6Out @labels(TCPSegment,IPv6ControlInfo/down);
}
