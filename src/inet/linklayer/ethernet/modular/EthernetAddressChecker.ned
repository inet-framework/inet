//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Performs MAC address filtering for Ethernet frames in modular Ethernet interfaces.
// This module examines the destination MAC address of incoming frames and determines
// whether they should be accepted or discarded based on the interface's MAC address
// and operating mode.
//
// The module filters frames based on the following rules:
// - Frames with a destination MAC address matching the interface's MAC address are accepted
// - Broadcast frames (destination FF:FF:FF:FF:FF:FF) are always accepted
// - Multicast frames are accepted if the interface is configured to receive them
// - In promiscuous mode, all frames are accepted regardless of destination address
// - All other frames (with different unicast destination) are discarded
//
// This module is typically used within modular Ethernet interfaces as part of the
// MAC layer processing pipeline. It works in conjunction with other Ethernet protocol
// elements to implement the complete MAC functionality.
//
// @see ~EthernetMacLayer, ~EthernetAddressInserter, ~LayeredEthernetInterface
//
simple EthernetAddressChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        string interfaceTableModule;
        bool promiscuous = default(false);  // When true, accepts all frames regardless of destination address
        @class(EthernetAddressChecker);
        @display("i=block/checker");
}
