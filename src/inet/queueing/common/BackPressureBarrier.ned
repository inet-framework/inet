//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketFlowBase;

//
// Prevents backpressure propagation between connected queueing modules by isolating
// flow control signals. Acts as a barrier that decouples the push/pull behavior
// between upstream and downstream components in a packet processing pipeline.
//
// Key features:
// - Isolates flow control between connected modules
// - Allows pushing packets regardless of downstream module state
// - Allows pulling packets regardless of upstream module state
// - Passes packets through unchanged
// - Enables independent operation of connected modules
//
// Operation mechanism:
// - When a packet is pushed from upstream, it immediately accepts it regardless of
//   whether the downstream module is ready to receive
// - When a packet is pulled from downstream, it immediately provides one regardless of
//   whether the upstream module is ready to provide
// - May raise runtime errors if actual packet availability doesn't match expectations
//
// Applications:
// - Decoupling components with different processing rates
// - Breaking circular dependencies in flow control
// - Isolating different parts of a packet processing pipeline
// - Preventing congestion propagation across network boundaries
// - Testing module behavior in isolation
// - Creating asynchronous processing boundaries
//
// Warning: Using this module may lead to packet loss or runtime errors if the
// connected modules cannot handle the decoupled flow control properly. It should
// be used with caution and typically in combination with appropriate buffering.
//
// @see ~PacketQueue, ~PacketBuffer, ~PacketDelayer
//
simple BackPressureBarrier extends PacketFlowBase
{
    parameters:
        displayStringTextFormat = default("passed %p pk (%l)");
        @class(BackPressureBarrier);
        @display("i=block/segm");
}
