//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee80211.llc;

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
