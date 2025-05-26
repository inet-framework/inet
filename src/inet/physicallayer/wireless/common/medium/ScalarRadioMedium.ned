//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.medium;

//
// Radio medium model that uses scalar representation for signal power. Provides
// a simpler and more computationally efficient alternative to dimensional models
// by representing signals as single scalar values rather than as functions of
// time and frequency. Suitable for simulations where detailed physical layer
// behavior is not required.
//
// @see ScalarMediumAnalogModel, ScalarTransmitterAnalogModel, ScalarReceiverAnalogModel
// @see DimensionalRadioMedium, UnitDiskRadioMedium
//
module ScalarRadioMedium extends RadioMedium
{
    parameters:
        signalAnalogRepresentation = default("scalar");
}

