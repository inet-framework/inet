//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
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
// FTP client based on request-response style protocol over TCP that can handle
// very large-size fiiles.
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
// @see HttpClientApp, TCPGenericSrvApp, GenericAppMsg, TelnetApp
//
simple FtpClientApp like TCPApp2
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
        int connectPort = default(20); // port number to connect to (20 is for FTP)
        double startTime @unit("s") = default(1s); // time first session begins
        volatile int fileSize @unit("B") = default(10KiB); // size of file to download
        volatile int requestLength @unit(B) = default(200B); // length of a request
        volatile double idleInterval @unit(s); // time gap between sessions
        volatile double reconnectInterval @unit("s") = default(30s);  // if connection breaks, waits this much before trying to reconnect
    gates:
        input tcpIn;
        output tcpOut;
}
