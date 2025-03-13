//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

import inet.linklayer.contract.IIeee8021rLayer;
import inet.protocolelement.contract.IProtocolLayer;

//
// Implements a modular IEEE 802.1CB Frame Replication and Elimination for Reliability
// (FRER) protocol layer. This module provides a layered architecture that separates
// policy decisions from protocol operations to enable flexible configuration of
// redundancy mechanisms.
//
// The layer consists of two main components:
// - Policy layer: Responsible for making decisions about frame replication and
//   elimination based on configured policies. This includes determining which
//   frames should be replicated, how many copies to create, and which paths to use.
// - Protocol layer: Handles the actual protocol operations including sequence number
//   management, frame tagging, and the mechanics of replication and elimination.
//
// This modular design allows for different policy implementations to be used with
// the same protocol implementation, enabling customization for different network
// requirements and redundancy strategies.
//
// The IEEE 802.1CB standard is a key component of Time-Sensitive Networking (TSN),
// providing seamless redundancy with zero recovery time for critical applications
// that cannot tolerate packet loss or network failures.
//
// @see ~Ieee8021rProtocol, ~IIeee8021rLayer
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
