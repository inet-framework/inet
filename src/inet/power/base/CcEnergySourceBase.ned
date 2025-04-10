//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.base;

import inet.common.SimpleModule;
import inet.power.contract.ICcEnergySource;

//
// This is an abstract base module for current-based energy source models.
// It defines shared signals and statistics.
//
// @see ~CcEnergyConsumerBase, ~CcEnergyGeneratorBase, ~CcEnergySinkBase, ~CcEnergyStorageBase
//
simple CcEnergySourceBase extends SimpleModule like ICcEnergySource
{
    parameters:
        @class(CcEnergySourceBase);
        @display("i=block/plug");
        @signal[currentConsumptionChanged];
        @statistic[currentConsumption](title="Current consumption"; source=currentConsumptionChanged; record=vector; interpolationmode=sample-hold; unit=A);
}

