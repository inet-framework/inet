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

import inet.common.MessageDispatcher;
import inet.networklayer.arp.generic.GenericArp;
import inet.networklayer.common.EchoProtocol;
import inet.networklayer.contract.INetworkLayer;


//
// This module provides the generic network layer.
//
module GenericNetworkLayer like INetworkLayer
{
    parameters:
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        string interfaceTableModule;
        *.forwarding = forwarding;
        *.multicastForwarding = multicastForwarding;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.routingTableModule = default(absPath(".routingTable"));
        *.arpModule = default(absPath(".arp"));
        @display("i=block/fork");
    gates:
        input ifIn @labels(INetworkHeader);
        output ifOut @labels(INetworkHeader);
        input transportIn @labels(Ipv4ControlInfo/down);
        output transportOut @labels(Ipv4ControlInfo/up);

    submodules:
        routingTable: GenericRoutingTable {
            parameters:
                @display("p=100,100;is=s");
        }
        gnp: GenericNetworkProtocol {
            parameters:
                @display("p=350,300;q=queue");
        }
        echo: EchoProtocol {
            parameters:
                @display("p=500,100");
        }
        arp: GenericArp {
            parameters:
                @display("p=200,300;q=pendingQueue");
        }
        dp: MessageDispatcher {
            parameters:
                @display("p=350,200;b=380,5");
        }

    connections allowunconnected:
        dp.out++ --> { @display("m=n"); } --> transportOut;
        dp.in++ <-- { @display("m=n"); } <-- transportIn;

        gnp.transportOut --> dp.in++;
        gnp.transportIn <-- dp.out++;

        dp.out++ --> echo.ipIn;
        dp.in++ <-- echo.ipOut;

        gnp.arpOut --> arp.netwIn;
        gnp.arpIn <-- arp.netwOut;

        ifIn --> { @display("m=s"); } --> gnp.queueIn;
        gnp.queueOut --> { @display("m=s"); } --> ifOut;
}
