//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.protocolelement.contract.IProtocolHeaderInserter;
import inet.queueing.base.PacketFlowBase;

//
// Creates and inserts Ethernet MAC headers into outgoing frames. This module is
// responsible for constructing the standard Ethernet frame header according to
// IEEE 802.3 specifications.
//
// The Ethernet MAC header consists of several fields:
// - Destination MAC address (6 bytes): The hardware address of the intended recipient
// - Source MAC address (6 bytes): The hardware address of the sending interface
// - EtherType/Length field (2 bytes): Indicates either the protocol type of the
//   encapsulated packet (for values ≥ 1536) or the length of the payload (for values < 1536)
//
// The module performs the following operations for each outgoing packet:
// - Creates a new Ethernet MAC header with the appropriate fields
// - Sets the source MAC address to the interface's own MAC address
// - Sets the destination MAC address based on the next hop information
// - Determines and sets the appropriate EtherType/Length field value
// - Prepends the header to the outgoing packet
//
// This module obtains the interface's MAC address from the interface table, which
// must be properly configured before the simulation starts. The destination MAC
// address is typically determined by address resolution protocols (like ARP for IPv4)
// and provided by the network layer.
//
// This module is typically used within modular Ethernet interfaces as part of the
// MAC layer processing pipeline, paired with the EthernetMacHeaderChecker module
// that processes MAC headers on incoming frames.
//
// @see ~EthernetMacLayer, ~EthernetMacHeaderChecker, ~LayeredEthernetInterface
//
simple EthernetMacHeaderInserter extends PacketFlowBase like IProtocolHeaderInserter
{
    parameters:
        string interfaceTableModule;  // Path to the InterfaceTable module to obtain the interface's MAC address
        @class(EthernetMacHeaderInserter);
        @display("i=block/inserter");
}
