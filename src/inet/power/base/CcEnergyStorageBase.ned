//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.base;

import inet.power.contract.ICcEnergyStorage;

//
// This is an abstract base module for current based energy storage models.
// It defines shared signals and statistics.
//
// @see ~CcEnergyConsumerBase, ~CcEnergySourceBase, ~CcEnergyGeneratorBase, ~CcEnergySinkBase
//
simple CcEnergyStorageBase like ICcEnergyStorage
{
    parameters:
        @display("i=block/plug");
        @signal[currentConsumptionChanged];
        @signal[currentGenerationChanged];
        @signal[residualChargeCapacityChanged];
        @statistic[currentConsumption](title="Current consumption"; source=currentConsumptionChanged; record=vector; interpolationmode=sample-hold; unit=A);
        @statistic[currentGeneration](title="Current generation"; source=currentGenerationChanged; record=vector; interpolationmode=sample-hold; unit=A);
        @statistic[residualChargeCapacity](title="Residual charge capacity"; source=residualChargeCapacityChanged; record=vector; interpolationmode=linear; unit=C);
}

