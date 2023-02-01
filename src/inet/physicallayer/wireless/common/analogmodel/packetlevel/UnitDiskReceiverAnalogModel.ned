//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IReceiverAnalogModel;

module UnitDiskReceiverAnalogModel like IReceiverAnalogModel
{
    parameters:
        @class(UnitDiskReceiverAnalogModel);
        @display("i=block/tunnel");
}
