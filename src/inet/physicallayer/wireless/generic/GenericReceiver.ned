//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.generic;

import inet.physicallayer.wireless.common.base.packetlevel.ReceiverBase;
import inet.physicallayer.wireless.common.contract.packetlevel.IReceiver;

//
// This receiver model receives a transmission succesfully within communication
// range unless there's another interfering transmission within interference
// range. It also supports an ideal communication channel with configuring the
// receiver to ignore interfering transmissions.
//
// @see ~UnitDiskTransmitter, ~UnitDiskRadio, ~UnitDiskRadioMedium,
// ~UnitDiskMediumAnalogModel.
//
module GenericReceiver extends ReceiverBase like IReceiver
{
    parameters:
        bool ignoreInterference = default(false); // true means interfering transmissions are ignored when determining whether a reception is successful or not
        *.ignoreInterference = default(this.ignoreInterference);
        @class(GenericReceiver);
        @display("i=block/wrx");
}

