//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.physicallayer.wireless.ieee802154.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.DimensionalTransmitterBase;

//
// Some example values for the frequencyGains parameter:
// - default (flat signal): "left s 0dB either e 0dB right"
// - 802.15.4 narrowband spectral mask: "left c-3.5MHz 0dB linear c+3.5MHz 0dB right"
// - 802.15.4 narrowband more realistic power spectral density (based on CC2420 ZigBee transmitter): "left c-5MHz -40dB linear c-2MHz -20dB linear c-1MHz 0dB either c+1MHz 0dB linear c+2MHz -20dB linear c+5MHz -40dB right"
//

module Ieee802154NarrowbandDimensionalTransmitter extends DimensionalTransmitterBase
{
    parameters:
        modulation = default("DSSS-OQPSK-16");
        @class(Ieee802154NarrowbandDimensionalTransmitter);
}

