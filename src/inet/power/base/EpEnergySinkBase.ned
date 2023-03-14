//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.base;

import inet.power.contract.IEpEnergySink;

//
// This is an abstract base module for power based energy sink models.
// It defines signals and statistics.
//
// @see ~EpEnergyConsumerBase, ~EpEnergySourceBase, ~EpEnergyGeneratorBase, ~EpEnergyStorageBase
//
simple EpEnergySinkBase like IEpEnergySink
{
    parameters:
        @display("i=block/plug");
        @signal[powerGenerationChanged];
        @statistic[powerGeneration](title="Power generation"; source=powerGenerationChanged; record=vector; interpolationmode=sample-hold; unit=W);
}

