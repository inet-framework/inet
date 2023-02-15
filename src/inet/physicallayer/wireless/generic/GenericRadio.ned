//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.generic;

import inet.physicallayer.wireless.common.radio.packetlevel.Radio;

//
// @see ~GenericTransmitter, ~GenericReceiver.
//
module GenericRadio extends Radio
{
    parameters:
        transmitter.typename = default("GenericTransmitter");
        receiver.typename = default("GenericReceiver");
        @class(GenericRadio);
}

