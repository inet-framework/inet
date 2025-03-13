//
// Copyright (C) 2015 Irene Ruengeler
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.tun;

import inet.linklayer.contract.ITunnelInterface;
import inet.networklayer.common.NetworkInterface;

//
// Provides a TUN (network tunnel) interface that creates a virtual network
// interface in the simulation. TUN interfaces capture and inject packets at
// the network layer (IP), allowing applications to process network packets
// directly without requiring physical hardware.
//
// This module is commonly used for implementing VPNs, network namespaces,
// virtual routing and forwarding (VRF), and other network virtualization
// scenarios where applications need direct access to network layer packets.
//
// @see ~Tun, ~ITunnelInterface, ~TunSocket
//
module TunInterface extends NetworkInterface like ITunnelInterface
{
    parameters:
        bool isWireless = default(false);
        string interfaceTableModule;
        int mtu @unit(B) = default(1500B);
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        @display("i=block/tunnel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
    submodules:
        tun: Tun {
            parameters:
                @display("p=300,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> tun.upperLayerIn;
        tun.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}
