//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Filters Ethernet packets based on their destination MAC address. Extracts
// source and destination addresses from Ethernet frames and adds them as tags
// for use by higher layers. Accepts packets where the destination address is
// the receiving interface's MAC address, the broadcast address, or a multicast
// address for which the interface has group membership. Drops the rest as "not
// addressed to us". When in promiscuous mode, accepts all packets regardless of
// destination address.
//
simple EthernetAddressChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        string interfaceTableModule;
        bool promiscuous = default(false);
        @class(EthernetAddressChecker);
        @display("i=block/checker");
}
