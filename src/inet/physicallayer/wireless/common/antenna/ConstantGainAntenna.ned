//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.antenna;

import inet.physicallayer.wireless.common.base.packetlevel.AntennaBase;

//
// This antenna model describes an antenna that has an antenna gain indepent of
// the transmission or reception direction.
//
module ConstantGainAntenna extends AntennaBase
{
    parameters:
        double gain @unit(dB); // antenna gain indepent of transmission or reception direction
        @class(ConstantGainAntenna);
}

