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

import inet.applications.contract.IApp;

//
// Single-connection TCP application: it opens a connection, sends
// the given number of bytes, and closes. Sending may be one-off,
// or may be controlled by a "script" which is a series of
// (time, number of bytes) pairs. May act either as client or as server.
// Compatible with both IPv4 (~Ipv4) and IPv6 (~Ipv6).
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
// Currently you have three dataTransferMode choices:
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
//      This mode is not implemented in ~TcpNsc yet.
//
//   -# use "bytestream", which transmits real bytes of messages.
//
simple TcpSessionApp like IApp
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
        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // if not -1, set the DSCP (IPv4/IPv6) field of sent packets to this value
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        @signal[connect](type=long);
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}

