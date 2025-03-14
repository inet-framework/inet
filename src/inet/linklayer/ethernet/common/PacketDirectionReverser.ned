//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Packet processing module that converts inbound packets to outbound packets by transforming indication tags 
// to request tags. Preserves essential packet attributes like MAC addresses, VLAN IDs, and priority codes 
// while changing the packet direction. Supports packet forwarding in Ethernet networks by maintaining 
// addressing information and other relevant metadata during direction reversal.
//
// @see ~DirectionReverserLayer, ~IPacketFlow, ~PacketFlowBase
//
simple PacketDirectionReverser extends PacketFlowBase like IPacketFlow
{
    parameters:
        bool forwardVlan = default(true);
        bool forwardPcp = default(true);
        object excludeEncapsulationProtocols = default([]);
        @class(PacketDirectionReverser);
}
