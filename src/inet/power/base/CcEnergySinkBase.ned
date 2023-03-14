//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.base;

import inet.power.contract.ICcEnergySink;

//
// This is an abstract base module for current based energy sink models.
// It defines signals and statistics.
//
// @see ~CcEnergyConsumerBase, ~CcEnergySourceBase, ~CcEnergyGeneratorBase, ~CcEnergyStorageBase
//
simple CcEnergySinkBase like ICcEnergySink
{
    parameters:
        @display("i=block/plug");
        @signal[currentGenerationChanged];
        @statistic[currentGeneration](title="Current generation"; source=currentGenerationChanged; record=vector; interpolationmode=sample-hold; unit=A);
}

