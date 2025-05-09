//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.medium;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IMediumLimitCache;

//
// Caches various limits and constraints of the radio medium to optimize
// performance. Maintains information about maximum transmission power, minimum
// interference and reception power, maximum antenna gain, communication and
// interference ranges, and other parameters. These limits can be specified
// directly or computed dynamically based on the radios in the network.
//
module MediumLimitCache extends Module like IMediumLimitCache
{
    parameters:
        double centerFrequency @unit(Hz) = default(nan Hz);       // TODO rename to maxRangeCenterFrequency?
        double maxSpeed @unit(mps) = default(nan mps);             // Maximum speed on the medium, NaN means medium computes using mobility models
        double maxTransmissionPower @unit(W) = default(nan W);     // TODO @unit(W) + dBm/dBW <--> W
        double minInterferencePower @unit(dBm) = default(nan dBm); // TODO @unit(W) + dBm/dBW <--> W
        double minReceptionPower @unit(dBm) = default(nan dBm);    // TODO @unit(W) + dBm/dBW <--> W
        double maxAntennaGain @unit(dB) = default(0dB);           // Maximum antenna gain on the medium, NaN means medium computes using antenna models
        double minInterferenceTime @unit(s) = default(1ps);       // Minimum time interval to consider two overlapping signals interfering
        double maxTransmissionDuration @unit(s) = default(10ms);  // Maximum duration of a transmission on the medium
        double maxCommunicationRange @unit(m) = default(nan m);    // Maximum communication range on the medium, NaN means medium computes using transmitter and receiver models
        double maxInterferenceRange @unit(m) = default(nan m);     // Maximum interference range on the medium, NaN means medium computes using transmitter and receiver models
        @display("i=block/table2");
        @class(MediumLimitCache);
}

