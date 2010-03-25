//
// Copyright (C) 2006 Sam Jansen, Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


package inet.transport.tcp_nsc;

import inet.transport.tcp.ITCP;


//
// \TCP model based on the Network Simulation Cradle by Sam Jansen.
// \TCP segments are represented by the class ByteArrayMessage which 
// carries a network byte order packet. Compatible with both IPv4 and IPv6.
//
// <b>Communication with clients</b>
//
// For communication between client applications and TCP, the TcpCommandCode
// and TcpStatusInd enums are used as message kinds, and TCPCommand
// and its subclasses are used as control info.
//
// To open a connection from a client app, send a cMessage to TCP with
// TCP_C_OPEN_ACTIVE as message kind and a TCPOpenCommand object filled in
// and attached to it as control info. (The peer TCP will have to be LISTENing;
// the server app can achieve this with a similar cMessage but TCP_C_OPEN_PASSIVE
// message kind.) With passive open, there's a possibility to cause the connection
// "fork" on an incoming connection, leaving the original connection LISTENing
// on the port (see the fork field in TCPOpenCommand).
//
// The client app can send data by assigning the TCP_C_SEND message kind
// and attaching a TCPSendCommand control info object to the data packet,
// and sending it to TCP. The server app will receive data as messages
// with the TCP_I_DATA message kind and TCPSendCommand control info.
// (Whether you'll receive the same or identical messages, or even whether
// you'll receive data in the same sized chunks as sent depends on the
// sendQueueClass and receiveQueueClass used, see below. With
// TCPVirtualDataSendQueue and TCPVirtualDataRcvQueue set, message objects
// and even message boundaries are not preserved.)
//
// To close, the client sends a cMessage to TCP with the TCP_C_CLOSE message kind
// and TCPCommand control info.
//
// TCP sends notifications to the application whenever there's a significant
// change in the state of the connection: established, remote TCP closed,
// closed, timed out, connection refused, connection reset, etc. These
// notifications are also cMessages with message kind TCP_I_xxx
// (TCP_I_ESTABLISHED, etc.) and TCPCommand as control info.
//
// One TCP module can serve several application modules, and several
// connections per application. The <i>k</i>th application connects to TCP's
// from_appl[k] and appOut[k] ports. When talking to applications, a
// connection is identified by the (application port index, connId) pair,
// where connId is assigned by the application in the OPEN call.
//
// <b>Sockets</b>
//
// The TCPSocket C++ class is provided to simplify managing \TCP connections
// from applications. TCPSocket handles the job of assembling and sending
// command messages (OPEN, CLOSE, etc) to TCP, and it also simplifies
// the task of dealing with packets and notification messages coming from TCP.
//
// <b>Communication with the \IP layer</b>
//
// The TCP model relies on sending and receiving IPControlInfo objects
// attached to \TCP segment objects as control info
// (see cMessage::setControlInfo()).
//
simple TCP_NSC like ITCP
{
    parameters:
        // full library name of the tcp stack implementation
        string stackName = default("liblinux2.6.26.so") @choice("liblinux2.6.10.so","liblinux2.6.18.so","liblinux2.6.26.so","liblwip.so","libopenbsd3.5.so","libfreebsd5.3.so");
        int stackBufferSize = default(80000); // the size of buffer used when communicating with the specified NSC stack
        string sendQueueClass = default("TCP_NSC_VirtualDataSendQueue");
        string receiveQueueClass = default("TCP_NSC_VirtualDataReceiveQueue");
    gates:
        input appIn[];
        input ipIn;
        input ipv6In;
        output appOut[];
        output ipOut;
        output ipv6Out;
}

