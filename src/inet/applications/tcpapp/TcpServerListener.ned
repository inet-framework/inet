//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// This module accepts/rejects TCP connections and creates TCP server connections
// as part of a generic TCP server application.
//
// @see ~TcpServerApp, ~TcpServerConnection
//
simple TcpServerListener like IApp
{
    parameters:
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(1000); // port number to listen on
        string serverConnectionModuleType = default("inet.applications.tcpapp.TcpServerConnection"); // module type of "connection" module to launch on incoming connections
        @display("i=block/server");
    gates:
        input socketIn;
        output socketOut;
}
