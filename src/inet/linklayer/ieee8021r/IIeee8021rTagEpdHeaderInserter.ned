//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021r;

import inet.protocolelement.contract.IProtocolHeaderInserter;

//
// Module interface for IEEE 802.1r tag header inserters that add headers to
// Ethernet frames. Defines the interface for modules that insert IEEE 802.1r tag
// headers with sequence numbers for duplicate detection and typeOrLength fields
// identifying the encapsulated protocol. Used in the frame replication and
// elimination for reliability (FRER) protocol to support redundant transmission
// paths and enable duplicate frame detection.
//
moduleinterface IIeee8021rTagEpdHeaderInserter extends IProtocolHeaderInserter
{
    parameters:
        @display("i=block/inserter");
}

