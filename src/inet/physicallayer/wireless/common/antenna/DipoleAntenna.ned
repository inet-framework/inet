//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.antenna;

import inet.physicallayer.wireless.common.base.packetlevel.AntennaBase;

//
// This antenna model describes the well-known dipole antenna or doublet. It
// consists of two identical conductive elements, which are bilaterally
// symmetrical.
//
module DipoleAntenna extends AntennaBase
{
    parameters:
        double length @unit(m); // the length of the dipole antenna
        string wireAxis = default("z"); // allows matching the antenna radiation pattern (IAntennaGain) to the visual representation, antenna orientation is determined by the antenna's mobility model
                                        // the default value is Z axis, so that antenna mobility models are not required when antennas are approximately positioned in the same plane
        @class(DipoleAntenna);
}

