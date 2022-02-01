//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.portal;

import inet.linklayer.ieee80211.llc.IIeee80211Llc;
import inet.linklayer.ieee8022.IIeee8022Llc;

simple Ieee80211Portal like IIeee8022Llc, IIeee80211Llc
{
    parameters:
        string fcsMode @enum("declared", "computed") = default("declared");
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

