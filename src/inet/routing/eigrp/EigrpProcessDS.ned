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
import inet.routing.eigrp.EigrpSplitter;
import inet.routing.contract.IEigrp;


import inet.routing.eigrp.tables.EigrpInterfaceTable;
import inet.routing.eigrp.tables.EigrpIpv4NeighborTable;
import inet.routing.eigrp.tables.EigrpIpv4TopologyTable;

import inet.routing.eigrp.tables.EigrpIpv6NeighborTable;
import inet.routing.eigrp.tables.EigrpIpv6TopologyTable;



module EigrpProcessDS like IEigrp
{
    parameters:
        @display("bgb=494,229,white;i=block/network2");
        bool enableIPv4 = default(true);
        bool enableIPv6 = default(true);
        xml configData = default(xmldoc("config.xml"));
		string interfaceTableModule;
        string routingTableModule;
        string routingTableModule6;
    gates:
        input ipIn @labels(Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down,Ipv6ControlInfo/down);
    submodules:
        // vlevo nazev, vpravo typ
        eigrpSplitter: EigrpSplitter {
            @display("p=254,176");
        }
        eigrpIpv4Pdm: EigrpIpv4Pdm if enableIPv4 {
            @display("p=64,66;i=block/circle");
            configData = configData;
            routingTableModule = routingTableModule;
            interfaceTableModule = interfaceTableModule;
        }
        eigrpInterfaceTable: EigrpInterfaceTable if enableIPv4 {
            @display("p=185,180");
        }
        eigrpIpv4NeighborTable: EigrpIpv4NeighborTable if enableIPv4 {
            @display("p=185,45");
        }
        eigrpIpv4TopologyTable: EigrpIpv4TopologyTable if enableIPv4 {
            @display("p=185,112");
        }
        eigrpRtp: EigrpRtp if enableIPv4 {
            @display("p=64,157;i=block/dispatch");
        }

        eigrpInterfaceTable6: EigrpInterfaceTable if enableIPv6 {
            @display("p=320,180");
        }
        eigrpIpv6Pdm: EigrpIpv6Pdm if enableIPv6 {
            @display("p=433,66;i=block/circle");
            configData = configData;
            interfaceTableModule = interfaceTableModule;
            routingTableModule = routingTableModule6;
        }
        eigrpIpv6NeighborTable: EigrpIpv6NeighborTable if enableIPv6 {
            @display("p=320,45");
        }
        eigrpIpv6TopologyTable: EigrpIpv6TopologyTable if enableIPv6 {
            @display("p=320,112");
        }
        eigrpRtp6: EigrpRtp6 if enableIPv6 {
            @display("p=433,157;i=block/dispatch");
        }
    connections allowunconnected:
        ipIn --> eigrpSplitter.ipIn;
        ipOut <-- eigrpSplitter.ipOut;
        
        eigrpSplitter.splitterOut --> eigrpIpv4Pdm.splitterIn if enableIPv4;
        eigrpSplitter.splitterIn <-- eigrpIpv4Pdm.splitterOut if enableIPv4;

        eigrpIpv4Pdm.rtpOut --> eigrpRtp.pdmIn if enableIPv4;
        eigrpIpv4Pdm.rtpIn <-- eigrpRtp.pdmOut if enableIPv4;
        
        eigrpSplitter.splitter6Out --> eigrpIpv6Pdm.splitterIn if enableIPv6;
        eigrpSplitter.splitter6In <-- eigrpIpv6Pdm.splitterOut if enableIPv6;

        eigrpIpv6Pdm.rtpOut --> eigrpRtp6.pdmIn if enableIPv6;
        eigrpIpv6Pdm.rtpIn <-- eigrpRtp6.pdmOut if enableIPv6;

}
