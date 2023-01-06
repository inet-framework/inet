//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4layer.ipv4_modular;

import inet.queueing.base.PacketPusherBase;

//
// Reassembles fragmented IPv4 packets for local delivery. It does this by
// adding the packet to an internal fragmentation buffer and checking if the
// packet completes a datagram. If it does, the module sends out the completed
// packet, otherwise it does nothing. If the packet is not a fragment, it is
// passed to the output immediately.
//
simple Ipv4Defragmenter extends PacketPusherBase
{
    parameters:
        @class(Ipv4Defragmenter);
        string icmpModule;
        double fragmentTimeout @unit(s);
        @display("i=block/defrag");
}
