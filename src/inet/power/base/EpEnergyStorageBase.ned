//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.base;

import inet.power.contract.IEpEnergyStorage;

//
// This is an abstract base module for power based energy storage models.
// It defines shared signals and statistics.
//
// @see ~EpEnergyConsumerBase, ~EpEnergySourceBase, ~EpEnergyGeneratorBase, ~EpEnergySinkBase
//
simple EpEnergyStorageBase like IEpEnergyStorage
{
    parameters:
        @display("i=block/plug");
        @class(EpEnergyStorageBase);
        @signal[powerConsumptionChanged];
        @signal[powerGenerationChanged];
        @signal[residualEnergyCapacityChanged];
        @statistic[powerConsumption](title="Power consumption"; source=powerConsumptionChanged; record=vector; interpolationmode=sample-hold; unit=W);
        @statistic[powerGeneration](title="Power generation"; source=powerGenerationChanged; record=vector; interpolationmode=sample-hold; unit=W);
        @statistic[residualEnergyCapacity](title="Residual energy capacity"; source=residualEnergyCapacityChanged; record=vector,last; interpolationmode=linear; unit=J);
}

