//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.portal;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.llc.IIeee80211Llc;
import inet.linklayer.contract.IIeee8022Llc;

simple Ieee80211Portal extends SimpleModule like IIeee8022Llc, IIeee80211Llc
{
    parameters:
        @class(Ieee80211Portal);
        string fcsMode @enum("declared", "computed") = default("declared");
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

