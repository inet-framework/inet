//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.consumer;

import inet.power.base.EpEnergyConsumerBase;

//
// This energy consumer model alternates between two modes called consumption
// and sleep mode. In consumption mode it consumes a randomly selected constant
// power for a random time interval. In sleep mode it doesn't consume energy for
// another random time interval.
//
// @see ~AlternatingEpEnergyGenerator
//
simple AlternatingEpEnergyConsumer extends EpEnergyConsumerBase
{
    parameters:
        string energySourceModule = default("^.energyStorage"); // module path of energy source
        volatile double powerConsumption @unit(W); // amount of power consumed in consumption mode
        volatile double consumptionInterval @unit(s); // time interval for consumption mode
        volatile double sleepInterval @unit(s); // time interval for sleep mode
        @class(AlternatingEpEnergyConsumer);
}

