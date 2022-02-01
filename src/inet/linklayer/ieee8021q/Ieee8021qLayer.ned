//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolLayer;

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
