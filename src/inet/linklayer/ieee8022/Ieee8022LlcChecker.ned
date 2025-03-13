//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFilterBase;

//
// Processes and validates IEEE 802.2 Logical Link Control (LLC) headers in
// incoming packets. This module examines the LLC header fields to ensure they
// are valid and to determine the appropriate handling for the packet.
//
// The checker performs several key functions:
// - Validates the LLC header structure and field values
// - Extracts protocol information from the DSAP/SSAP fields
// - Determines the upper layer protocol based on the LLC header
// - Filters out packets with invalid or unsupported LLC headers
// - Removes the LLC header from packets before forwarding them to upper layers
//
// The module operates as a packet filter, allowing only packets with valid
// LLC headers to pass through. Packets with invalid headers are dropped and
// a packetDropped signal is emitted for statistical purposes.
//
// @see ~Ieee8022LlcHeader, ~Ieee8022LlcInserter, ~Ieee8022SnapChecker
//
simple Ieee8022LlcChecker extends PacketFilterBase like IIeee8022LlcChecker
{
    parameters:
        @class(Ieee8022LlcChecker);
        @display("i=block/checker");
        @signal[packetDropped](type=inet::Packet);
}
