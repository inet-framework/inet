//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

import inet.queueing.base.PacketPusherBase;


simple Ipv4Defragmenter extends PacketPusherBase
{
    parameters:
        @class(Ipv4Defragmenter);
        string icmpModule;
        double fragmentTimeout @unit(s);
        @display("i=block/defrag");
}
