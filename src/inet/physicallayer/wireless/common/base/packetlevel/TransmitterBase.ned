//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.common.CompoundModule;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitterAnalogModel;

module TransmitterBase extends CompoundModule
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

