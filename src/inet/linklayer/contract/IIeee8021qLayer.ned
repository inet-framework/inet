//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

import inet.protocolelement.contract.IProtocolLayer;

//
// Defines the interface for IEEE 802.1Q layer implementations. IEEE 802.1Q is a
// networking standard that supports Virtual LANs (VLANs) and Quality of Service
// (QoS) features on Ethernet networks.
//
// The IEEE 802.1Q standard provides:
// - VLAN tagging to logically separate networks on the same physical infrastructure
// - Priority-based traffic handling through Priority Code Point (PCP) values
// - Traffic shaping mechanisms for time-sensitive networking applications
// - Frame preemption capabilities for critical traffic
//
// Modules implementing this interface provide the protocol layer functionality
// for handling IEEE 802.1Q frames, including tag insertion/removal, traffic
// classification, and various traffic shaping mechanisms defined in the standard.
//
// @see ~Ieee8021qLayer, ~Ieee8021qProtocol, ~IProtocolLayer
//
moduleinterface IIeee8021qLayer extends IProtocolLayer
{
    parameters:
        @display("i=block/layer");
}
