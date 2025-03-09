//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.StreamingTransmitterBase;
import inet.protocolelement.transceiver.contract.IPacketTransmitter;

//
// Receives packets from upper layers as streams and transmits them to the
// physical medium as streams. Maintains streaming mode throughout the entire
// processing chain, enabling signal preemption.
//
// Operation:
// - Processes incoming packet streams incrementally
// - Forwards signal parts to physical medium as they arrive
// - Preserves streaming nature of signals end-to-end
// - Supports preemption of ongoing transmissions
// - Maintains minimal processing delay for time-sensitive applications
//
// @see ~StreamThroughReceiver, ~StreamingTransmitter, ~PacketTransmitter
//
simple StreamThroughTransmitter extends StreamingTransmitterBase like IPacketTransmitter
{
    parameters:
        @class(StreamThroughTransmitter);
}
