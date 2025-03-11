//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.marker;

import inet.queueing.base.PacketTaggerBase;
import inet.queueing.contract.IPacketMarker;

//
// Network protocol parameter tagger that attaches various request tags to packets
// based on configurable filter criteria. This module enables protocol-specific
// parameter configuration for packets as they flow through the network stack.
//
// Operation mechanism:
// - Evaluates each packet against a filter function
// - For matching packets, attaches multiple protocol-specific request tags
// - Supports a wide range of network parameters including DSCP, ECN, ToS, hop limit,
//   VLAN ID, PCP, user priority, interface selection, transmission power, and
//   encapsulation protocols
//
// The PacketTagger is particularly useful for:
// - Implementing Quality of Service (QoS) mechanisms
// - Configuring protocol-specific parameters for selected traffic flows
// - Setting up differentiated services in IP networks
// - Controlling packet routing and forwarding behavior
// - Configuring physical layer parameters for wireless transmissions
//
// The module uses a pluggable filter function that implements the IPacketFilterFunction
// interface, allowing for flexible and extensible packet selection criteria.
//
// @see ~DscpReq, ~EcnReq, ~TosReq, ~HopLimitReq, ~VlanReq, ~PcpReq, ~UserPriorityReq
//
simple PacketTagger extends PacketTaggerBase like IPacketMarker
{
    parameters:
        string filterClass = default("inet::queueing::AnyPacketFilter"); // Determines which packets are tagged
        @class(PacketTagger);
}
