//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFlowBase;

//
// Creates and inserts IEEE 802.2 Logical Link Control (LLC) headers into
// outgoing packets. This module prepares packets from upper layers for
// transmission over link layer technologies that use LLC headers.
//
// The inserter performs several key functions:
// - Creates LLC headers with appropriate DSAP/SSAP fields based on the packet's protocol
// - Sets the control field according to the LLC Type 1 (connectionless) service
// - Attaches the LLC header to outgoing packets
// - Maps network layer protocol types to appropriate SAP values
//
// The LLC header enables protocol multiplexing across the link layer, allowing
// multiple network layer protocols to share the same physical medium. The header
// contains Service Access Point (SAP) identifiers that indicate which higher-layer
// protocol the frame should be delivered to at the receiving end.
//
// @see ~Ieee8022LlcHeader, ~Ieee8022LlcChecker, ~Ieee8022SnapInserter
//
simple Ieee8022LlcInserter extends PacketFlowBase like IIeee8022LlcInserter
{
    parameters:
        @class(Ieee8022LlcInserter);
        @display("i=block/inserter");
}
