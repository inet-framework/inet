//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IErrorModel;
import inet.physicallayer.wireless.common.contract.packetlevel.IReceiver;

//
// This module servces as a base module for narrowband receiver models.
//
module NarrowbandReceiverBase extends SnirReceiverBase like IReceiver
{
    parameters:
        double energyDetection @unit(dBm); // no signal is detected at all below this reception power threshold (idle state)
        double sensitivity @unit(dBm);     // reception is not possible if the signal power is below sensitivity (idle or busy states)
        double centerFrequency @unit(Hz); // center frequency of the band where this receiver listens on the medium
        double bandwidth @unit(Hz);        // bandwidth of the band where this receiver listens on the medium
        string modulation;                 // one of "BPSK", "16-QAM", "256-QAM"
        @display("i=block/wrx");

    submodules:
        errorModel: <> like IErrorModel if typename != "" {
            @display("p=100,100");
        }
}

