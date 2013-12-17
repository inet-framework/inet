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
// Single-connection TCP application: it opens a connection, sends
// the given number of bytes, and closes. Sending may be one-off,
// or may be controlled by a "script" which is a series of
// (time, number of bytes) pairs. May act either as client or as server,
// and works with TCPVirtualBytesSendQueue/RcvQueue as sendQueue/receiveQueue
// setting for ~TCP.
// Compatible with both IPv4 (~IPv4) and ~IPv6.
//
// <b>Opening the connection</b>
//
// Regarding the type of opening the connection, the application may
// be either a client or a server. When active=false, the application
// will listen on the given local localPort, and wait for an incoming connection.
// When active=true, the application will bind to given local localAddress:localPort,
// and connect to the connectAddress:connectPort. To use an ephemeral port
// as local port, set the localPort parameter to -1.
//
// Even when in server mode (active=false), the application will only
// serve one incoming connection. Further connect attempts will be
// refused by TCP (it will send RST) for lack of LISTENing connections.
//
// The time of opening the connection is in the tOpen parameter.
//
// <b>Sending data</b>
//
// Regardless of the type of OPEN, the application can be made to send
// data. One way of specifying sending is via the tSend, sendBytes
// parameters, the other way is sendScript. With the former, sendBytes
// bytes will be sent at tSend. With sendScript, the format is
// "<time> <numBytes>;<time> <numBytes>;..."
//
// <b>Closing the connection</b>
//
// The application will issue a TCP CLOSE at time tClose. If tClose=-1, no
// CLOSE will be issued.
//
// <b>Configuring App</b>
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
simple TCPSessionApp like ITCPApp
{
    parameters:
        string localAddress = default("");
        int localPort = default(-1);  // local port
        bool active = default(true);
        string connectAddress;
        int connectPort = default(1000);
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
        double tOpen @unit(s) = default(1s);
        double tSend @unit(s) = default(1s);
        int sendBytes @unit(B) = default(1MiB);
        string sendScript = default("");
        double tClose @unit(s) = default(2s);
        @display("i=block/app");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @signal[recvIndications](type=cMessage);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

