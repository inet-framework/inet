//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021r;

import inet.queueing.base.PacketFlowBase;

//
// Adds IEEE 802.1r tag headers to Ethernet frames for the frame replication and
// elimination for reliability (FRER) protocol. Inserts sequence numbers into the
// headers for duplicate detection, and sets the typeOrLength field based on the
// encapsulated protocol. Supports the Ethernet Protocol Discrimination (EPD) format
// where the tag header includes an EtherType field identifying the encapsulated
// protocol. Can be configured to use a specific next protocol in the protocol stack.
//
simple Ieee8021rTagEpdHeaderInserter extends PacketFlowBase like IIeee8021rTagEpdHeaderInserter
{
    parameters:
        string nextProtocol = default("ieee8021qctag");
        @class(Ieee8021rTagEpdHeaderInserter);
        @display("i=block/inserter");
}

