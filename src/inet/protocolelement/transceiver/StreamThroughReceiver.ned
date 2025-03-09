//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.StreamingReceiverBase;
import inet.protocolelement.transceiver.contract.IPacketReceiver;

//
// Receives signals from the physical medium as streams and forwards them to
// upper layers as streams. Maintains streaming mode throughout the entire
// processing chain, enabling signal preemption.
//
// Operation:
// - Processes incoming signal streams incrementally
// - Forwards signal parts to upper layers as they arrive
// - Preserves streaming nature of signals end-to-end
// - Supports preemption of ongoing transmissions
// - Maintains minimal processing delay for time-sensitive applications
//
// Particularly useful for:
// - Implementing fully streaming protocol stacks
// - Creating receivers for time-sensitive networking
// - Supporting preemptible transmission systems
// - Building low-latency communication systems
// - Simulating realistic streaming communication with preemption
//
// @see ~StreamThroughTransmitter, ~DestreamingReceiver, ~PacketReceiver
//
simple StreamThroughReceiver extends StreamingReceiverBase like IPacketReceiver
{
    parameters:
        @class(StreamThroughReceiver);
}
