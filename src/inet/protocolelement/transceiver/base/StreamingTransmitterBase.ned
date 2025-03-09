//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver.base;

//
// Base module for streaming transmitters that provides common functionality for
// processing packets as continuous streams rather than discrete units.
// Extends packet transmitter capabilities with streaming support.
//
// Operation:
// - Converts packets into signal streams for transmission
// - Processes packets incrementally as streams
// - Supports sending signal parts separately
// - Handles transmission start and end as separate events
// - Provides foundation for specialized streaming transmitter implementations
//
// @see ~StreamingReceiverBase, ~PacketTransmitterBase, ~StreamThroughTransmitter
//
simple StreamingTransmitterBase extends PacketTransmitterBase
{
    parameters:
        @class(StreamingTransmitterBase);
}
