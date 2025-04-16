//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee80211.llc;

//
// Module interface for IEEE 802.11 Logical Link Control (LLC) implementations.
// LLC defines the interface between the MAC layer and network layer in IEEE 802.11
// networks. Implementations handle encapsulation and decapsulation of packets,
// using either EtherType Protocol Discrimination (EPD) or Link Protocol
// Discrimination (LPD) methods.
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
