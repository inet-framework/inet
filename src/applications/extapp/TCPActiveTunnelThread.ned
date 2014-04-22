//
// Copyright (C) 2014 OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// author: Zoltan Bojthe


package inet.applications.extapp;

// a real extTcpPort-on bejovo adatot tovabbitja a connectAddress:connectPort-ra a tcpOut gate-en,
// a tcpIn gate-en bejovo adatot pedig az extTcpPort-on kuldi ki.
// kezeli a CLOSE, stb parancsokat is.
simple TCPTunnelThread
{
    parameters:
        string dataTransferMode @enum("bytecount","object","bytestream") = "bytestream";
        string extTcpAddress = default(""); // may be left empty ("")
        int extTcpPort;     // real port number on computer
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(-1); // port number to listen on
        string connectAddress;  // server address (may be symbolic)
        int connectPort; // port number to connect to (port on the server)

    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}
