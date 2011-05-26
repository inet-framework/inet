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
// Client for a generic request-response style protocol over TCP.
// May be used as a rough model of HTTP or FTP users.
// Compatible with both IPv4 (~IPv4) and ~IPv6.
//
// The model communicates with the server in sessions. During a session,
// the client opens a single TCP connection to the server, sends several
// requests (always waiting for the complete reply to arrive before
// sending a new request), and closes the connection.
//
// The server app should be ~TCPGenericSrvApp; the model sends ~GenericAppMsg
// messages.
//
// Example settings:
//
// FTP:
// <pre>
//    numRequestsPerSession = exponential(3)
//    requestLength = truncnormal(20,5)
//    replyLength = exponential(1000000)
// </pre>
//
// Note that this module doesn't open separate TCP connections for commands
// and data transfer as the FTP protocol.
//
// HTTP:
// <pre>
//    numRequestsPerSession = 1 <i>(HTTP 1.0)</i>
//    numRequestsPerSession = exponential(5) <i>(HTTP 1.1, with keepalive)</i>
//    requestLength = truncnormal(350,20)
//    replyLength = exponential(2000)
// </pre>
//
// Note that since most web pages contain images and may contain frames,
// applets etc, possibly from various servers, and browsers usually download
// these items in parallel to the main HTML document, this module cannot
// serve as a realistic web client.
//
// Also, with HTTP 1.0 it is the server that closes the connection after
// sending the response, while in this model it is the client.
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
// @see ~TCPGenericSrvApp, ~GenericAppMsg, ~TelnetApp
//
simple TCPBasicClientApp like ITCPApp
{
    parameters:
        string address = default(""); // may be left empty ("")
        int port = default(-1); // port number to listen on
        string connectAddress = default("");  // server address (may be symbolic)
        int connectPort = default(1000); // port number to connect to
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
        double startTime @unit("s") = default(1s); // time first session begins
        volatile int numRequestsPerSession = default(1);  // number of requests sent per session
        volatile int requestLength @unit(B) = default(200B); // length of a request
        volatile int replyLength @unit("B") = default(1MB); // length of a reply
        volatile double thinkTime @unit("s"); // time gap between requests
        volatile double idleInterval @unit(s); // time gap between sessions
        volatile double reconnectInterval @unit("s") = default(30s);  // if connection breaks, waits this much before trying to reconnect
        @display("i=block/app");
        @statistic[connect](title="Connections"; record=sum,timeavg,vector);
        @statistic[sentPkBytes](title="Sent packet bytes"; record=count,sum,vector);
        @statistic[rcvdPkBytes](title="Received packet bytes"; record=count,sum,vector);
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}
