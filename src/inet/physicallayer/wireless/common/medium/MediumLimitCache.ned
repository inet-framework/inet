//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.medium;

import inet.physicallayer.wireless.common.contract.packetlevel.IMediumLimitCache;

module MediumLimitCache like IMediumLimitCache
{
    parameters:
        double centerFrequency @unit(Hz) = default(nan Hz);       // TODO rename to maxRangeCenterFrequency?
        double maxSpeed @unit(mps) = default(nan mps);             // maxium speed on the medium, NaN means medium computes using mobility models
        double maxTransmissionPower @unit(W) = default(nan W);     // TODO @unit(W) + dBm/dBW <--> W
        double minInterferencePower @unit(dBm) = default(nan dBm); // TODO @unit(W) + dBm/dBW <--> W
        double minReceptionPower @unit(dBm) = default(nan dBm);    // TODO @unit(W) + dBm/dBW <--> W
        double maxAntennaGain @unit(dB) = default(0dB);           // maximum antenna gain on the medium, NaN means medium computes using antenna models
        double minInterferenceTime @unit(s) = default(1ps);       // minimum time interval to consider two overlapping signals interfering
        double maxTransmissionDuration @unit(s) = default(10ms);  // maximum duration of a transmission on the medium
        double maxCommunicationRange @unit(m) = default(nan m);    // maximum communication range on the medium, NaN means medium computes using transmitter and receiver models
        double maxInterferenceRange @unit(m) = default(nan m);     // maximum interference range on the medium, NaN means medium computes using transmitter and receiver models
        @display("i=block/table2");
        @class(MediumLimitCache);
}

