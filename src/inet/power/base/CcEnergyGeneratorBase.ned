//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.base;

import inet.common.SimpleModule;
import inet.power.contract.ICcEnergyGenerator;

//
// This is an abstract base module for current-based energy generator models.
// It defines shared signals and statistics.
//
// @see ~CcEnergyConsumerBase, ~CcEnergySourceBase, ~CcEnergySinkBase, ~CcEnergyStorageBase
//
simple CcEnergyGeneratorBase extends SimpleModule like ICcEnergyGenerator
{
    parameters:
        @class(CcEnergyGeneratorBase);
        @display("i=block/plug");
        @signal[currentGenerationChanged];
        @statistic[currentGeneration](title="Current generation"; source=currentGenerationChanged; record=vector; interpolationmode=sample-hold; unit=A);
}

