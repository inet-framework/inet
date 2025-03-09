//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.PacketTransmitterBase;
import inet.protocolelement.transceiver.contract.IPacketTransmitter;

//
// Transmits packets from upper layers as complete signals to the physical
// medium. Implements a straightforward packet-based transmission model.
//
// Operation:
// - Processes outgoing packets as atomic units
// - Converts complete packets to signals
// - Handles transmission as a single operation
// - Maintains packet boundaries during transmission
// - Provides simple packet-based transmission without streaming
//
// @see ~PacketReceiver, ~StreamingTransmitter, ~StreamThroughTransmitter
//
simple PacketTransmitter extends PacketTransmitterBase like IPacketTransmitter
{
    parameters:
        @class(PacketTransmitter);
}
