//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4layer.ipv4_modular;

import inet.queueing.base.PacketPusherBase;

//
// Removes the IPv4 header from the packet, and converts the header's
// content into various indication tags attached to the packet.
//
simple Ipv4Decapsulate extends PacketPusherBase
{
    parameters:
        @class(Ipv4Decapsulate);
        string icmpModule; // ModuleReference
        @display("i=block/checker");
}
