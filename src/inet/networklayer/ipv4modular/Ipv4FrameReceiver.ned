//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

simple Ipv4FrameReceiver
{
    parameters:
        string icmpModule;
        string displayStringTextFormat = default("");   // TODO required for PacketProcessorBase
        @signal[packetReceivedFromLower](type=cPacket);
    gates:
        input in;
        output out;
}
