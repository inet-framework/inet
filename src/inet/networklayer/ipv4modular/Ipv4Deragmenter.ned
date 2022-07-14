//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

simple Ipv4Defragmenter
{
    parameters:
        string icmpModule;
        double fragmentTimeout @unit(s);
        string displayStringTextFormat = default("");   // TODO required for PacketProcessorBase
    gates:
        input in;
        output out;
}
