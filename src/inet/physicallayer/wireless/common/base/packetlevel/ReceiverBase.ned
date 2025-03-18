//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.common.base.packetlevel;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IReceiverAnalogModel;

//
// Base module for wireless receivers in the packet-level simulation of physical
// layer. Provides configuration for the analog representation of receptions in
// the wireless medium. Supports three signal representation types: unitDisk
// (simple range-based model), scalar (power/frequency as scalar values), and
// dimensional (power spectral density as a function). The appropriate analog
// model is automatically selected based on the signalAnalogRepresentation
// parameter.
//
module ReceiverBase extends Module
{
    parameters:
        bool ignoreInterference = default(false); // True means interfering transmissions are ignored when determining whether a reception is successful or not
        string signalAnalogRepresentation @enum("unitDisk","scalar","dimensional") = default("scalar");
        analogModel.typename = default(signalAnalogRepresentation == "unitDisk" ? "UnitDiskReceiverAnalogModel" : signalAnalogRepresentation == "scalar" ? "ScalarReceiverAnalogModel" : "DimensionalReceiverAnalogModel");
        @display("i=block/tx");
    submodules:
        analogModel: <default("")> like IReceiverAnalogModel if typename != "" {
            @display("p=100,100");
        }
}

