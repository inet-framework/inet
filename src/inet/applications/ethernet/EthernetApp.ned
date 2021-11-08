//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.applications.ethernet;

import inet.applications.contract.IApp;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketFlow;
import inet.queueing.contract.IPassivePacketSink;

//
// This module generates traffic as an Ethernet application. The traffic source
// and traffic sink modules can be built from queueing model elements.
//
module EthernetApp like IApp
{
    parameters:
        string interfaceTableModule; // relative module path of the interface table
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        @application;
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        source: <default("ActivePacketSource")> like IActivePacketSource if typename != "" {
            parameters:
                @display("p=200,200");
        }
        outbound: <default("")> like IPacketFlow {
            parameters:
                @display("p=200,400");
        }
        sink: <default("PassivePacketSink")> like IPassivePacketSink if typename != "" {
            parameters:
                @display("p=600,200");
        }
        inbound: <default("")> like IPacketFlow {
            parameters:
                @display("p=600,400");
        }
        io: EthernetSocketIo {
            parameters:
                @display("p=400,600");
        }
    connections allowunconnected:
        source.out --> outbound.in if exists(source);
        outbound.out --> io.trafficIn;
        io.socketOut --> socketOut;

        socketIn --> io.socketIn;
        io.trafficOut --> inbound.in;
        inbound.out --> sink.in if exists(sink);
}
