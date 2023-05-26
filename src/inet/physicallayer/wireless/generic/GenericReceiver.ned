//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.generic;

import inet.physicallayer.wireless.common.base.packetlevel.SnirReceiverBase;
import inet.physicallayer.wireless.common.contract.packetlevel.IReceiver;

//
// This receiver is part of the generic radio architecture.
//
// @see ~GenericTransmitter, ~GenericRadio.
//
module GenericReceiver extends SnirReceiverBase like IReceiver
{
    parameters:
        double energyDetection @unit(dBm) = default(-85 dBm);
        snirThresholdMode = "mean";
        snirThreshold = -100dB; // to match the previous permissive behavior
        *.ignoreInterference = default(this.ignoreInterference);
        @class(GenericReceiver);
        @display("i=block/wrx");
}

