//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.power.storage;

import inet.power.base.CcEnergyStorageBase;

//
// This battery model maintains a residual charge capacity by integrating the
// difference between the total consumed current and the total generated current
// over time. This model uses a charge independent ideal voltage source and an
// charge independent internal resistance. It initiates node crash when the
// residual charge capacity reaches zero. See the base module for signals and
// statistics.
//
simple SimpleCcBattery extends CcEnergyStorageBase
{
    parameters:
        double nominalCapacity @unit(C); // maximum amount of charge stored
        double initialCapacity @unit(C) = default(nominalCapacity); // the initially stored amount of charge
        double nominalVoltage @unit(V); // open circuit voltage at maximum charge
        double internalResistance @unit(Ohm); // internal resistance
        string displayStringTextFormat = default("%c (%p)");
        @class(SimpleCcBattery);
}

