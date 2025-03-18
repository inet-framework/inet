//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021r;

import inet.queueing.base.PacketFilterBase;

//
// Processes IEEE 802.1r tag headers in Ethernet frames for the frame replication
// and elimination for reliability (FRER) protocol. Validates and removes the tag
// header, extracts sequence numbers for duplicate detection, and determines the
// encapsulated protocol type from the typeOrLength field. Supports the Ethernet
// Protocol Discrimination (EPD) format where the tag header is followed by an
// EtherType field identifying the next protocol in the stack.
//
simple Ieee8021rTagEpdHeaderChecker extends PacketFilterBase like IIeee8021rTagEpdHeaderChecker
{
    parameters:
        @class(Ieee8021rTagEpdHeaderChecker);
        @display("i=block/checker");
}

