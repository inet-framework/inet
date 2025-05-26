//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee802154.bitlevel;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitter;

module Ieee802154UwbIrTransmitter extends Module like ITransmitter
{
    parameters:
        @class(Ieee802154UwbIrTransmitter);
}

