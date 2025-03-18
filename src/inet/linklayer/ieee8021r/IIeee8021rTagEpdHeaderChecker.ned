//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021r;

import inet.protocolelement.contract.IProtocolHeaderChecker;

//
// Module interface for IEEE 802.1r tag header checkers that process Ethernet
// Protocol Discrimination (EPD) format frames. Defines the interface for modules
// that validate and remove IEEE 802.1r tag headers, extract sequence numbers for
// duplicate detection, and determine the encapsulated protocol type. Used in the
// frame replication and elimination for reliability (FRER) protocol to support
// redundant transmission paths and eliminate duplicate frames.
//
moduleinterface IIeee8021rTagEpdHeaderChecker extends IProtocolHeaderChecker
{
    parameters:
        @display("i=block/checker");
}

