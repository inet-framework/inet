//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.bitlevel;

import inet.physicallayer.wireless.apsk.packetlevel.ApskRadio;

//
// This radio model is part of a simple hypothetical layered radio. It
// produces detailed transmissions that have separate representation for
// all simulated domains. It must be used in conjunction with the
// ~ApskLayeredDimensionalRadioMedium model.
//
// @see ~ApskLayeredDimensionalRadioMedium, ~LayeredDimensionalAnalogModel,
// ~ApskLayeredTransmitter, ~ApskLayeredReceiver.
//
module ApskLayeredDimensionalRadio extends ApskRadio
{
    parameters:
        transmitter.typename = default("ApskLayeredTransmitter");
        receiver.typename = default("ApskLayeredReceiver");
}

