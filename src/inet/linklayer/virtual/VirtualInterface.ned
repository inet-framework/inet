//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.virtual;

import inet.linklayer.contract.IVirtualInterface;
import inet.networklayer.common.NetworkInterface;

//
// Provides a virtual network interface that can be attached to a real network
// interface, allowing multiple logical interfaces to share the same physical
// connectivity. This is similar to virtual interfaces in operating systems
// (e.g., eth0.1, vlan0, etc.).
//
// Virtual interfaces are commonly used for:
// - VLAN implementations where multiple logical networks share the same physical medium
// - Network virtualization where multiple virtual networks operate over the same physical infrastructure
// - Traffic isolation and segregation in multi-tenant environments
// - Creating specialized interfaces for specific traffic types or protocols
//
// The interface contains a VirtualTunnel module that handles the actual packet
// forwarding between the virtual interface and the real interface.
//
// @see ~VirtualTunnel, ~IVirtualInterface
//
module VirtualInterface extends NetworkInterface like IVirtualInterface
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        @display("i=block/tunnel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
    submodules:
        tunnel: VirtualTunnel {
            parameters:
                @display("p=300,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> tunnel.upperLayerIn;
        tunnel.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}
