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
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketFlow;
import inet.queueing.contract.IPassivePacketSink;

//
// This module is a generic TCP server connection. The traffic source and traffic
// sink modules can be built from queueing model elements.
//
// @see ~TcpServerApp
//
module TcpServerConnection like IApp
{
    parameters:
        @display("i=block/cogwheel");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        source: <default("ActivePacketSource")> like IActivePacketSource {
            parameters:
                @display("p=100,100");
        }
        outbound: <default("OmittedPacketFlow")> like IPacketFlow {
            parameters:
                @display("p=100,200");
        }
        sink: <default("PassivePacketSink")> like IPassivePacketSink {
            parameters:
                @display("p=200,100");
        }
        inbound: <default("OmittedPacketFlow")> like IPacketFlow {
            parameters:
                @display("p=200,200");
        }
        io: TcpServerSocketIo {
            parameters:
                @display("p=150,300");
        }
    connections:
        source.out --> outbound.in;
        outbound.out --> io.trafficIn;
        io.socketOut --> socketOut;

        socketIn --> io.socketIn;
        io.trafficOut --> inbound.in;
        inbound.out --> sink.in;
}
