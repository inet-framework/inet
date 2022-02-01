//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandReceiverBase;

//
// This receiver model serves as the base module for IEEE 802.11 receivers. It
// supports switching channels, configuring different operation modes, and
// preamble modes.
//
// @see ~Ieee80211TransmitterBase, ~Ieee80211ScalarReceiver,
// ~Ieee80211DimensionalReceiver.
//
module Ieee80211ReceiverBase extends NarrowbandReceiverBase
{
    parameters:
        string opMode @enum("a", "b", "g(erp)", "g(mixed)", "n(mixed-2.4Ghz)", "p", "ac");
        string bandName @enum("2.4 GHz", "5 GHz", "5 GHz (20 MHz)", "5 GHz (40 MHz)", "5 GHz (80 MHz)", "5 GHz (160 MHz)", "5.9 GHz");
        int channelNumber;
        modulation = default("BPSK"); // TODO this is simply wrong
        errorModel.typename = default("Ieee80211NistErrorModel");
        // TODO remove when opMode is removed from the error model
        *.opMode = this.opMode;
}

