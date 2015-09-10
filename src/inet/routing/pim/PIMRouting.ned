//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

package inet.routing.pim;

import inet.routing.contract.IPIMRouting;
import inet.routing.pim.modes.PIMDM;
import inet.routing.pim.modes.PIMSM;
import inet.routing.pim.tables.PIMInterfaceTable;
import inet.routing.pim.tables.PIMNeighborTable;

module PIMRouting like IPIMRouting
{
    parameters:
        @display("i=block/network2;bgl=4;bgb=350,207");
        string interfaceTableModule;
        string routingTableModule;
        *.interfaceTableModule = default(interfaceTableModule);
        *.routingTableModule = default(routingTableModule);
        *.pimInterfaceTableModule = default("^.pimInterfaceTable");
        *.pimNeighborTableModule = default("^.pimNeighborTable");

    gates:
        input networkLayerIn;
        output networkLayerOut;

    submodules:
        pimSplitter: PIMSplitter {
            parameters:
                @display("p=69,95");
        }

        pimDM: PIMDM {
            parameters:
                @display("p=165,142");
        }

        pimSM: PIMSM {
            parameters:
                @display("p=165,56");
        }

        pimInterfaceTable: PIMInterfaceTable {
            parameters:
                @display("p=268,56");
        }

        pimNeighborTable: PIMNeighborTable {
            parameters:
                @display("p=268,142");
        }

    connections:

        // pimDM
        pimSplitter.pimDMOut --> pimDM.ipIn;
        pimSplitter.pimDMIn <-- pimDM.ipOut;

        // pimSM
        pimSplitter.pimSMOut --> pimSM.ipIn;
        pimSplitter.pimSMIn <-- pimSM.ipOut;

        // splitter out
        pimSplitter.ipOut --> networkLayerOut;
        pimSplitter.ipIn <-- networkLayerIn;
}
