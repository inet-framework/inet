//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.contract;

//
// Module interface for packet fragmenters that split packets into smaller
// fragments. Implementations divide packets that exceed maximum transmission
// size into multiple fragments that can be reassembled at the destination.
//
moduleinterface IPacketFragmenter
{
    parameters:
        @display("i=block/fork");
}
