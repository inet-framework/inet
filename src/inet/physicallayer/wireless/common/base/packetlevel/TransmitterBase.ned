//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.common.base.packetlevel;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitterAnalogModel;

//
// Base module for wireless transmitters in the packet-level simulation of
// physical layer. Provides configuration for the analog representation of
// transmissions in the wireless medium. Supports three signal representation
// types: unitDisk (simple range-based model), scalar (power/frequency as scalar
// values), and dimensional (power spectral density as a function). The
// appropriate analog model is automatically selected based on the
// signalAnalogRepresentation parameter.
//
module TransmitterBase extends Module
{
    parameters:
        string signalAnalogRepresentation @enum("unitDisk","scalar","dimensional") = default("scalar"); // Determines the analog domain signal representation of transmissions
        analogModel.typename = default(signalAnalogRepresentation == "unitDisk" ? "UnitDiskTransmitterAnalogModel" : signalAnalogRepresentation == "scalar" ? "ScalarTransmitterAnalogModel" : "DimensionalTransmitterAnalogModel");
        @display("i=block/tx");
    submodules:
        analogModel: <default("")> like ITransmitterAnalogModel if typename != "" {
            @display("p=100,100");
        }
}

