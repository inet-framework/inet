//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

import inet.queueing.base.PacketFilterBase;

//
// Implements the IEEE 802.1r R-TAG header checking functionality for the Frame
// Replication and Elimination for Reliability (FRER) protocol defined in IEEE 802.1CB.
// This module processes incoming frames with R-TAG headers, extracting sequence
// numbers that are used to identify and eliminate duplicate frames.
//
// The module processes R-TAG headers in the EtherType Protocol Discrimination (EPD)
// format, which includes:
// - A reserved field (16 bits)
// - A sequence number field (16 bits)
// - The EtherType/Length field (16 bits) for the encapsulated protocol
//
// Key functions of this module include:
// - Extracting sequence numbers from incoming frames
// - Maintaining sequence recovery windows for each stream
// - Identifying duplicate frames based on sequence numbers
// - Removing the R-TAG header from frames before forwarding
// - Supporting the elimination of duplicate frames when configured
//
// This module is a key component in implementing seamless redundancy for
// time-sensitive networking applications where zero recovery time is required
// in case of network failures.
//
// @see ~IIeee8021rTagEpdHeaderChecker, ~Ieee8021rTagEpdHeaderInserter
//
simple Ieee8021rTagEpdHeaderChecker extends PacketFilterBase like IIeee8021rTagEpdHeaderChecker
{
    parameters:
        @class(Ieee8021rTagEpdHeaderChecker);
        @display("i=block/checker");
}
