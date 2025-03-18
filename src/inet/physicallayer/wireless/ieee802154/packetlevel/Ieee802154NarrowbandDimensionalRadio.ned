//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee802154.packetlevel;

//
// Radio model for IEEE 802.15.4 narrowband PHY that uses dimensional
// representation for wireless transmissions. Models signal power as a function
// of time and frequency rather than as a single scalar value, providing more
// accurate simulation of signal propagation and interference. Designed to work
// with Ieee802154NarrowbandDimensionalRadioMedium for consistent signal
// representation, this model is suitable for detailed physical layer studies of
// IEEE 802.15.4 networks where frequency-selective effects are important.
//
module Ieee802154NarrowbandDimensionalRadio extends Ieee802154NarrowbandRadio
{
    parameters:
        signalAnalogRepresentation = default("dimensional");
}

