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

package inet.routing.ospfv3;

import inet.networklayer.contract.INetworkLayer;
import inet.routing.contract.IOspfv3;


module OSPFv3 like IOspfv3
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string routingTableModule6;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.routingTableModule = default(absPath(routingTableModule));
        *.routingTableModule6 = default(absPath(routingTableModule6));
    @display("bgb=510,238;i=block/network2");
    gates:
//        input ipv6In @labels(OSPFv3Packet/up);
//        output ipv6Out @labels(OSPFv3Packet/down);

        input networkLayerIn @labels(Ipv6ControlInfo/up);
        output networkLayerOut @labels(Ipv6ControlInfo/down);
//        input networkLayerIn @labels(OSPFv3Packet/up);					bolo predtym, ked Hello pakety sa nedostaly z IPv6 vyssie
//        output networkLayerOut @labels(OSPFv3Packet/down);

    submodules:
        OSPFv3Splitter: OSPFv3Splitter {
            @display("p=254,176");
        }

    connections:
        networkLayerIn --> OSPFv3Splitter.ipIn;
        networkLayerOut <-- OSPFv3Splitter.ipOut;
}
