//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

import inet.protocolelement.contract.IProtocolHeaderChecker;

//
// Defines the interface for modules that process IEEE 802.1r R-TAG headers in
// Ethernet frames. These modules are essential for the Frame Replication and
// Elimination for Reliability (FRER) protocol defined in IEEE 802.1CB.
//
// The R-TAG header checker extracts and processes sequence numbers from frames,
// which are used to identify and eliminate duplicate frames that have traversed
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
// - Extracting sequence numbers from incoming frames
// - Maintaining sequence recovery windows for duplicate identification
// - Identifying and potentially eliminating duplicate frames
// - Removing the R-TAG header from frames before forwarding
//
// @see ~Ieee8021rTagEpdHeaderChecker, ~IIeee8021rTagEpdHeaderInserter
//
moduleinterface IIeee8021rTagEpdHeaderChecker extends IProtocolHeaderChecker
{
    parameters:
        @display("i=block/checker");
}
