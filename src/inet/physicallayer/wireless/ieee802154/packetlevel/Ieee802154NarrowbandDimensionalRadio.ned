//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.physicallayer.wireless.ieee802154.packetlevel;

module Ieee802154NarrowbandDimensionalRadio extends Ieee802154NarrowbandRadio
{
    parameters:
        transmitter.typename = default("Ieee802154NarrowbandDimensionalTransmitter");
        receiver.typename = default("Ieee802154NarrowbandDimensionalReceiver");
}

