//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for various packet tagger modules.
//
simple PacketTaggerBase extends PacketMarkerBase
{
    parameters:
        int dscp = default(-1); // Specifies differentiated services code point
        int ecn = default(-1); // Specifies explicit congestion notification
        int tos = default(-1); // If not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        string interfaceName = default(""); // Specifies outgoing interface, unused by default
        int hopLimit = default(-1); // Specifies network layer TTL, unused by default
        int vlanId = default(-1); // Specifies VLAN, unused by default
        int pcp = default(-1); // Specifies PCP, unused by default
        int userPriority = default(-1); // Specifies user priority, unused by default
        double transmissionPower @unit(W) = default(nan W); // Specifies signal transmission power, unused by default
        object encapsulationProtocols = default([]);
        displayStringTextFormat = default("tagged %p pk (%l)");
        @class(PacketTaggerBase);
        @display("i=block/star");
}
