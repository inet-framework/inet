//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.base;

import inet.power.contract.ICcEnergyConsumer;

//
// This is an abstract base module for current based energy consumer models.
// It defines shared signals and statistics.
//
// @see ~CcEnergySourceBase, ~CcEnergyGeneratorBase, ~CcEnergySinkBase, ~CcEnergyStorageBase
//
simple CcEnergyConsumerBase like ICcEnergyConsumer
{
    parameters:
        @display("i=block/plug");
        @signal[currentConsumptionChanged](type=double);
        @statistic[currentConsumption](title="Current consumption"; source=currentConsumptionChanged; record=vector; interpolationmode=sample-hold; unit=A);
}

