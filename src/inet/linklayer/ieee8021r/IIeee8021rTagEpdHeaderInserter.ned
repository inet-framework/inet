//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

import inet.protocolelement.contract.IProtocolHeaderInserter;

//
// Defines the interface for modules that insert IEEE 802.1r R-TAG headers into
// Ethernet frames. These headers are essential for the Frame Replication and
// Elimination for Reliability (FRER) protocol defined in IEEE 802.1CB.
//
// The R-TAG header inserter adds sequence numbers to frames, which are used by
// receivers to identify and eliminate duplicate frames that have traversed
// different network paths. This is a key mechanism for achieving seamless
// redundancy in time-sensitive networks.
//
// The EtherType Protocol Discrimination (EPD) format is used for efficient
// processing, where the header includes:
// - A reserved field (16 bits)
// - A sequence number field (16 bits)
// - The EtherType/Length field (16 bits) for the encapsulated protocol
//
// Modules implementing this interface are responsible for:
// - Generating and managing sequence numbers for outgoing frames
// - Inserting the R-TAG header with the appropriate sequence number
// - Setting the correct EtherType for the next protocol in the stack
//
// @see ~Ieee8021rTagEpdHeaderInserter, ~IIeee8021rTagEpdHeaderChecker
//
moduleinterface IIeee8021rTagEpdHeaderInserter extends IProtocolHeaderInserter
{
    parameters:
        @display("i=block/inserter");
}
