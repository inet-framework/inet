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
import inet.common.MessageDispatcher;

//
// This module is a generic TCP server application with a TCP server listener
// that creates TCP server connections.
//
// @see ~TcpClientApp, ~TcpServerListener, ~TcpServerConnection
//
module TcpServerApp like IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        listener: <default("TcpServerListener")> like IApp {
            parameters:
                @display("p=100,100");
        }
        connection[0]: <default(listener.serverConnectionModuleType)> like IApp {
            parameters:
                @display("p=200,100,row,50");
        }
        ta: MessageDispatcher {
            parameters:
                @display("p=100,200");
        }
    connections:
        ta.out++ --> listener.socketIn;
        listener.socketOut --> ta.in++;
        ta.out++ --> { @display("m=s"); } --> socketOut;
        socketIn --> { @display("m=s"); } --> ta.in++;
}
