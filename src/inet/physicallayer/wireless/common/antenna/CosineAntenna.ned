//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.antenna;

import inet.physicallayer.wireless.common.base.packetlevel.AntennaBase;

//
// Models a hypotetical antenna with a cosine-based radiation pattern.
// This antenna model is commonly used in the real world to approximate
// various directional antennas.
//
// Gain is computed from the angle as follows:
// <pre>
//   exponent = -3 / (20 * log10(cos(beamWidth) / 4))
//   gain = maxGain * abs(cos(angle / 2)) ^ exponent
// </pre>
// This module represents the cosine antenna model found in
//
// Li Chunjian, "Efficient Antenna Patterns for Three-Sector WCDMA Systems",
// eq. B.1 and B.2
//
module CosineAntenna extends AntennaBase
{
    parameters:
        double maxGain @unit(dB);    // gain at the antenna boresight
        double beamWidth @unit(deg); // 3dB beam width
        @class(CosineAntenna);
}

