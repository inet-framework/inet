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
// This module implements a TUN network interface.
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

