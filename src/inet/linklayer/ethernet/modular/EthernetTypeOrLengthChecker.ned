//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Verifies the Type/Length field in Ethernet frames and processes frames accordingly.
// This module is responsible for interpreting the 2-byte field that follows the source
// and destination MAC addresses in the Ethernet header.
//
// The Type/Length field in Ethernet frames serves a dual purpose:
// - When the value is ≥ 1536 (0x0600), it represents an EtherType that identifies
//   the protocol encapsulated in the frame's payload (e.g., 0x0800 for IPv4)
// - When the value is < 1536, it represents the length of the payload in bytes
//
// This module performs the following functions:
// - Validates that the Type/Length field contains a valid value
// - For Length values: Verifies that the actual payload length matches the specified length
// - For EtherType values: Identifies the encapsulated protocol for proper handling
// - Discards frames with invalid Type/Length values
//
// The module works in conjunction with EthernetTypeOrLengthInserter, which sets
// this field in outgoing frames. Together, they ensure proper protocol identification
// and frame length validation according to IEEE 802.3 standards.
//
// @see ~EthernetMacLayer, ~EthernetTypeOrLengthInserter, ~LayeredEthernetInterface
//
simple EthernetTypeOrLengthChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        @class(EthernetTypeOrLengthChecker);
        @display("i=block/checker");
}
