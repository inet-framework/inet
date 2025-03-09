//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.contract;

//
// Defines the interface for packet fragmenter modules that split packets into
// smaller fragments for transmission over networks with maximum transmission
// unit (MTU) constraints.
//
// Operation:
// - Receives packets from upper layers
// - Divides packets into smaller fragments according to fragmentation policy
// - Adds necessary fragmentation information to each fragment
// - Forwards fragments to lower layers
// - Maintains relationships between original packets and their fragments
//
// @see ~IPacketDefragmenter, ~IFragmenterPolicy
//
moduleinterface IPacketFragmenter
{
    parameters:
        @display("i=block/fork");
}
