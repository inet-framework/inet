//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4layer.ipv4_modular;

import inet.queueing.base.PacketPusherBase;


simple Ipv4HeaderChecker extends PacketPusherBase
{
    parameters:
        string icmpModule;
        @class(Ipv4HeaderChecker);
        @signal[packetReceivedFromLower](type=cPacket);
        @display("i=block/uparrow");
}
