//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet tagger modules.
//
simple PacketTaggerBase extends PacketMarkerBase
{
    parameters:
        int dscp = default(-1); // specifies differentiated services code point
        int ecn = default(-1); // specifies explicit congestion notification
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        string interfaceName = default(""); // specifies outgoing interface, unused by default
        int hopLimit = default(-1); // specifies network layer TTL, unused by default
        int vlanId = default(-1); // specifies VLAN, unused by default
        int pcp = default(-1); // specifies PCP, unused by default
        int userPriority = default(-1); // specifies user priority, unused by default
        double transmissionPower @unit(W) = default(nan W); // specifies signal transmission power, unused by default
        object encapsulationProtocols = default([]);
        displayStringTextFormat = default("tagged %p pk (%l)");
        @class(PacketTaggerBase);
        @display("i=block/star");
}
