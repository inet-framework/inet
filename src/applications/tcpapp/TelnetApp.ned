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
// Models Telnet sessions with a specific user behaviour.
// The server app should be ~TCPGenericSrvApp.
// Compatible with both IPv4 (~IP) and ~IPv6.
//
// NOTE: This module emulates a very specific user behaviour, and as such,
// it should be viewed as an example rather than a generic Telnet model.
// If you want to model realistic Telnet traffic, you are encouraged
// to gather statistics from packet traces on a real network, and
// write your model accordingly.
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
simple TelnetApp like ITCPApp
{
    parameters:
        string localAddress = default(""); // local address or empty ("")
        int localPort; // local port number
        string connectAddress;  // server address (may be symbolic)
        int connectPort = default(1000); // port number to connect to
        string dataTransferMode = default("bytecount");
        double startTime @unit(s) = default(1s); // time first session begins
        double stopTime @unit(s) = default(0);  // time of finish sending, 0 means infinity
        volatile int numCommands = default(exponential(10)); // user types this many commands in a session
        volatile int commandLength @unit(B) = default(10B); // commands are this many characters (plus Enter)
        volatile double keyPressDelay @unit(s) = default(exponential(0.1s)); // delay between keypresses
        volatile int commandOutputLength @unit(B) = default(exponential(1000B)); // commands produce this much output
        volatile double thinkTime @unit(s) = default(exponential(10s)); // user waits this much before starting to type new command
        volatile double idleInterval @unit(s) = default(exponential(300s)); // time gap between sessions
        volatile double reconnectInterval @unit(s) = default(30s);  // if connection breaks, user waits this much before trying to reconnect
        @display("i=block/telnet");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPkBytes](title="Received packet bytes"; source="packetBytes(rcvdPk)"; unit=B; record=count,sum,vector; interpolationmode=none);
        @statistic[sentPkBytes](title="Sent packet bytes"; source="packetBytes(sentPk)"; unit=B; record=count,sum,vector; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,stats,vector; interpolationmode=none);

    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

