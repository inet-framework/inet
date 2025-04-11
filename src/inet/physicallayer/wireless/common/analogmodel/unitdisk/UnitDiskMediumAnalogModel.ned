//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.unitdisk;

import inet.common.SimpleModule;
import inet.physicallayer.wireless.common.contract.packetlevel.IMediumAnalogModel;

//
// This analog model provides a very simple and predictable physical layer
// behaviour. It determines the reception power by categorizing transmissions
// based on the distance between the transmitter and the receiver.
//
// @see ~UnitDiskRadioMedium, ~UnitDiskTransmitterAnalogModel, ~UnitDiskReceiverAnalogModel.
//
module UnitDiskMediumAnalogModel extends SimpleModule like IMediumAnalogModel
{
    parameters:
        @class(UnitDiskMediumAnalogModel);
        @display("i=block/tunnel");
}

