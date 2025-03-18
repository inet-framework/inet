//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFilterBase;

//
// Processes IEEE 802.2 LLC headers in packets at the link layer. Validates and
// removes the LLC header, extracts the DSAP (Destination Service Access Point)
// and SSAP (Source Service Access Point) fields, and determines the encapsulated
// protocol based on these values. Adds appropriate tags to the packet for further
// processing by higher layers. Drops packets with unknown or unsupported protocol
// identifiers.
//
simple Ieee8022LlcChecker extends PacketFilterBase like IIeee8022LlcChecker
{
    parameters:
        @class(Ieee8022LlcChecker);
        @display("i=block/checker");
        @signal[packetDropped](type=inet::Packet);
}

