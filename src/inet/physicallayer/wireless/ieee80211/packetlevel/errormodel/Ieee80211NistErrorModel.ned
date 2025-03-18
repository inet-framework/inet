//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee80211.packetlevel.errormodel;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;

//
// Implements the NIST error rate model for IEEE 802.11 networks. This model
// calculates bit error rates and packet success rates for various modulation
// schemes (BPSK, QPSK, QAM16, QAM64, QAM256, QAM1024) based on signal-to-noise
// ratio. It supports different IEEE 802.11 modes including OFDM, DSSS, HT, and
// VHT, handling forward error correction with different code rates. The model
// computes success rates separately for header and data portions of packets.
//
module Ieee80211NistErrorModel extends ErrorModelBase
{
    parameters:
        @class(Ieee80211NistErrorModel);
        @display("i=block/broadcast");
}
