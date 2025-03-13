//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Sets the Type/Length field in outgoing Ethernet frames according to IEEE 802.3
// standards. This module is responsible for determining and inserting the appropriate
// value for the 2-byte field that follows the source and destination MAC addresses
// in the Ethernet header.
//
// The Type/Length field in Ethernet frames serves a dual purpose:
// - When the value is ≥ 1536 (0x0600), it represents an EtherType that identifies
//   the protocol encapsulated in the frame's payload (e.g., 0x0800 for IPv4)
// - When the value is < 1536, it represents the length of the payload in bytes
//
// This module performs the following functions:
// - Examines the outgoing packet to determine the encapsulated protocol
// - If the protocol has a registered EtherType, sets the field to that value
// - Otherwise, calculates the payload length and sets the field accordingly
// - Ensures the field is properly encoded in network byte order
//
// The module works in conjunction with EthernetTypeOrLengthChecker, which verifies
// this field in incoming frames. Together, they ensure proper protocol identification
// and frame length validation according to IEEE 802.3 standards.
//
// @see ~EthernetMacLayer, ~EthernetTypeOrLengthChecker, ~LayeredEthernetInterface
//
simple EthernetTypeOrLengthInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(EthernetTypeOrLengthInserter);
        @display("i=block/inserter");
}
