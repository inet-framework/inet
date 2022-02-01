//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.physicallayer.wireless.common.radio.packetlevel.Radio;

//
// This module servces as a base module for narrowband radio models.
//
module NarrowbandRadioBase extends Radio
{
    parameters:
        double centerFrequency @unit(Hz);     // center frequency of the band where the radio transmits and receives signals on the medium
        double bandwidth @unit(Hz);            // bandwidth of the band where the radio transmits and receives signals on the medium
        *.centerFrequency = default(this.centerFrequency); // passed down to transmitter and receiver by default
        *.bandwidth = default(this.bandwidth);               // passed down to transmitter and receiver by default
}

