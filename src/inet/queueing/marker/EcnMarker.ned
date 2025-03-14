//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.marker;

import inet.queueing.base.PacketMarkerBase;

//
// Explicit Congestion Notification (ECN) marker that sets ECN bits in the IPv4 header
// based on the attached `EcnReq` tag on packets. This module enables network congestion
// control by modifying IP headers to indicate congestion without dropping packets.
//
// Operation mechanism:
// - Examines packets for the presence of an EcnReq tag
// - For packets with this tag, sets the ECN field in the IPv4 header
// - Updates the IPv4 header checksum to maintain packet integrity
// - Supports both direct IP packets and Ethernet-encapsulated IP packets
//
// ECN allows routers to signal impending congestion without dropping packets,
// which can significantly improve performance for TCP and other protocols that
// support ECN.
//
// @see ~EcnReq, ~IpEcnCode
//
simple EcnMarker extends PacketMarkerBase
{
    parameters:
        @class(EcnMarker);
}
