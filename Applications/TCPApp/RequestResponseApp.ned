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
// Client for a request-reply protocol over TCP. May be used to
// model an FTP or HTTP (1.0 or 1.1) user.
//
// The model communicates with the server in sessions. During a session,
// the client opens a single TCP connection to the server, sends several
// requests (always waiting for the complete reply to arrive before
// sending a new request), and closes the connection.
//
// The server app should be TCPGenericSrvApp.
//
// When modelling FTP: note that this module doesn't open separate TCP
// connections for commands and data transfer.
//
// To model an HTTP/1.0 user, set numRequestsPerSession=1;
// for HTTP/1.1 with keep-alive, numRequestsPerSession can be >1.
// Note that since most web pages contain images and may contain frames,
// applets etc, possibly from various servers, and browsers usually download
// these items in parallel to the main HTML document, this module cannot
// serve as a realistic web client.
//
simple RequestResponseApp
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

