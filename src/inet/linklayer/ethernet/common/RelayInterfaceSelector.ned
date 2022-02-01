//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketFlow;

//
// This module selects the outgoing interface for the packet passing through
// from the MAC address table (forwarding information database) based on the
// destination MAC address. The selected interface is attached to the packet
// in an ~InterfaceReq. The packet may be duplicated if multiple interfaces
// are found.
//
simple RelayInterfaceSelector extends PacketPusherBase like IPacketFlow
{
    parameters:
        string macTableModule; // relative module path of MAC table
        string interfaceTableModule; // relative module path of interface table
        @class(RelayInterfaceSelector);
        @display("i=block/fork");
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropped](title="packets dropped"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}

