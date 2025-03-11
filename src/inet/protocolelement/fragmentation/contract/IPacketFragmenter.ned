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
// Particularly useful for:
// - Implementing IP fragmentation
// - Adapting large packets to network constraints
// - Supporting networks with different MTU sizes
// - Enabling transmission of packets larger than link capacity
// - Implementing protocol-specific fragmentation mechanisms
//
// @see ~IPacketDefragmenter, ~IFragmenterPolicy
//
moduleinterface IPacketFragmenter
{
    parameters:
        @display("i=block/fork");
}
