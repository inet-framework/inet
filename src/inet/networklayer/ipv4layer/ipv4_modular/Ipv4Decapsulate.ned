//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4layer.ipv4_modular;

import inet.queueing.base.PacketPusherBase;


simple Ipv4Decapsulate extends PacketPusherBase
{
    parameters:
        @class(Ipv4Decapsulate);
        string icmpModule; // ModuleReference
        @display("i=block/checker");
}
