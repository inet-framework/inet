//
// Copyright (C) 2005 Andras Babos
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


package inet.node.ospfv3;

import inet.node.inet.Router;
import inet.routing.ospfv3.OSPFv3;
import inet.routing.contract.IOspfv3;


//
// An OSPFv2 router.
//
module Ospfv3Router extends Router
{
	parameters:
      //  hasOspfv3 = true;
        hasIpv6 = true;
        *.routingTableModule6 = "^.ipv6.routingTable";
//        bool hasOSPFv3 = default(true);
		hasOSPFv3 = true;
		xml configData = default(xmldoc("config.xml"));
        
   	submodules:
//       ospfv3: OSPFv3Routing if hasOSPFv3 {
//            parameters:
//                @display("p=689,331");
//        }
//         ospfv3: <"Ospfv3"> like IOspfv3 if hasOSPFv3 {
//            parameters:
//                @display("p=825,226");
//         }
	connections allowunconnected:
//		ospfv3.networkLayerOut --> tn.in++ if hasOspf;
//        ospfv3.networkLayerIn <-- tn.out++ if hasOspf;
}

