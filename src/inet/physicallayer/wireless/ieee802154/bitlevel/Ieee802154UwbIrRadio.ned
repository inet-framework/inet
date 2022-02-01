//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee802154.bitlevel;

import inet.physicallayer.wireless.common.radio.packetlevel.Radio;

module Ieee802154UwbIrRadio extends Radio
{
    parameters:
        transmitter.typename = default("Ieee802154UwbIrTransmitter");
        receiver.typename = default("Ieee802154UwbIrReceiver");
        @class(Radio);
}

