//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.unitdisk;

import inet.common.SimpleModule;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitterAnalogModel;

//
// This transmitter analog model produces signals which are described with a few
// distance-based parameters.
//
// @see ~UnitDiskRadioMedium, ~UnitDiskMediumAnalogModel, ~UnitDiskReceiverAnalogModel.
//
module UnitDiskTransmitterAnalogModel extends SimpleModule like ITransmitterAnalogModel
{
    parameters:
        double communicationRange @unit(m);                              // The range where the reception of transmissions produced by this transmitter is possible
        double interferenceRange @unit(m) = default(communicationRange); // The range where transmissions produced by this transmitter interfere with other transmissions
        double detectionRange @unit(m) = default(interferenceRange);     // The range where transmissions produced by this transmitter can be detected at all
        @class(UnitDiskTransmitterAnalogModel);
        @display("i=block/tunnel");
}
