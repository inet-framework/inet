//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.unitdisk;

import inet.common.SimpleModule;
import inet.physicallayer.wireless.common.contract.packetlevel.IReceiverAnalogModel;

//
// @see ~UnitDiskRadioMedium, ~UnitDiskMediumAnalogModel, ~UnitDiskReceiverAnalogModel.
//
simple UnitDiskReceiverAnalogModel extends SimpleModule like IReceiverAnalogModel
{
    parameters:
        @class(UnitDiskReceiverAnalogModel);
        bool ignoreInterference = default(false); // True means interfering transmissions are ignored when determining whether a reception is successful or not
        @display("i=block/tunnel");
}
