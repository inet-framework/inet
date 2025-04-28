//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee802154.bitlevel;

import inet.common.SimpleModule;
import inet.physicallayer.wireless.common.contract.packetlevel.IReceiver;

module Ieee802154UwbIrReceiver extends SimpleModule like IReceiver
{
    parameters:
        @class(Ieee802154UwbIrReceiver);
}

