//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.virtual;

import inet.linklayer.contract.IVirtualInterface;
import inet.networklayer.common.NetworkInterface;

//
// This module implements a virtual network interface.
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

