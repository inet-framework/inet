//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

import inet.protocolelement.contract.IProtocolLayer;

module Ieee8021rLayer like IIeee8021rLayer
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(this.interfaceTableModule);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        policy: <default("StreamPolicyLayer")> like IProtocolLayer {
            @display("p=300,150");
        }
        protocol: <default("Ieee8021rProtocol")> like IIeee8021rLayer {
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
