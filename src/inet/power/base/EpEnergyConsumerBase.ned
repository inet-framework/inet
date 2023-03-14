//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.base;

import inet.power.contract.IEpEnergyConsumer;

//
// This is an abstract base module for power based energy consumer models.
// It defines shared signals and statistics.
//
// @see ~EpEnergySourceBase, ~EpEnergyGeneratorBase, ~EpEnergySinkBase, ~EpEnergyStorageBase
//
simple EpEnergyConsumerBase like IEpEnergyConsumer
{
    parameters:
        @display("i=block/plug");
        @signal[powerConsumptionChanged](type=double);
        @statistic[powerConsumption](title="Power consumption"; source=powerConsumptionChanged; record=vector; interpolationmode=sample-hold; unit=W);
}

