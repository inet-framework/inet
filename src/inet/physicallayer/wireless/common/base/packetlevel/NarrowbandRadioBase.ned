//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.physicallayer.wireless.common.radio.packetlevel.Radio;

//
// This module serves as a base module for narrowband radio models.
//
module NarrowbandRadioBase extends Radio
{
    parameters:
        double centerFrequency @unit(Hz);     // Center frequency of the band where the radio transmits and receives signals on the medium
        double bandwidth @unit(Hz);            // Bandwidth of the band where the radio transmits and receives signals on the medium
        *.centerFrequency = default(this.centerFrequency); // Passed down to transmitter and receiver by default
        *.bandwidth = default(this.bandwidth);               // Passed down to transmitter and receiver by default
}

