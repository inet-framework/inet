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
// Models Telnet sessions with a specific user behaviour.
// The server app should be TCPGenericSrvApp.
// Compatible with both IPv4 and IPv6.
//
// NOTE: This module emulates a very specific user behaviour, and as such,
// it should be viewed as an example rather than a generic Telnet model.
// If you want to model realistic Telnet traffic, you are encouraged
// to gather statistics from packet traces on a real network, and
// write your model accordingly.
//
simple TelnetApp
    parameters:
        address: string, // may be left empty ("")
        port: numeric const, // port number to listen on
        connectAddress: string,  // server address (may be symbolic)
        connectPort: numeric const, // port number to connect to
        startTime: numeric, // time first session begins
        numCommands: numeric, // user types this many commands in a session
        commandLength: numeric, // commands are this many characters (plus Enter)
        keyPressDelay: numeric, // delay between keypresses
        commandOutputLength: numeric, // commands produce this much output (bytes)
        thinkTime: numeric, // user waits this much before starting to type new command
        idleInterval: numeric, // time gap between sessions
        reconnectInterval;  // if connection breaks, user waits this much before trying to reconnect
    gates:
        in: tcpIn;
        out: tcpOut;
endsimple

