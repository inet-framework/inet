//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.medium;

//
// This radio medium model provides a very simple but fast and predictable
// physical layer behavior. It must be used in conjunction with the ~UnitDiskRadio
// model.
//
// The idea behind this analog model is to describe transmissions with a few
// distance based parameters: communication range, interference range, and
// detection range. Whether the reception is successful or not, depends on the
// distance between the transmitter and the receiver. This model is similar to
// the unit disk radio, but it's also more general than that. The analog model
// also supports an ideal communication channel where all transmissions are
// received sucessfully independent of distance and interfering transmissions.
//
// @see ~UnitDiskMediumAnalogModel, ~UnitDiskTransmitterAnalogModel,
// ~UnitDiskReceiverAnalogModel.
//
module UnitDiskRadioMedium extends RadioMedium
{
    parameters:
        signalAnalogRepresentation = default("unitDisk");
}

