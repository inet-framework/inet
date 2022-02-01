//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFilter;

//
// This module combines two packet filters into a protocol layer so that it can
// be used in a layered compound module. There are separate submodules for ingress
// and egress traffic, but in most cases only the ingress filter is used.
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

