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


package inet.applications.tcpapp;

import inet.applications.ITCPApp;


//
// Accepts any number of incoming TCP connections, and sends back the
// messages that arrive on them, The lengths of the messages are
// multiplied by echoFactor before sending them back (echoFactor=1 will
// result in sending back the same message unmodified.) The reply can also be
// delayed by a constant time (echoDelay parameter).
//
// When ~TCPEchoApp receives data packets from ~TCP (and such, when they can be
// echoed) depends on the dataTransferMode setting.
// With "bytecount" and "bytestream", ~TCP passes up data to us
// as soon as a segment arrives, so it can be echoed immediately.
// With "object" mode, our local ~TCP reproduces the same
// messages that the sender app passed down to its ~TCP -- so if the sender
// app sent a single 100 MB message, it will be echoed only when all
// 100 megabytes have arrived.
//
// The module parameter dataTransferMode should be set the transfer mode in TCP layer.
// Currently you have three choices:
//
//   -# set them to "bytecount".
//      This mode manages "virtual bytes", that is, only byte counts are
//      transmitted over the TCP connection and no actual data. cMessage
//      contents, and even message boundaries are not preserved with these
//      classes: for example, if the client sends a single cMessage with
//      length = 1 megabyte over TCP, the receiver-side client will see a
//      sequence of MSS-sized messages.
//
//   -# use "object", which transmits
//      cMessage objects (and subclasses) over a TCP connection. The same
//      message object sequence that was sent by the client to the
//      sender-side TCP entity will be reproduced on the receiver side.
//      If a client sends a cMessage with length = 1 megabyte, the
//      receiver-side client will receive the same message object (or a clone)
//      after the TCP entities have completed simulating the transmission
//      of 1 megabyte over the connection. This is a different behaviour
//      from TCPVirtualDataSendQueue/RcvQueue.
//      This mode is not implemented in ~TCP_NSC yet.
//
//   -# use "bytestream", which transmits real bytes of messages.
//
// Compatible with both ~IPv4 and ~IPv6.
//
simple TCPEchoApp like ITCPApp
{
    parameters:
        string localAddress = default(""); // local address; may be left empty ("")
        int localPort; // port number to listen on
        double echoFactor = default(1);
        double echoDelay @unit(s) = default(0s);
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
        @display("i=block/app");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

