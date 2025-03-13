//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

import inet.protocolelement.contract.IProtocolLayer;

//
// Defines the interface for IEEE 802.1CB Frame Replication and Elimination for
// Reliability (FRER) protocol layer implementations. This protocol provides seamless
// redundancy for critical Ethernet networks by duplicating frames across multiple
// paths and eliminating duplicates at the receiving end.
//
// The IEEE 802.1r tag is used to implement sequence numbering for frames, which
// is essential for duplicate identification and elimination. This mechanism enables
// networks to achieve ultra-high reliability with zero recovery time in case of
// link or node failures.
//
// Modules implementing this interface handle:
// - Frame sequence numbering
// - Frame replication across redundant paths
// - Duplicate frame elimination
// - Management of sequence recovery windows
//
// This protocol is particularly important in time-sensitive networking (TSN)
// applications where reliability is critical, such as industrial automation,
// automotive networks, and other mission-critical systems.
//
// @see ~Ieee8021rLayer, ~Ieee8021rProtocol
//
moduleinterface IIeee8021rLayer extends IProtocolLayer
{
    parameters:
        @display("i=block/layer");
}
