//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.mobility.contract.IMobility;

//
// A base station site of `numSectors` sectors, spread evenly in azimuth. Each sector has its
// own panels, radios and MAC (see ~GnbSector); the sectors share the position of the site.
//
module Gnb
{
    parameters:
        int numSectors = default(3); // Number of sectors of the site
        int numCarriers = default(1); // Number of component carriers of each sector
        int numBands = default(1); // Number of bands of each sector, one panel antenna each
        @display("i=device/antennatower");
    submodules:
        mobility: <default("StationaryMobility")> like IMobility {
            @display("p=100,100");
        }
        sector[numSectors]: GnbSector {
            parameters:
                numCarriers = default(parent.numCarriers);
                numBands = default(parent.numBands);
                boresight = default(360deg * index / parent.numSectors);
                @display("p=250,100,row,150");
        }
}
