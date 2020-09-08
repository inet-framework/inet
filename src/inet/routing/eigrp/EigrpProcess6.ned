//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
// @author Jan Zavrel (honza.zavrel96@gmail.com)
// @author Jan Bloudicek (jbloudicek@gmail.com)
// @author Vit Rek (rek@kn.vutbr.cz)
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3


package inet.routing.eigrp;

import inet.routing.eigrp.pdms.EigrpIpv4Pdm;
import inet.routing.eigrp.pdms.EigrpIpv6Pdm;

import inet.routing.eigrp.tables.EigrpInterfaceTable;
import inet.routing.eigrp.tables.EigrpIpv4NeighborTable;
import inet.routing.eigrp.tables.EigrpIpv4TopologyTable;

import inet.routing.eigrp.tables.EigrpIpv6NeighborTable;
import inet.routing.eigrp.tables.EigrpIpv6TopologyTable;


module EigrpProcess6
{
    parameters:

        @display("bgb=494,229,white");
    gates:
        //input splitterIn;
        input splitter6In;
        //output splitterOut;
        output splitter6Out;

    submodules:

		eigrpInterfaceTable6: EigrpInterfaceTable {
            @display("p=320,180");
        }
        eigrpIpv6Pdm: EigrpIpv6Pdm {
            @display("p=433,66;i=block/circle");
        }
        eigrpIpv6NeighborTable: EigrpIpv6NeighborTable {
            @display("p=320,45");
        }
        eigrpIpv6TopologyTable: EigrpIpv6TopologyTable {
            @display("p=320,112");
        }
        eigrpRtp6: EigrpRtp6 {
            @display("p=433,157;i=block/dispatch");
        }
    connections:

        splitter6In --> eigrpIpv6Pdm.splitterIn;
        splitter6Out <-- eigrpIpv6Pdm.splitterOut;

        eigrpIpv6Pdm.rtpOut --> eigrpRtp6.pdmIn;
        eigrpIpv6Pdm.rtpIn <-- eigrpRtp6.pdmOut;
}
