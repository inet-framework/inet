//
// Copyright (C) 2012 Opensim Ltd.
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

package inet.networklayer.generic;

import inet.networklayer.arp.generic.GenericARP;
import inet.networklayer.common.EchoProtocol;
import inet.networklayer.contract.INetworkLayer;


//
// This module provides the generic network layer.
//
module GenericNetworkLayer like INetworkLayer
{
    parameters:
        string interfaceTableModule;
        string routingTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.routingTableModule = default(absPath(routingTableModule));
        *.arpModule = default(absPath(".arp"));
        @display("i=block/fork");
    gates:
        input ifIn[] @labels(INetworkDatagram);
        output ifOut[] @labels(INetworkDatagram);
        input transportIn[] @labels(IPv4ControlInfo/down);
        output transportOut[] @labels(IPv4ControlInfo/up);
        input pingIn[] @labels(PingPayload/down);
        output pingOut[] @labels(PingPayload/up);

    submodules:
        gnp: GenericNetworkProtocol {
            parameters:
                @display("p=85,95;q=queue");
            gates:
                queueIn[sizeof(ifIn)];
                queueOut[sizeof(ifOut)];
        }
        echo: EchoProtocol {
            parameters:
                @display("p=160,63");
        }
        arp: GenericARP {
            parameters:
                @display("p=202,142;q=pendingQueue");
        }
    connections allowunconnected:
        for i=0..sizeof(transportIn)-1 {
            transportIn[i] --> { @display("m=n"); } --> gnp.transportIn++;
        }
        for i=0..sizeof(transportOut)-1 {
            gnp.transportOut++ --> { @display("m=n"); } --> transportOut[i];
        }

        gnp.transportOut++ --> echo.localIn;
        gnp.transportIn++ <-- echo.sendOut;

        for i=0..sizeof(pingOut)-1 {
            echo.pingOut++ --> { @display("m=n"); } --> pingOut[i];
        }
        for i=0..sizeof(pingIn)-1 {
            echo.pingIn++ <-- { @display("m=n"); } <-- pingIn[i];
        }

        gnp.arpOut --> arp.netwIn;
        gnp.arpIn <-- arp.netwOut;

        for i=0..sizeof(ifIn)-1 {
            ifIn[i] --> { @display("m=s"); } --> gnp.queueIn[i];
        }
        for i=0..sizeof(ifOut)-1 {
            gnp.queueOut[i] --> { @display("m=s"); } --> ifOut[i];
        }
}
