//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

simple Ipv4PrepareForward
{
    parameters:
        string icmpModule;
        string interfaceTableModule;
        string routingTableModule;
        string displayStringTextFormat = default("");   // TODO required for PacketProcessorBase
        @signal[packetDropped](type=cPacket);
        @signal[ipv4NewMulticast](type=inet::Ipv4Header);
        @signal[ipv4DataOnNonrpf](type=inet::Ipv4Header);
        @signal[ipv4DataOnRpf](type=inet::Ipv4Header);
        @signal[ipv4MdataRegister](type=inet::Packet);
    gates:
        input in;
        output out;
}
