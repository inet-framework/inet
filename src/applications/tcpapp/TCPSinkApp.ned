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
// <b>Configuring App</b>
//
// The module parameter dataTransferMode should be set the names of
// classes that manage the actual send and receive queues in TCP layer.
// Currently you have three choices:
//
//   -# set them to "bytecount".
//      These classes manage "virtual bytes", that is, only byte counts are
//      transmitted over the TCP connection and no actual data. cMessage
//      contents, and even message boundaries are not preserved with these
//      classes: for example, if the client sends a single cMessage with
//      length = 1 megabyte over TCP, the receiver-side client will see a
//      sequence of MSS-sized messages.
//
// It depends on the client (app) modules which sendQueue/rcvQueue they require.
// For example, ~TCPGenericSrvApp needs message-based sendQueue/rcvQueue,
// while ~TCPEchoApp or ~TCPSinkApp can work with any (but ~TCPEchoApp will
// display different behaviour with both!) In the future, other send queue
// and receive queue classes may be implemented, e.g. to allow transmission
// of "raw bytes" (actual byte arrays).
//
//   -# use "object", which transmit
//      cMessage objects (and subclasses) over a TCP connection. The same
//      message object sequence that was sent by the client to the
//      sender-side TCP entity will be reproduced on the receiver side.
//      If a client sends a cMessage with length = 1 megabyte, the
//      receiver-side client will receive the same message object (or a clone)
//      after the TCP entities have completed simulating the transmission
//      of 1 megabyte over the connection. This is a different behaviour
//      from TCPVirtualDataSendQueue/RcvQueue.
//      These classes are not implemented in ~TCP_NSC yet.
//
//   -# use "bytestream", which transmit real bytes of messages.
//      These classes are not implemented yet.
//


package inet.applications.tcpapp;

import inet.applications.ITCPApp;


//
// Accepts any number of incoming TCP connections, and discards whatever
// arrives on them.
// Compatible with both IPv4 (~IPv4) and ~IPv6.
//
simple TCPSinkApp like ITCPApp
{
    parameters:
        string address = default(""); // may be left empty ("")
        int port = default(1000); // port number to listen on
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
        @display("i=block/sink");
        @statistic[rcvdPkBytes](title="Received packet bytes"; record=sum,count,vector);
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

