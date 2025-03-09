//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.lifetime;

import inet.protocolelement.contract.IPacketLifeTimer;

//
// Monitors network interface carrier state and drops packets from a collection
// when the carrier is lost. Ensures packets don't remain queued when an interface
// becomes unavailable.
//
// Operation:
// - Subscribes to interface state change signals
// - Monitors the carrier state of the containing network interface
// - Clears the packet collection when carrier is lost
// - Drops packets with appropriate notification signals
// - Checks carrier state when new packets are pushed to the collection
//
// Particularly useful for:
// - Implementing interface down behavior in network protocols
// - Preventing packet buildup in queues when links are down
// - Simulating carrier-dependent packet lifetime management
// - Modeling realistic network interface behavior
// - Implementing clean queue management during link failures
//
// @see ~IPacketLifeTimer
//
simple CarrierBasedLifeTimer like IPacketLifeTimer
{
    parameters:
        string collectionModule;
        @display("i=block/timer");
        @class(CarrierBasedLifeTimer);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropInterfaceDown](title="packet drop: interface down"; source=packetDropReasonIsInterfaceDown(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}
