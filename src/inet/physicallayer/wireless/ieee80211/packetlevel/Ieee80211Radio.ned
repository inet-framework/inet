//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.FlatRadioBase;

//
// This radio model is part of the IEEE 802.11 physical layer model. It supports
// multiple channels, different operation modes, and preamble modes. It must be
// used in conjunction with the ~Ieee80211RadioMedium model or other derived
// models.
//
// This model provides sensible default values for the radio parameters to be
// used in IEEE 802.11 simulations. It also supports switching channels via
// via direct C++ function calls.
//
// @see ~Ieee80211ScalarRadio, ~Ieee80211DimensionalRadio.
//
//# TODO check this Table 18-14—Receiver performance requirements
module Ieee80211Radio extends FlatRadioBase
{
    parameters:
        string opMode @enum("a", "b", "g(erp)", "g(mixed)", "n(mixed-2.4Ghz)", "p", "ac") = default("g(mixed)"); // Operation mode
        string bandName @enum("2.4 GHz", "5 GHz", "5 GHz (20 MHz)", "5 GHz (40 MHz)", "5 GHz (80 MHz)", "5 GHz (160 MHz)", "5.9 GHz") = default("2.4 GHz"); // Band name
        int channelNumber = default(0);                                 // Initial channel number within the band (TODO this is offset by 1)
        string crcMode @enum("declared","computed") = default("declared");
        transmitter.typename = default("Ieee80211Transmitter");
        receiver.typename = default("Ieee80211Receiver");
        receiver.errorModel.typename = default(signalAnalogRepresentation != "unitDisk" ? "Ieee80211NistErrorModel" : "");
        centerFrequency @unit(Hz) = default(2.412GHz);                 // The center frequency of the band is automatically set by the radio
        bandwidth @unit(Hz) = default(nan MHz);
        transmitter.preambleDuration = default(-1 s);
        transmitter.bitrate = default(11Mbps);
        transmitter.headerLength = default(192b);
        transmitter.power = default(20mW);
        receiver.bandwidth = default(22 MHz); // The highest value
        receiver.sensitivity = default(-85dBm);
        receiver.energyDetection = default(-85dBm);
        receiver.snirThreshold = default(4dB);
        *.opMode = this.opMode;
        *.bandName = this.bandName;
        *.channelNumber = this.channelNumber;
        @class(Ieee80211Radio);
        @signal[radioChannelChanged](type=long);
        @statistic[radioChannel](title="Radio channel"; source=radioChannelChanged; record=histogram,vector; interpolationmode=sample-hold);
}

