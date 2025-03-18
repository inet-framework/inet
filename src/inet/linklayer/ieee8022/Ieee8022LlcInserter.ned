//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFlowBase;

//
// Creates and inserts IEEE 802.2 LLC headers into packets at the link layer.
// Determines the DSAP (Destination Service Access Point) and SSAP (Source
// Service Access Point) fields based on the encapsulated protocol, and sets the
// control field. Adds appropriate protocol tags to the packet for further
// processing by lower layers. Supports both protocol-based SAP determination and
// explicit SAP values from packet tags.
//
simple Ieee8022LlcInserter extends PacketFlowBase like IIeee8022LlcInserter
{
    parameters:
        @class(Ieee8022LlcInserter);
        @display("i=block/inserter");
}

