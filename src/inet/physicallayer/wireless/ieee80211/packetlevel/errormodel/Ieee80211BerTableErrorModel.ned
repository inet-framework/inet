//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee80211.packetlevel.errormodel;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;

//
// Error model for IEEE 802.11 networks that uses pre-computed bit error rate
// (BER) tables to determine packet error rates. It loads BER data from the
// specified berTableFile and calculates packet error probability based on
// signal-to-noise ratio, bitrate, and packet length. Supports different IEEE
// 802.11 operation modes (a, b, g, p) and provides realistic error modeling for
// wireless simulations without the computational overhead of analytical error
// models.
//
module Ieee80211BerTableErrorModel extends ErrorModelBase
{
    parameters:
        string opMode @enum("a", "b", "g(erp)", "g(mixed)", "n", "p");
        string berTableFile;
        @class(Ieee80211BerTableErrorModel);
        @display("i=block/broadcast");
}
