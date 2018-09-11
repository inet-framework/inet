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

package inet.networklayer.common;

import inet.common.MessageDispatcher;
import inet.networklayer.contract.IArp;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.contract.INetworkProtocol;

//
// This module provides a simple network layer.
//
module SimpleNetworkLayer like INetworkLayer
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @display("i=block/fork");

    gates:
        input ifIn @labels(INetworkHeader);
        output ifOut @labels(INetworkHeader);
        input transportIn @labels(ITransportPacket/down);
        output transportOut @labels(ITransportPacket/up);

    submodules:
        echo: EchoProtocol {
            parameters:
                @display("p=450,100");
        }
        up: MessageDispatcher {
            parameters:
                @display("p=300,200;b=480,5");
        }
        arp: <default("")> like IArp if typename != "" {
            parameters:
                @display("p=150,300;q=pendingQueue");
        }
        np: <> like INetworkProtocol {
            parameters:
                @display("p=300,300;q=queue");
        }
        lp: MessageDispatcher {
            parameters:
                @display("p=300,400;b=480,5");
        }

    connections allowunconnected:
        up.out++ --> { @display("m=n"); } --> transportOut;
        up.in++ <-- { @display("m=n"); } <-- transportIn;

        np.transportOut --> up.in++;
        np.transportIn <-- up.out++;

        up.out++ --> echo.ipIn;
        up.in++ <-- echo.ipOut;

        lp.out++ --> arp.ifIn if exists(arp);
        lp.in++ <-- arp.ifOut if exists(arp);

        lp.out++ --> np.queueIn;
        lp.in++ <-- np.queueOut;

        lp.in++ <-- { @display("m=s"); } <-- ifIn;
        lp.out++ --> { @display("m=s"); } --> ifOut;
}

