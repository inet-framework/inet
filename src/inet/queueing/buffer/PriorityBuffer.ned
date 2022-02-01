//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.buffer;

//
// This buffer drops packets among the connected packet queues based on their
// module id.
//
simple PriorityBuffer extends PacketBuffer
{
    parameters:
        packetCapacity = default(100);
        dropperClass = default("inet::queueing::PacketWithHighestOwnerModuleIdDropper");
}

