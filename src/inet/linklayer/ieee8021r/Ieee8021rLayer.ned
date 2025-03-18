//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021r;

import inet.linklayer.contract.IIeee8021rLayer;
import inet.protocolelement.contract.IProtocolLayer;

//
// Implements the IEEE 802.1r protocol layer for frame replication and elimination
// to increase reliability in Ethernet networks. Consists of a policy layer for
// stream-specific handling and a protocol layer for adding sequence numbers to
// frames. Enables redundant transmission paths for critical traffic, allowing
// duplicate frames to be identified and eliminated based on sequence numbers.
// Supports Time-Sensitive Networking (TSN) applications requiring high reliability
// and deterministic delivery.
//
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
