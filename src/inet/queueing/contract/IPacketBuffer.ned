//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Defines the contract for packet buffer implementations
// in the queueing framework. A packet buffer provides a shared storage space
// for packets that can be used by multiple packet queues.
//
// Key responsibilities:
// - Storing packets in a shared resource with configurable capacity limits
// - Managing packet storage when capacity limits are reached
// - Implementing packet dropping strategies when buffer becomes overloaded
// - Notifying packet owners when their packets are dropped
//
// Implementations of this interface include PacketBuffer and PriorityBuffer.
//
moduleinterface IPacketBuffer
{
    parameters:
        @display("i=block/buffer");
}
