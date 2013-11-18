// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
// @file pim.ned
// @date 21.1.2011
// @author: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief File implements compbound module pim
// @details Module with implementation of PIM-DM, also PIM-SM is included


package inet.networklayer.pim;

import inet.networklayer.pim.modes.PIMDM;
import inet.networklayer.pim.modes.PIMSM;
import inet.networklayer.pim.tables.PimInterfaceTable;
import inet.networklayer.pim.tables.PimNeighborTable;

module PIMRouting
{
    parameters:
        // pim-mode
        @display("i=block/network2;bgl=4;bgb=350,207");
        string configFile;
        string hostname;
        string deviceType;
        string deviceId;
    gates:
        input networkLayerIn;
        output networkLayerOut;

    submodules:
        pimSplitter: PIMSplitter {
            parameters:
                hostname = hostname;
                configFile = configFile;
                @display("p=69,95;q=queue;i=block/dispatch");
            gates:
                pimDMIn;
                pimSMIn;
                pimDMOut;
                pimSMOut;
                transportIn;
                transportOut;
        }

        pimDM: PIMDM {
            parameters:
                @display("p=165,142;q=queue;i=block/network2");
            gates:
                splitterIn;
                spiltterOut;
        }

        pimSM: PIMSM {
            parameters:
                @display("p=165,56;q=queue;i=block/network2");
            gates:
                splitterIn;
                spiltterOut;
        }

        PimInterfaceTable: PimInterfaceTable {
            parameters:
                @display("p=268,56");
        }

        PimNeighborTable: PimNeighborTable {
            parameters:
                @display("p=268,142");
        }

    connections:

        // pimDM
        pimSplitter.pimDMOut --> pimDM.splitterIn;
        pimSplitter.pimDMIn <-- pimDM.spiltterOut;

        // pimSM
        pimSplitter.pimSMOut --> pimSM.splitterIn;
        pimSplitter.pimSMIn <-- pimSM.spiltterOut;

        // splitter out
        pimSplitter.transportOut --> networkLayerOut;
        pimSplitter.transportIn <-- networkLayerIn;
}
