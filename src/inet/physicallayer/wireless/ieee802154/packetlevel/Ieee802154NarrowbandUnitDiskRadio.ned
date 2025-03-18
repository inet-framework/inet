//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee802154.packetlevel;

//
// Radio model for IEEE 802.15.4 narrowband PHY that uses unit disk representation
// for wireless transmissions. Simplifies radio propagation to a binary (on/off)
// reception model based on distance thresholds, ignoring fading and interference
// effects. Provides significantly improved simulation performance at the cost of
// physical layer realism, making it suitable for large-scale network simulations
// where computational efficiency is more important than detailed radio modeling.
//
module Ieee802154NarrowbandUnitDiskRadio extends Ieee802154NarrowbandRadio
{
    parameters:
        signalAnalogRepresentation = default("unitDisk");
}
