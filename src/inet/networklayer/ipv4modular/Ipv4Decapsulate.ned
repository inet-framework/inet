//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

simple Ipv4Decapsulate
{
    parameters:
        string displayStringTextFormat = default("");   // TODO required for PacketProcessorBase
        string icmpModule; // ModuleReference
        @display("i=block/checker");
    gates:
        input in;
        output out;
}
