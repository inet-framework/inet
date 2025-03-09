//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.base;

import inet.queueing.base.PacketPusherBase;

//
// Base module for packet defragmenters that provides common functionality for
// reassembling original packets from fragments. Handles buffering and
// reconstruction of fragmented packets.
//
// Operation:
// - Receives packet fragments from lower layers
// - Buffers fragments until all pieces of a packet are received
// - Tracks fragment relationships using identifiers
// - Reassembles original packets from collected fragments
// - Forwards complete packets to upper layers
//
// Particularly useful for:
// - Creating protocol-specific defragmenters with shared base functionality
// - Implementing reliable packet reconstruction
// - Handling out-of-order fragment arrival
// - Building protocol stacks with defragmentation capabilities
// - Providing a foundation for specialized defragmenter implementations
//
// @see ~FragmenterBase, ~IPacketDefragmenter
//
simple DefragmenterBase extends PacketPusherBase
{
    parameters:
        bool deleteSelf = default(false);
        @display("i=block/join");
        @class(DefragmenterBase);
}
