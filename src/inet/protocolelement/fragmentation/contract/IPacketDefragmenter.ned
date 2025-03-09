//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.contract;

//
// Defines the interface for packet defragmenter modules that reassemble
// original packets from fragments received over networks with maximum
// transmission unit (MTU) constraints.
//
// Operation:
// - Receives packet fragments from lower layers
// - Identifies fragments belonging to the same original packet
// - Buffers fragments until all pieces of a packet are received
// - Reassembles original packets from collected fragments
// - Forwards complete packets to upper layers
//
// @see ~IPacketFragmenter
//
moduleinterface IPacketDefragmenter
{
    parameters:
        @display("i=block/join");
}
