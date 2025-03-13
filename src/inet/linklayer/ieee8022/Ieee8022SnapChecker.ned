//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFilterBase;

//
// Processes and validates IEEE 802.2 SNAP (SubNetwork Access Protocol) headers
// in incoming packets. This module examines SNAP headers to ensure they are valid
// and to determine the appropriate handling for the packet.
//
// The SNAP header extends the basic LLC header to provide a larger protocol
// identification space through a 5-byte protocol identifier consisting of:
// - 3-byte Organizationally Unique Identifier (OUI)
// - 2-byte protocol type field
//
// The checker performs several key functions:
// - Validates the SNAP header structure and field values
// - Extracts protocol information from the OUI and protocol type fields
// - Determines the upper layer protocol based on the SNAP header
// - Filters out packets with invalid or unsupported SNAP headers
// - Removes the SNAP header from packets before forwarding them to upper layers
//
// The module operates as a packet filter, allowing only packets with valid
// SNAP headers to pass through. Packets with invalid headers are dropped and
// a packetDropped signal is emitted for statistical purposes.
//
// @see ~Ieee8022SnapHeader, ~Ieee8022SnapInserter, ~Ieee8022LlcChecker
//
simple Ieee8022SnapChecker extends PacketFilterBase like IIeee8022SnapChecker
{
    parameters:
        @class(Ieee8022SnapChecker);
        @display("i=block/checker");
        @signal[packetDropped](type=inet::Packet);
}
