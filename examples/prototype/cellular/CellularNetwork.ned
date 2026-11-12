//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.prototype.cellular;

import inet.prototype.cellular.CellularRadioMedium;
import inet.prototype.cellular.Gnb;
import inet.prototype.cellular.IdealControlChannel;
import inet.prototype.cellular.Ue;

//
// Base station sites on a hexagonal grid (rows of `numColumns` sites, every second row shifted
// by half the inter-site distance), and UEs dropped uniformly over the area of the grid. All
// nodes have the same component carriers.
//
network CellularNetwork
{
    parameters:
        int numSites = default(1); // Number of base station sites
        int numUesPerSector = default(10); // Number of UEs per sector, on average
        int numUes = default(numSites * 3 * numUesPerSector); // Number of UEs
        int numCarriers = default(1); // Number of component carriers of every node
        int numBands = default(1); // Number of bands of every node, one antenna each
        double interSiteDistance @unit(m) = default(500m); // Distance of neighboring sites
        int numColumns = default(int(ceil(sqrt(numSites)))); // Number of sites in a row of the grid
        double width @unit(m) = default(interSiteDistance * (numColumns + (numSites > numColumns ? 0.5 : 0))); // Width of the area
        double height @unit(m) = default(interSiteDistance * 0.866 * ceil(numSites / numColumns)); // Height of the area
        @display("bgb=$width,$height");
    submodules:
        radioMedium: CellularRadioMedium {
            @display("p=50,50");
        }
        controlChannel: IdealControlChannel {
            @display("p=50,120");
        }
        gnb[numSites]: Gnb {
            parameters:
                numCarriers = default(parent.numCarriers);
                numBands = default(parent.numBands);
                mobility.initFromDisplayString = false;
                mobility.initialX = parent.interSiteDistance * (0.5 + index % parent.numColumns + 0.5 * (int(floor(index / parent.numColumns)) % 2));
                mobility.initialY = parent.interSiteDistance * 0.866 * (0.5 + floor(index / parent.numColumns));
                mobility.initialZ = 25m;
        }
        ue[numUes]: Ue {
            parameters:
                numCarriers = default(parent.numCarriers);
                numBands = default(parent.numBands);
                mobility.initFromDisplayString = false;
                mobility.initialX = uniform(0m, parent.width);
                mobility.initialY = uniform(0m, parent.height);
                mobility.initialZ = 1.5m;
        }
}
