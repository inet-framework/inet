//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.unitdisk;

import inet.physicallayer.wireless.common.contract.packetlevel.IAnalogModel;

//
// This analog model provides a very simple and predictable physical layer
// behavior. It determines the reception power by categorizing transmissions
// based on the distance between the transmitter and the receiver. It doesn't
// support noise and signal to noise ratio calculations.
//
// @see ~UnitDiskRadio, ~UnitDiskRadioMedium, ~UnitDiskTransmitter,
// ~UnitDiskReceiver.
//
module UnitDiskAnalogModel like IAnalogModel
{
    parameters:
        @class(UnitDiskAnalogModel);
        @display("i=block/tunnel");
}

