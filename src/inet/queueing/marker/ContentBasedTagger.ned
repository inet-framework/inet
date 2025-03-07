//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.marker;

import inet.queueing.base.PacketTaggerBase;
import inet.queueing.contract.IPacketMarker;

//
// Deep packet inspection tagger that attaches protocol-specific request tags to packets
// based on their content. This module enables dynamic protocol parameter configuration
// by examining packet data and attaching appropriate control information.
//
// Operation mechanism:
// - Evaluates each packet against a content-based filter expression
// - For matching packets, attaches multiple protocol-specific request tags
// - Supports a wide range of network parameters including DSCP, ECN, ToS, hop limit,
//   VLAN ID, PCP, user priority, interface selection, transmission power, and
//   encapsulation protocols
//
// The ContentBasedTagger is particularly useful for:
// - Implementing content-aware Quality of Service (QoS) mechanisms
// - Configuring protocol-specific parameters based on packet content
// - Setting up differentiated services based on deep packet inspection
// - Controlling packet routing and forwarding behavior based on payload
// - Configuring physical layer parameters for specific traffic types
//
// The module uses a filter expression that can examine any aspect of the packet,
// including headers, payload, and metadata, allowing for sophisticated protocol
// parameter configuration based on actual content.
//
// @see ~PacketTagger, ~DscpReq, ~EcnReq, ~TosReq, ~HopLimitReq, ~VlanReq, ~PcpReq
//
simple ContentBasedTagger extends PacketTaggerBase like IPacketMarker
{
    parameters:
        object packetFilter = default("*"); // Packet filter expressions, matches all packets by default
        @class(ContentBasedTagger);
}
