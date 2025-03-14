//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee80211.llc;

//
// IEEE 802.11 Logical Link Control (LLC) implementation that uses EtherType Protocol 
// Discrimination (EPD) for identifying upper layer protocols. Encapsulates packets with 
// the IEEE 802 EtherType header as defined in section 9.2 of IEEE Std 802-2014. Provides 
// a standardized way to identify which network layer protocol is carried in the frame.
//
// @see ~IIeee80211Llc, ~Ieee80211LlcLpd, ~Ieee802EpdHeader
//
simple Ieee80211LlcEpd like IIeee80211Llc
{
    parameters:
        @display("i=block/layer");
        @signal[packetDropped](type=Packet);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
