//
// Copyright (C) 2015 Irene Ruengeler
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.tun;

simple Tun
{
    parameters:
        string interfaceTableModule;    // The path to the InterfaceTable module
        int mtu @unit(B) = default(1500B);
        @display("i=block/tunnel");

        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);

    gates:
        input upperLayerIn @messageKinds(inet::TunCommandCode);    // from upper layers
        output upperLayerOut @messageKinds(inet::TunSocketIndication);  // to upper layers
}

