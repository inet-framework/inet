//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.loopback;

import inet.linklayer.contract.ILoopbackInterface;
import inet.networklayer.common.NetworkInterface;

//
// This module implements a loopback network interface.
//
module LoopbackInterface extends NetworkInterface like ILoopbackInterface
{
    parameters:
        @display("i=block/ifcard");
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
    submodules:
        lo: Loopback {
            parameters:
                @display("p=300,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> lo.upperLayerIn;
        lo.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}

