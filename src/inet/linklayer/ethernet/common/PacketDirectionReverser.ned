//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// This module is part of the layer 2 architecture. It turns an incoming packet
// into an outgoing packet simply by removing all attached indication tags and
// turning some of them into an attached request tag on the packet.
//
simple PacketDirectionReverser extends PacketFlowBase like IPacketFlow
{
    parameters:
        bool forwardVlan = default(true);
        bool forwardPcp = default(true);
        object excludeEncapsulationProtocols = default([]);
        @class(PacketDirectionReverser);
}

