//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IReceiver;

//
// This module servces as a base module for SNIR receiver models.
//
module SnirReceiverBase like IReceiver
{
    parameters:
        double snirThreshold @unit(dB);    // reception is not successful if the SNIR is below this threshold (unsuccessful reception)
        string snirThresholdMode @enum("min","mean") = default("min");
        @display("i=block/wrx");
}

