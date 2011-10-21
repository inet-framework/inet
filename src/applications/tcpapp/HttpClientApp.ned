//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
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

//
// Client for a generic request-response style protocol over TCP.
// May be used as a rough model of HTTP or FTP users.
// Compatible with both IPv4 and IPv6.
//
// The model communicates with the server in sessions. During a session,
// the client opens a single \TCP connection to the server, sends several
// requests (always waiting for the complete reply to arrive before
// sending a new request), and closes the connection.
//
// The server app should be TCPGenericSrvApp; the model sends GenericAppMsg
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
// Note that this module doesn't open separate \TCP connections for commands
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
// @see TCPGenericSrvApp, GenericAppMsg, TelnetApp
//
simple HttpClientApp like TCPApp2
{
    parameters:
        @display("i=block/app");
		@signal[sessionDelay](type="double");
		@signal[sessionTransferRate](type="double");
		@statistic[sessionDelay](title="session delay"; record=stats,percentile?);
		@statistic[sessionTransferRate](title="session transfer rate"; record=stats);

        string address = default(""); // may be left empty ("")
        int port = default(-1); // port number to listen on
        string connectAddress = default("");  // server address (may be symbolic)
        int connectPort = default(80); // port number to connect to (80 is for HTTP)
        double startTime @unit("s") = default(1s); // time first session begins
        volatile int htmlObjectLength @unit("B") = default(10KiB); // length of an HTML object
        volatile int numEmbeddedObjects = default(1); // number of embedded objects per HTML object
        volatile int embeddedObjectLength @unit("B") = default(12KiB); // length of an embedded object
        volatile int requestLength @unit(B) = default(200B); // length of a request
        volatile double parsingTime @unit("s") = default(0s); // time gap between requests for HTML object and 1st embedded object
        volatile double thinkTime @unit("s") = default(0s); // time gap between requests
        volatile double idleInterval @unit(s); // time gap between sessions
        volatile double reconnectInterval @unit("s") = default(30s);  // if connection breaks, waits this much before trying to reconnect
    gates:
        input tcpIn;
        output tcpOut;
}
