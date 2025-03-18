//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee80211.packetlevel;

//
// Radio model for IEEE 802.11 networks that uses a simplified unit disk-based
// representation for wireless transmissions. Determines reception success based
// primarily on distance between transmitter and receiver using communication,
// interference, and detection range thresholds. Provides a computationally
// efficient alternative to more detailed radio models when precise physical
// layer behavior is not critical, making it suitable for large-scale network
// simulations focusing on higher-layer protocols.
//
module Ieee80211UnitDiskRadio extends Ieee80211Radio
{
    parameters:
        signalAnalogRepresentation = default("unitDisk");
}
