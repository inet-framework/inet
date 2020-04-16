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

import inet.routing.contract.IPim;
import inet.routing.pim.modes.PimDm;
import inet.routing.pim.modes.PimSm;
import inet.routing.pim.tables.PimInterfaceTable;
import inet.routing.pim.tables.PimNeighborTable;

module Pim like IPim
{
    parameters:
        @class(Pim);
        @display("i=block/network2");
        string interfaceTableModule;
        string routingTableModule;
        string crcMode @enum("declared", "computed") = default("declared");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.routingTableModule = default(absPath(routingTableModule));
        *.pimInterfaceTableModule = default("^.pimInterfaceTable");
        *.pimNeighborTableModule = default("^.pimNeighborTable");

    gates:
        input networkLayerIn;
        output networkLayerOut;

    submodules:
        pimSplitter: PimSplitter {
            parameters:
                @display("p=250,150");
        }

        pimDM: PimDm {
            parameters:
                @display("p=400,200");
        }

        pimSM: PimSm {
            parameters:
                @display("p=400,100");
        }

        pimInterfaceTable: PimInterfaceTable {
            parameters:
                @display("p=100,100;is=s");
        }

        pimNeighborTable: PimNeighborTable {
            parameters:
                @display("p=100,200;is=s");
        }

    connections:

        // pimDM
        pimSplitter.pimDMOut --> pimDM.ipIn;
        pimSplitter.pimDMIn <-- pimDM.ipOut;

        // pimSM
        pimSplitter.pimSMOut --> pimSM.ipIn;
        pimSplitter.pimSMIn <-- pimSM.ipOut;

        // splitter out
        pimSplitter.ipOut --> { @display("m=s"); } --> networkLayerOut;
        pimSplitter.ipIn <-- { @display("m=s"); } <-- networkLayerIn;
}

