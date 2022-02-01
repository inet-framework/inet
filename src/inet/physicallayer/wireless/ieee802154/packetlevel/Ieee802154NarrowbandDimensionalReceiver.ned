//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.physicallayer.wireless.ieee802154.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandReceiverBase;

module Ieee802154NarrowbandDimensionalReceiver extends NarrowbandReceiverBase
{
    parameters:
        modulation = default("DSSS-OQPSK-16");
        errorModel.typename = default("ApskErrorModel");
        double minInterferencePower @unit(dBm);
        @class(Ieee802154NarrowbandDimensionalReceiver);
}

