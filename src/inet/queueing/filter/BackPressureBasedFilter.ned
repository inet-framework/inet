//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Flow control filter that implements back-pressure by selectively forwarding packets
// based on downstream consumer availability. This module enables transparent congestion
// management without requiring explicit signaling.
//
// Operation mechanism:
// - Always accepts incoming packets from the producer (canPushPacket() returns true)
// - Checks if the downstream consumer can accept the packet
// - Forwards packets only when the consumer is ready to receive them
// - Drops packets when the consumer cannot accept them
// - Transparently forwards protocol registration between input and output gates
//
// Unlike other filters that make decisions based on packet content, this filter
// bases its decision solely on the state of the connected consumer module.
//
// @see ~PacketFilter, ~ContentBasedFilter, ~GatingQueue
//
simple BackPressureBasedFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        @class(BackPressureBasedFilter);
}
