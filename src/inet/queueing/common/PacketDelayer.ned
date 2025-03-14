//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketDelayerBase;
import inet.queueing.contract.IPacketDelayer;

//
// Introduces configurable delay for packets passing through the module.
// Simulates transmission, propagation, processing, or queueing delays
// in network communication without implementing the actual mechanisms.
//
// Key features:
// - Configurable per-packet delay (constant or random distribution)
// - Optional bitrate-based additional delay calculation
// - Preserves packet content and attributes
// - Operates in push mode (passive sink, active source)
// - Supports both deterministic and stochastic delay models
//
// Operation mechanism:
// - When a packet arrives at the input, the module calculates the delay
//   using the configured delay parameter and optionally the packet length
//   and bitrate parameter
// - The packet is held for the calculated amount of simulation time
// - After the delay period, the packet is pushed to the output
// - If the delay is configured as a random distribution, packet order
//   may change during transmission
//
// The total delay is calculated as:
//   totalDelay = delay + packetLength / bitrate
//
// @see ~QueueingPacketDelayer, ~OmittedPacketDelayer, ~PacketServer
//
simple PacketDelayer extends PacketDelayerBase like IPacketDelayer
{
    parameters:
        volatile double delay @unit(s); // Specifies delay per packet
        volatile double bitrate @unit(bps) = default(inf bps); // Determines additional delay per bit
        @class(PacketDelayer);
}
