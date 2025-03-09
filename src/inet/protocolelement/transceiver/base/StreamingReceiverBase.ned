//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver.base;

//
// Base module for streaming receivers that provides common functionality for
// processing signals as continuous streams rather than discrete packets.
// Extends packet receiver capabilities with streaming support.
//
// Operation:
// - Receives signals from the physical medium as streams
// - Processes signal parts as they arrive
// - Supports incremental signal processing
// - Handles partial signal reception and continuation
// - Provides foundation for specialized streaming receiver implementations
//
// Particularly useful for:
// - Implementing preemptible signal reception
// - Creating receivers for time-sensitive networking
// - Supporting signal processing with partial information
// - Building receivers that can handle interrupted transmissions
// - Simulating realistic streaming communication systems
//
// @see ~StreamingTransmitterBase, ~PacketReceiverBase, ~StreamThroughReceiver
//
simple StreamingReceiverBase extends PacketReceiverBase
{
    parameters:
        @class(StreamingReceiverBase);
}
