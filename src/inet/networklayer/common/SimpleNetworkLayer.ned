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
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.contract.INetworkProtocol;


//
// This module provides a simple network layer.
//
module SimpleNetworkLayer like INetworkLayer
{
    parameters:
        string interfaceTableModule;
        string networkProtocolType;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @display("i=block/fork");

    gates:
        input ifIn @labels(INetworkHeader);
        output ifOut @labels(INetworkHeader);
        input transportIn @labels(ITransportPacket/down);
        output transportOut @labels(ITransportPacket/up);

    submodules:
        np: <networkProtocolType> like INetworkProtocol {
            parameters:
                @display("p=200,300;q=queue");
        }
        echo: EchoProtocol {
            parameters:
                @display("p=300,100");
        }
        dp: MessageDispatcher {
            parameters:
                @display("p=200,200;b=280,5");
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
