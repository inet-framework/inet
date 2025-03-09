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
