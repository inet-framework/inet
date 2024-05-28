//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.physicallayer.wireless.ieee802154.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.FlatRadioBase;

module Ieee802154NarrowbandRadio extends FlatRadioBase
{
    parameters:
        transmitter.typename = default("Ieee802154NarrowbandTransmitter");
        receiver.typename = default("Ieee802154NarrowbandReceiver");

        centerFrequency = default(2450 MHz);

        // B_20dB ATmega256RFR2 (page 564)
        bandwidth = default(2.8 MHz);

        // 802.15.4-2006 (page 28)
        *.bitrate = default(250 kbps);

        // PHY Header (without preamble), 802.15.4-2006 (page 43)
        // 1 octet SFD
        // 7 bit Frame length
        // 1 bit Reserved
        *.headerLength = (1*8 + 7 + 1) * 1 b;

        // Preamble
        // 4 octets Preamble
        // 1 symbol of 16us -> 4 bits
        transmitter.preambleDuration = (4*8/4) * 16 us;

        // RSSI sensitivity (ATmega256RFR2, page 566)
        receiver.energyDetection = default(-90 dBm);

        // Receiver sensitivity (ATmega256RFR2, page 565)
        // TODO That is not quite true, because sensitivity
        //      is defined as the input signal power that yields
        //      a PER < 1% for a PSDU of 20 octets, but INET handles it
        //      as minimum reception power.
        receiver.sensitivity = default(-100 dBm);

        // There is no fixed boundary, because of the
        // DSSS and the capture effect. Taking the sensitivity minus some
        // arbitrary value as an approximate guess.
        receiver.minInterferencePower = default(-120 dBm);

        // Minimum SNIR
        // -8 dB results in 98% PER for a PSDU of 20 octets
        receiver.snirThreshold = default(-8 dB);

        // TX Output power (typ. 3.5 dBm, ATmega256RFR2, page 564)
        transmitter.power = default(2.24 mW);

        @class(FlatRadioBase);
}

