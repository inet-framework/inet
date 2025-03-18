//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee802154.packetlevel;

//
// Radio model for IEEE 802.15.4 narrowband PHY that uses scalar representation
// for wireless transmissions. Represents signal power as a single scalar value
// rather than as a function of time and frequency. Provides a balance between
// accuracy and computational efficiency, suitable for network-level simulations
// of IEEE 802.15.4 networks where detailed physical layer behavior is not
// critical.
//
module Ieee802154NarrowbandScalarRadio extends Ieee802154NarrowbandRadio
{
    parameters:
        signalAnalogRepresentation = default("scalar");
}

