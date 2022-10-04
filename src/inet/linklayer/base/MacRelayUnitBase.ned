//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.base;

simple MacRelayUnitBase
{
    parameters:
        string macTableModule;
        string interfaceTableModule;
        string displayStringTextFormat = default("proc: %p\ndisc: %d");
        @display("i=block/switch");
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromUpper](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropped](title="packets dropped"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input upperLayerIn;
        output upperLayerOut;
}

