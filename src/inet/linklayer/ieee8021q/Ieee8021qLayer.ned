//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.linklayer.contract.IIeee8021qLayer;
import inet.protocolelement.contract.IProtocolLayer;

//
// Implements a modular IEEE 802.1Q layer that provides VLAN and QoS functionality
// for Ethernet networks. This compound module integrates policy enforcement and
// protocol handling into a complete IEEE 802.1Q implementation.
//
// The IEEE 802.1Q layer processes frames in both directions, adding VLAN tags to
// outgoing frames and removing/interpreting tags from incoming frames. It also
// implements traffic prioritization and shaping mechanisms defined in the standard.
//
// The module consists of two main components:
// - Policy: Handles VLAN policy decisions such as filtering, priority mapping, and
//   traffic classification. The default implementation uses ~VlanPolicyLayer.
// - Protocol: Manages the actual IEEE 802.1Q protocol operations including tag
//   insertion/removal and socket support. The default implementation uses
//   ~Ieee8021qProtocol.
//
// This layered architecture allows for flexible configuration of IEEE 802.1Q
// functionality by enabling the replacement of either component with custom
// implementations. The module can be used in various network devices that require
// VLAN support, including switches, routers, and end stations.
//
// @see ~IIeee8021qLayer, ~Ieee8021qProtocol, ~VlanPolicyLayer
//
module Ieee8021qLayer like IIeee8021qLayer
{
    parameters:
        @display("i=block/layer");
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        policy: <default("VlanPolicyLayer")> like IProtocolLayer {
            @display("p=300,150");
        }
        protocol: <default("Ieee8021qProtocol")> like IIeee8021qLayer {
            @display("p=300,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> policy.upperLayerIn;
        policy.lowerLayerOut --> protocol.upperLayerIn;
        protocol.lowerLayerOut --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> protocol.lowerLayerIn;
        protocol.upperLayerOut --> policy.lowerLayerIn;
        policy.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}
