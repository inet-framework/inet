//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.generic;

import inet.physicallayer.wireless.common.base.packetlevel.ReceiverBase;
import inet.physicallayer.wireless.common.contract.packetlevel.IReceiver;

//
// @see ~GenericTransmitter, ~GenericRadio.
//
module GenericReceiver extends ReceiverBase like IReceiver
{
    parameters:
        bool ignoreInterference = default(false); // true means interfering transmissions are ignored when determining whether a reception is successful or not
        double energyDetection @unit(dBm) = default(-85 dBm);
        *.ignoreInterference = default(this.ignoreInterference);
        @class(GenericReceiver);
        @display("i=block/wrx");
}

