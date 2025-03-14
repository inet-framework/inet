//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// A packet filter that drops packets when the following module (consumer)
// does not allow packets to be pushed.
//
// Operation mechanism:
// - Always accepts incoming packets from the producer (canPushPacket() returns true)
// - Checks if the downstream consumer can accept the packet
// - Forwards packets only when the consumer is ready to receive them
// - Drops packets when the consumer cannot accept them
//
// Unlike other filters that make decisions based on packet content, this filter
// bases its decision solely on the state of the connected consumer module.
//
simple BackPressureBasedFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        @class(BackPressureBasedFilter);
}
