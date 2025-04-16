//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.portal;

import inet.linklayer.ieee80211.llc.IIeee80211Llc;
import inet.linklayer.contract.IIeee8022Llc;

//
// Implements the portal functionality in IEEE 802.11 networks, serving as a bridge
// between wireless LANs and wired networks. Performs protocol conversion between
// IEEE 802.11 frames and IEEE 802.3 (Ethernet) frames, handling frame format
// differences and LLC header management.
//
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

