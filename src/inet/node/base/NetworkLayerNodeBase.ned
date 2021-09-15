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

package inet.node.base;

import inet.common.MessageDispatcher;
import inet.networklayer.contract.INetworkLayer;

module NetworkLayerNodeBase extends LinkLayerNodeBase
{
    parameters:
        bool hasIpv4 = default(true);
        bool hasIpv6 = default(false);
        bool hasGn = default(false);
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        *.forwarding = this.forwarding;
        *.multicastForwarding = this.multicastForwarding;
        @figure[networkLayer](type=rectangle; pos=250,308; size=1000,134; fillColor=#00ff00; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[networkLayer.title](type=text; pos=1245,313; anchor=ne; text="network layer");
    submodules:
        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer if hasIpv4 {
            @display("p=375,375;q=queue");
        }
        ipv6: <default("Ipv6NetworkLayer")> like INetworkLayer if hasIpv6 {
            @display("p=525,375;q=queue");
        }
        generic: <default("")> like INetworkLayer if hasGn {
            @display("p=675,375;q=queue");
        }
        nl: MessageDispatcher {
            @display("p=750,450;b=1000,5,,,,1");
        }
    connections allowunconnected:
        ipv4.ifIn <-- nl.out++ if hasIpv4;
        ipv4.ifOut --> nl.in++ if hasIpv4;

        ipv6.ifIn <-- nl.out++ if hasIpv6;
        ipv6.ifOut --> nl.in++ if hasIpv6;

        generic.ifIn <-- nl.out++ if exists(generic);
        generic.ifOut --> nl.in++ if exists(generic);

        cb.out++ --> nl.in++;
        cb.in++ <-- nl.out++;

        llc.upperLayerOut --> nl.in++ if exists(llc);
        llc.upperLayerIn <-- nl.out++ if exists(llc);
}

