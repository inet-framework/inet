//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet buffer modules. A packet
// buffer is often used by packet queues to store packets in a limited shared
// resource. Packets can be added to and removed from a packet buffer. When the
// buffer becomes overloaded, one or more packets are dropped according to the
// configured dropping strategy. For each dropped packet, the packet owner is
// notified in order to update its internal state.
//
moduleinterface IPacketBuffer
{
    parameters:
        @display("i=block/buffer");
}
