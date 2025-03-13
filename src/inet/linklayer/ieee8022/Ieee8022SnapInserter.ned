//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFlowBase;

//
// Creates and inserts IEEE 802.2 SNAP (SubNetwork Access Protocol) headers into
// outgoing packets. This module prepares packets from upper layers for
// transmission over link layer technologies that use SNAP headers for extended
// protocol identification.
//
// The SNAP header extends the basic LLC header with a 5-byte protocol identifier:
// - 3-byte Organizationally Unique Identifier (OUI)
// - 2-byte protocol type field
//
// This extended protocol identification space allows for a much larger range
// of protocols to be identified compared to the limited space available in
// the standard LLC header's DSAP/SSAP fields.
//
// The inserter performs several key functions:
// - Creates SNAP headers with appropriate OUI and protocol type values
// - Sets the LLC header fields to indicate SNAP encapsulation
// - Attaches the SNAP header to outgoing packets
// - Maps network layer protocol types to appropriate SNAP protocol identifiers
//
// SNAP headers are commonly used with Ethernet and other IEEE 802 networks
// to support protocols that cannot be directly identified using the standard
// LLC header's limited addressing space.
//
// @see ~Ieee8022SnapHeader, ~Ieee8022SnapChecker, ~Ieee8022LlcInserter
//
simple Ieee8022SnapInserter extends PacketFlowBase like IIeee8022SnapInserter
{
    parameters:
        @class(Ieee8022SnapInserter);
        @display("i=block/inserter");
}
