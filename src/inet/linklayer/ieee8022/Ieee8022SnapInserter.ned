//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFlowBase;

//
// Adds SNAP (SubNetwork Access Protocol) headers to packets in IEEE 802.2 LLC
// frames. Examines protocol tags in incoming packets to determine appropriate
// OUI (Organizationally Unique Identifier) and protocol ID values for the SNAP
// header. Creates and inserts headers at the front of packets, handling both
// Ethertype-based protocols (OUI=0) and OUI-based protocols. Updates protocol
// tags to indicate packets now contain SNAP headers.
//
simple Ieee8022SnapInserter extends PacketFlowBase like IIeee8022SnapInserter
{
    parameters:
        @class(Ieee8022SnapInserter);
        @display("i=block/inserter");
}
