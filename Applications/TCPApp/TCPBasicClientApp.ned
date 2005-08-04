//
// Copyright (C) 2004 Andras Varga
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
simple TCPBasicClientApp
    parameters:
        address: string, // may be left empty ("")
        port: numeric const, // port number to listen on
        connectAddress: string,  // server address (may be symbolic)
        connectPort: numeric const, // port number to connect to
        startTime: numeric, // time first session begins
        numRequestsPerSession: numeric,  // number of requests sent per session
        requestLength: numeric, // length of a request (bytes)
        replyLength: numeric, // length of a reply (bytes)
        thinkTime: numeric, // time gap between requests
        idleInterval: numeric, // time gap between sessions
        reconnectInterval: numeric;  // if connection breaks, waits this much before trying to reconnect
    gates:
        in: tcpIn;
        out: tcpOut;
endsimple

