//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.generator;

import inet.power.base.EpEnergyGeneratorBase;

//
// This energy generator model alternates between two modes called generation
// and sleep mode. In generation mode it generates a randomly selected constant
// power for a random time interval. In sleep mode it doesn't generate energy
// for another random time interval.
//
// @see ~AlternatingEpEnergyConsumer
//
simple AlternatingEpEnergyGenerator extends EpEnergyGeneratorBase
{
    parameters:
        string energySinkModule = default("^.energyStorage"); // module path of energy sink
        volatile double powerGeneration @unit(W); // amount of power generated in generation mode
        volatile double generationInterval @unit(s); // time interval for generation mode
        volatile double sleepInterval @unit(s); // time interval for sleep mode
        @class(AlternatingEpEnergyGenerator);
}

