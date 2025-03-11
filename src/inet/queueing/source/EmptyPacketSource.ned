//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketSource;
import inet.queueing.contract.IPassivePacketSource;

//
// Simulates a permanently inactive or unavailable packet source. Implements
// both active and passive packet source interfaces but never produces any
// packets and always generates backpressure when packets are requested from it.
//
// Key features:
// - Implements all packet source interfaces for compatibility
// - Never pushes packets to connected modules
// - Always rejects pull requests with backpressure
// - Maintains statistics on attempted operations
// - Provides a placeholder for a packet source that is temporarily unavailable
//
// Applications:
// - Modeling network endpoints that are offline or unavailable
// - Testing how networks handle source unavailability
// - Simulating failed or disconnected network nodes
// - Creating placeholder components in network topologies
// - Temporarily disabling packet generation without changing network structure
//
// The EmptyPacketSource is particularly useful in scenarios where:
// - You need to test how a network responds to source unavailability
// - You want to temporarily disable packet generation from a source
// - You need to maintain the network structure while simulating a failed component
// - You want to model a network endpoint that is temporarily offline
//
// This module is the source counterpart to the FullPacketSink, which similarly
// never accepts any packets.
//
// @see ~FullPacketSink, ~ActivePacketSource, ~PassivePacketSource
//
simple EmptyPacketSource extends PacketProcessorBase like IPacketSource, IActivePacketSource, IPassivePacketSource
{
    parameters:
        displayStringTextFormat = default("created 0 pk (0 B)");
        @class(EmptyPacketSource);
        @display("i=block/source");
    gates:
        output out @labels(push,pull);
}
