//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee802154.packetlevel.errormodel;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;

//
// Error model for IEEE 802.15.4 wireless networks. Calculates bit error rate,
// symbol error rate, and packet error rate based on the signal-to-noise ratio
// and IEEE 802.15.4 modulation parameters. Treats header and data separately
// when computing packet error rates, and handles different signal parts (whole,
// preamble, header, data) appropriately. Used in the physical layer simulation
// of low-power wireless personal area networks (LR-WPANs).
//
module Ieee802154ErrorModel extends ErrorModelBase
{
    parameters:
        @class(Ieee802154ErrorModel);
}
