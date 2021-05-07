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

package inet.networklayer.wiseroute;

import inet.common.MessageDispatcher;
import inet.networklayer.arp.ipv4.GlobalArp;
import inet.networklayer.common.EchoProtocol;
import inet.networklayer.contract.INetworkLayer;

//
// This module provides a simple network layer.
//
module WiseRouteNetworkLayer like INetworkLayer
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        @display("i=block/fork");

    gates:
        input ifIn @labels(INetworkHeader);
        output ifOut @labels(INetworkHeader);
        input transportIn @labels(ITransportPacket/down);
        output transportOut @labels(ITransportPacket/up);

    submodules:
        arp: GlobalArp {
            parameters:
                @display("p=100,300");
        }
        np: WiseRoute {
            parameters:
                arpModule = "^.arp";
                @display("p=250,300;q=queue");
        }
        echo: EchoProtocol {
            parameters:
                @display("p=400,100");
        }
        dp: MessageDispatcher {
            parameters:
                @display("p=250,200;b=400,5");
        }

    connections allowunconnected:
        dp.out++ --> { @display("m=n"); } --> transportOut;
        dp.in++ <-- { @display("m=n"); } <-- transportIn;

        np.transportOut --> dp.in++;
        np.transportIn <-- dp.out++;

        dp.out++ --> echo.ipIn;
        dp.in++ <-- echo.ipOut;

        ifIn --> { @display("m=s"); } --> np.queueIn;
        np.queueOut --> { @display("m=s"); } --> ifOut;
}

