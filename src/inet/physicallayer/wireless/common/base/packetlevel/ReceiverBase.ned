//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IReceiverAnalogModel;

module ReceiverBase
{
    parameters:
        bool ignoreInterference = default(false); // true means interfering transmissions are ignored when determining whether a reception is successful or not
        string signalAnalogRepresentation @enum("unitDisk","scalar","dimensional") = default("scalar");
        analogModel.typename = default(signalAnalogRepresentation == "unitDisk" ? "UnitDiskReceiverAnalogModel" : signalAnalogRepresentation == "scalar" ? "ScalarReceiverAnalogModel" : "DimensionalReceiverAnalogModel");
        @display("i=block/tx");
    submodules:
        analogModel: <default("")> like IReceiverAnalogModel if typename != "" {
            @display("p=100,100");
        }
}

