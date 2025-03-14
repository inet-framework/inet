//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.portal;

import inet.linklayer.ieee80211.llc.IIeee80211Llc;
import inet.linklayer.contract.IIeee8022Llc;

//
// Implements the Portal functionality defined in the IEEE 802.11 standard, which provides 
// integration between IEEE 802.11 wireless networks and IEEE 802.3 (Ethernet) wired networks. 
// This module handles the conversion between Ethernet frames and IEEE 802.11 frames, performing 
// the necessary encapsulation and decapsulation operations. It manages MAC address translation, 
// frame format conversion, and frame check sequence (FCS) handling. The module implements both 
// IIeee8022Llc and IIeee80211Llc interfaces, allowing it to serve as a bridge between the two 
// network types.
//
// @see ~IIeee8022Llc, ~IIeee80211Llc
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
