//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.contract;

//
// Module interface for packet defragmenters that reassemble original packets
// from fragments. Implementations collect fragments and reconstruct the
// complete packet once all fragments have been received.
//
moduleinterface IPacketDefragmenter
{
    parameters:
        @display("i=block/join");
}
