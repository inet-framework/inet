//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.storage;

import inet.power.base.EpEnergyStorageBase;

//
// This energy storage model stores an infinite amount of energy. It can provide
// energy for any number of consumers, and it can absorb energy from any number
// of generators. The ideal energy storage never gets completely charged or
// depleted. This module is primarily useful for testing energy consumer and
// energy generator models. See the base module for signals and statistics.
//
simple IdealEpEnergyStorage extends EpEnergyStorageBase
{
    parameters:
        @class(IdealEpEnergyStorage);
}

