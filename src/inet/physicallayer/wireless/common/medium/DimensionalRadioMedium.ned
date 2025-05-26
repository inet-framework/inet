//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.medium;

//
// Radio medium model that uses dimensional representation (time and frequency
// domain) for signal power. Provides more accurate modeling of signal
// propagation, interference, and reception compared to scalar or unit disk
// models. Suitable for simulations requiring detailed physical layer behavior.
//
// @see DimensionalMediumAnalogModel, DimensionalTransmitterAnalogModel, DimensionalReceiverAnalogModel
// @see ScalarRadioMedium, UnitDiskRadioMedium
//
module DimensionalRadioMedium extends RadioMedium
{
    parameters:
        signalAnalogRepresentation = default("dimensional");
}

