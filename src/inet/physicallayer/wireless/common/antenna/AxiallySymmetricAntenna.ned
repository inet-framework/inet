//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.antenna;

import inet.physicallayer.wireless.common.base.packetlevel.AntennaBase;

// This antenna model computes the antenna gain from the direction using linear
// interpolation. The gain parameter contains a sequence of angle [degree] and
// gain [dB] pairs. The first pair must be at 0 [degree].
//
module AxiallySymmetricAntenna extends AntennaBase
{
    parameters:
        double baseGain @unit(dB) = default(0dB);
        string gains = default("0 0"); // sequence of angle [degree] in the range (0, 180) and gain [dB] pairs
        string axisOfSymmetry = default("x"); // allows matching the antenna radiation pattern (IAntennaGain) to the visual representation, antenna orientation is determined by the antenna's mobility model
        @class(AxiallySymmetricAntenna);
}

