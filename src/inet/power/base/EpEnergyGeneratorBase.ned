//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.base;

import inet.power.contract.IEpEnergyGenerator;

//
// This is an abstract base module for power based energy generator models.
// It defines shared signals and statistics.
//
// @see ~EpEnergyConsumerBase, ~EpEnergySourceBase, ~EpEnergySinkBase, ~EpEnergyStorageBase
//
simple EpEnergyGeneratorBase like IEpEnergyGenerator
{
    parameters:
        @display("i=block/plug");
        @signal[powerGenerationChanged];
        @statistic[powerGeneration](title="Power generation"; source=powerGenerationChanged; record=vector; interpolationmode=sample-hold; unit=W);
}

