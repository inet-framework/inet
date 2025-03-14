//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee80211.llc;

//
// Interface for IEEE 802.11 Logical Link Control (LLC) implementations that handle 
// encapsulation and decapsulation of packets between the MAC and network layers. 
// Provides protocol discrimination to identify the upper layer protocol being carried 
// in the frame. Implementations include EPD (EtherType Protocol Discrimination) and 
// LPD (LLC Protocol Discrimination) variants.
//
// @see ~Ieee80211LlcEpd, ~Ieee80211LlcLpd, ~Ieee8022Llc
//
moduleinterface IIeee80211Llc
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
