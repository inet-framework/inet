//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.buffer;

//
// The PriorityBuffer is a specialized version of PacketBuffer that uses the
// PacketWithHighestOwnerModuleIdDropper strategy by default. This implements
// a simple priority scheme where queues with lower module IDs have higher
// priority (their packets are less likely to be dropped).
//
simple PriorityBuffer extends PacketBuffer
{
    parameters:
        packetCapacity = default(100);
        dropperClass = default("inet::queueing::PacketWithHighestOwnerModuleIdDropper");
}

