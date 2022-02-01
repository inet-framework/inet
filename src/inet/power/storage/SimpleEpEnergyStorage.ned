//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.storage;

import inet.power.base.EpEnergyStorageBase;

//
// This energy storage model maintains a residual energy capacity by integrating
// the difference between the total consumed power and the total generated power
// over time. It initiates node crash when the residual energy capacity reaches
// zero. This model doesn't have various properties such as self-discharge, memory
// effect, overcharging, temperature-dependence, etc. that real world batteries
// have. See the base module for signals and statistics.
//
// @see ~SimpleEpEnergyManagement
//
simple SimpleEpEnergyStorage extends EpEnergyStorageBase
{
    parameters:
        double nominalCapacity @unit(J); // the maximum amount of energy stored
        double initialCapacity @unit(J) = default(nominalCapacity); // the initially stored amount of energy
        double printCapacityStep @unit(J) = default(0.01 * nominalCapacity); // displays residual capacity changes in the log
        string displayStringTextFormat = default("%c (%p)");
        @class(SimpleEpEnergyStorage);
}

