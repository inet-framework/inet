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

//
// This module hosts TCP-based server applications. It dynamically creates
// and launches a new "thread" object for each incoming connection.
//
// Server threads should be subclassed from the TCPServerThreadBase
// C++ class, registered in the C++ code using the Register_Class() macro,
// and the class name should be specified in the serverThreadClass
// parameter of TCPSrvHostApp. The thread object will receive events
// via a callback interface (methods like established(), dataArrived(),
// peerClosed(), timerExpired()), and can send packets via TCPSocket's send()
// method.
//
// Example server thread class: TCPGenericSrvThread (in the C++ documentation only).
//
// IMPORTANT: Before you try to use this module, make sure you actually need it!
// In most cases, TCPGenericSrvApp and GenericAppMsg will be completely
// enough, and they are a lot easier to handle. You'll want to subclass your
// client from TCPGenericCliAppBase then; check TelnetApp and TCPBasicClientApp
// for examples.
//
// Compatible with both IPv4 and IPv6.
//
simple TCPSrvHostApp like TCPApp
{
    parameters:
        string address = default(""); // may be left empty ("")
        int port = default(1000); // port number to listen on
        string serverThreadClass; // class name of "thread" objects to launch on incoming connections
        @display("i=block/app");
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

