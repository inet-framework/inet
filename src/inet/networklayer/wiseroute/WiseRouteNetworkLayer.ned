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

import inet.networklayer.arp.generic.GenericARP;
import inet.networklayer.common.EchoProtocol;
import inet.networklayer.contract.INetworkLayer;


//
// This module provides a simple network layer.
//
module WiseRouteNetworkLayer like INetworkLayer
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @display("i=block/fork");

    gates:
        input ifIn[] @labels(INetworkDatagram);
        output ifOut[] @labels(INetworkDatagram);
        input transportIn[] @labels(ITransportPacket/down);
        output transportOut[] @labels(ITransportPacket/up);
        input pingIn[] @labels(PingPayload/down);
        output pingOut[] @labels(PingPayload/up);

    submodules:
        arp: GenericARP {
            @display("p=50,50");
        }
        np: WiseRoute {
            parameters:
                @display("p=85,95;q=queue");
            gates:
                lowerLayerIn[sizeof(ifIn)];
                lowerLayerOut[sizeof(ifOut)];
        }
        echo: EchoProtocol {
            parameters:
                @display("p=160,63");
        }

    connections allowunconnected:
        for i=0..sizeof(transportIn)-1 {
            transportIn[i] --> { @display("m=n"); } --> np.upperLayerIn++;
        }
        for i=0..sizeof(transportOut)-1 {
            np.upperLayerOut++ --> { @display("m=n"); } --> transportOut[i];
        }

        np.upperLayerOut++ --> echo.localIn;
        np.upperLayerIn++ <-- echo.sendOut;

        for i=0..sizeof(pingOut)-1 {
            echo.pingOut++ --> { @display("m=n"); } --> pingOut[i];
        }
        for i=0..sizeof(pingIn)-1 {
            echo.pingIn++ <-- { @display("m=n"); } <-- pingIn[i];
        }

        for i=0..sizeof(ifIn)-1 {
            ifIn[i] --> { @display("m=s"); } --> np.lowerLayerIn[i];
        }
        for i=0..sizeof(ifOut)-1 {
            np.lowerLayerOut[i] --> { @display("m=s"); } --> ifOut[i];
        }
}
