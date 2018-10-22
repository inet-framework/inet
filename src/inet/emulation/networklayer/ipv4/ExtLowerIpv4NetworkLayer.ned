//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

package inet.emulation.networklayer.ipv4;

import inet.common.MessageDispatcher;
import inet.networklayer.configurator.ipv4.Ipv4NodeConfigurator;
import inet.networklayer.contract.IArp;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.ipv4.IIgmp;
import inet.networklayer.ipv4.IIpv4;
import inet.networklayer.ipv4.Icmp;
import inet.networklayer.ipv4.Ipv4NatTable;
import inet.networklayer.ipv4.Ipv4RoutingTable;

module ExtLowerIpv4NetworkLayer like INetworkLayer
{
    parameters:
        @display("i=block/fork");

    gates:
        input ifIn @labels(INetworkHeader);
        output ifOut @labels(INetworkHeader);
        input transportIn @labels(Ipv4ControlInfo/down);
        output transportOut @labels(Ipv4ControlInfo/up);

    submodules:
        ip: <default("ExtLowerIpv4")> like IIpv4 {
            parameters:
                @display("p=50,50");
        }

    connections allowunconnected:
        transportIn --> ip.transportIn;
        transportOut <-- ip.transportOut;
}

