//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.unitdisk;

import inet.physicallayer.wireless.common.radio.packetlevel.Radio;

//
// This radio model provides a very simple but fast and predictable physical
// layer behavior. It must be used in conjunction with the ~UnitDiskRadioMedium
// model.
//
// The idea behind this radio model is to describe transmissions with a few
// distance based parameters: communication range, interference range, and
// detection range. Whether the reception is successful or not, depends on the
// distance between the transmitter and the receiver. This model is similar to
// the unit disk radio, but it's also more general than that. The ideal model
// also supports an ideal communication channel where all transmissions are
// received sucessfully independent of distance and interfering transmissions.
//
// @see ~UnitDiskRadioMedium, ~UnitDiskAnalogModel, ~UnitDiskTransmitter,
// ~UnitDiskReceiver.
//
module UnitDiskRadio extends Radio
{
    parameters:
        transmitter.typename = default("UnitDiskTransmitter");
        receiver.typename = default("UnitDiskReceiver");
        separateTransmissionParts = default(false);
        separateReceptionParts = default(false);
        @class(UnitDiskRadio);
}

