//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.common.Module;
import inet.mobility.contract.IMobility;
import inet.physicallayer.wireless.common.contract.packetlevel.IAntenna;

//
// Serves as the base module for antenna models.
//
module AntennaBase extends Module like IAntenna
{
    parameters:
        string mobilityModule; // The path to the mobility module relative to this module, only used if no mobility submodule is present
        int numAntennas = default(1); // The number of antennas in the antenna array
        mobility.subjectModule = default(""); // Disable subject module of mobility submodule by default
        @display("i=device/antennatower");
    submodules:
        mobility: <default("")> like IMobility if typename != "" {
            parameters:
                @display("p=100,100;is=s");
        }
}

