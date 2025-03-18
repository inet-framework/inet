//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//TODO review
package inet.physicallayer.wireless.ieee802154.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandReceiverBase;

//
// Implements an IEEE 802.15.4 narrowband receiver for wireless sensor networks
// and low-rate wireless personal area networks (LR-WPANs). It uses DSSS-OQPSK-16
// modulation by default and includes a specialized error model for IEEE 802.15.4
// transmissions. The receiver filters incoming transmissions based on signal
// characteristics and a configurable minimum interference power threshold,
// ensuring that only valid IEEE 802.15.4 signals are processed.
//
module Ieee802154NarrowbandReceiver extends NarrowbandReceiverBase
{
    parameters:
        modulation = default("DSSS-OQPSK-16");
        errorModel.typename = default("Ieee802154ErrorModel");
        double minInterferencePower @unit(dBm);
        @class(Ieee802154NarrowbandReceiver);
}

