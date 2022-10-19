//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4layer.ipv4_modular;

import inet.queueing.base.PacketPusherBase;


//
// TODO documentation
//
simple Ipv4NetfilterHook extends PacketPusherBase like INetfilterHook
{
    parameters:
        @class(::inet::Ipv4NetfilterHook);
        @display("i=old/hook");
}
