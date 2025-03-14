//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFilter;

//
// Protocol layer that implements IEEE 802.1Q stream filtering functionality with separate filters for 
// ingress and egress traffic. Provides a framework for applying filtering policies to network streams 
// based on various criteria such as VLAN IDs, PCP values, or stream identifiers. Typically configured 
// with a SimpleIeee8021qFilter for ingress traffic, while the egress filter is optional.
//
// @see ~IProtocolLayer, ~SimpleIeee8021qFilter, ~StreamPolicyLayer, ~IPacketFilter
//
module StreamFilterLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        ingress: <default("SimpleIeee8021qFilter")> like IPacketFilter {
            @display("p=200,150");
        }
        egress: <default("")> like IPacketFilter {
            @display("p=500,150");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> egress.in;
        egress.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> ingress.in;
        ingress.out --> { @display("m=n"); } --> upperLayerOut;
}
