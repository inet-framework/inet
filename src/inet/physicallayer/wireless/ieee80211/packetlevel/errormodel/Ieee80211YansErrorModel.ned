//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee80211.packetlevel.errormodel;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;

//
// Error model for IEEE 802.11 networks based on the YANS (Yet Another Network
// Simulator) model. Calculates bit error rates for different modulations (BPSK,
// QPSK, QAM) and packet error rates based on signal-to-noise ratio. Supports
// various IEEE 802.11 standards with different modulation and coding schemes,
// providing accurate error modeling for DSSS, HR-DSSS, OFDM, and other PHY
// layers used in 802.11a/b/g/n/ac networks.
//
module Ieee80211YansErrorModel extends ErrorModelBase
{
    parameters:
        @class(Ieee80211YansErrorModel);
        @display("i=block/broadcast");
}
