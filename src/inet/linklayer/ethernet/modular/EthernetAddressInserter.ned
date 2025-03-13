//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Inserts MAC addresses into outgoing Ethernet frames in modular Ethernet interfaces.
// This module adds the source and destination MAC addresses to frames as they are
// transmitted from the network layer to the physical layer.
//
// For each outgoing packet, the module performs the following operations:
// - Inserts the interface's MAC address as the source address
// - Inserts the appropriate destination MAC address based on the next hop information:
//   * For unicast packets, uses the MAC address of the next hop
//   * For broadcast packets, uses the broadcast MAC address (FF:FF:FF:FF:FF:FF)
//   * For multicast packets, derives the multicast MAC address from the multicast group address
//
// The module obtains the interface's MAC address from the interface table, which
// must be properly configured before the simulation starts. The destination MAC
// address is typically determined by address resolution protocols (like ARP for IPv4)
// and provided by the network layer.
//
// This module is typically used within modular Ethernet interfaces as part of the
// MAC layer processing pipeline. It works in conjunction with other Ethernet protocol
// elements to implement the complete MAC functionality.
//
// @see ~EthernetMacLayer, ~EthernetAddressChecker, ~LayeredEthernetInterface
//
simple EthernetAddressInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        string interfaceTableModule;  // Path to the InterfaceTable module to obtain the interface's MAC address
        @class(EthernetAddressInserter);
        @display("i=block/inserter");
}
