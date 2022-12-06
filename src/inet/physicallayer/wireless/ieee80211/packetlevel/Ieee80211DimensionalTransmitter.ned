//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.DimensionalTransmitterBase;

//
// This transmitter model produces IEEE 802.11 transmissions that have
// dimensional transmission power (that changes over time and/or frequency) in
// their analog representation. The bit domain, symbol domain, and sample
// domains of the transmissions are not represented.
//
// @see ~Ieee80211DimensionalReceiver, ~Ieee80211DimensionalRadio,
// ~Ieee80211DimensionalRadioMedium.
//
// Some example values for the frequencyGains parameter:
// - default (flat signal): "left s 0dB either e 0dB right"
// - 802.11a/g/n/ac OFDM spectral mask (for 20/40/80/160MHz bandwidth): "left c-b*1.5 -40dB linear c-b -28dB linear c-b*0.5-1MHz -20dB linear c-b*0.5+1MHz 0dB linear c+b*0.5-1MHz 0dB linear c+b*0.5+1MHz -20dB linear c+b -28dB linear c+b*1.5 -40dB right"
//
module Ieee80211DimensionalTransmitter extends DimensionalTransmitterBase
{
    parameters:
        // TODO remove parameters when base module is changed
        string opMode @enum("a", "b", "g(erp)", "g(mixed)", "n(mixed-2.4Ghz)", "p", "ac");
        string bandName @enum("2.4 GHz", "5 GHz", "5 GHz (20 MHz)", "5 GHz (40 MHz)", "5 GHz (80 MHz)", "5 GHz (160 MHz)", "5.9 GHz");
        int channelNumber;
        @class(Ieee80211DimensionalTransmitter);
}

