//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.base;

import inet.common.SimpleModule;
import inet.power.contract.IEpEnergySource;

//
// This is an abstract base module for power-based energy source models.
// It defines shared signals and statistics.
//
// @see ~EpEnergyConsumerBase, ~EpEnergyGeneratorBase, ~EpEnergySinkBase, ~EpEnergyStorageBase
//
simple EpEnergySourceBase extends SimpleModule like IEpEnergySource
{
    parameters:
        @class(EpEnergySourceBase);
        @display("i=block/plug");
        @signal[powerConsumptionChanged];
        @statistic[powerConsumption](title="Power consumption"; source=powerConsumptionChanged; record=vector; interpolationmode=sample-hold; unit=W);
}

