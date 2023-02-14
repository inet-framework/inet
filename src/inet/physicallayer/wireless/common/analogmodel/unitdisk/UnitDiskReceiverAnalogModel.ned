//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.unitdisk;

import inet.physicallayer.wireless.common.contract.packetlevel.IReceiverAnalogModel;

module UnitDiskReceiverAnalogModel like IReceiverAnalogModel
{
    parameters:
        bool ignoreInterference = default(false); // true means interfering transmissions are ignored when determining whether a reception is successful or not
        @class(UnitDiskReceiverAnalogModel);
        @display("i=block/tunnel");
}
