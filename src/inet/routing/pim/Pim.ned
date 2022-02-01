//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.routingTableModule = default(absPath(this.routingTableModule));
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

simple PimCrcInsertionHook
{
}

