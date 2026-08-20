//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandReceiverBase;


//
// This receiver model receives an IEEE 802.11 transmission successfully if the
// minimum of the signal to noise and interference ratio over the duration of
// the reception is sufficiently high. It uses one of the IEEE 802.11 specific
// error models to compute the error rate based on this value and the used
// operation mode.
//
// @see ~Ieee80211Transmitter, ~Ieee80211ScalarRadio,
// ~Ieee80211ScalarRadioMedium.
//
module Ieee80211Receiver extends NarrowbandReceiverBase
{
    parameters:
        string opMode @enum("a","b","g(erp)","g(mixed)","n(mixed-2.4Ghz)","p","ac");
        string bandName @enum("2.4 GHz","5 GHz","5 GHz (20 MHz)","5 GHz (40 MHz)","5 GHz (80 MHz)","5 GHz (160 MHz)","5.9 GHz");
        double htCca20Sensitivity @unit(dBm) = default(-82 dBm); // IEEE 802.11 HT 20 MHz CCA sensitivity
        double htCca40Sensitivity @unit(dBm) = default(-79 dBm); // IEEE 802.11 HT 40 MHz CCA sensitivity
        double htCcaEnergyDetection @unit(dBm) = default(-62 dBm); // IEEE 802.11 HT energy-detection threshold
        int channelNumber = default(-1);
        modulation = default("BPSK"); // TODO this is simply wrong
        errorModel.typename = default("Ieee80211NistErrorModel");
        // TODO remove when opMode is removed from the error model
        *.opMode = this.opMode;
        @class(Ieee80211Receiver);
}

