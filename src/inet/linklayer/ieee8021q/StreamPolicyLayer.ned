//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolLayer;

//
// Compound protocol layer that implements IEEE 802.1Q stream processing functionality through a series 
// of specialized sublayers. Combines stream identification, relay, and coding operations in sequence to 
// provide comprehensive stream handling. Processes packets through three stages: identification (tagging 
// streams), relay (forwarding between interfaces), and coding (adding redundancy or other encoding).
//
// @see ~IProtocolLayer, ~StreamFilterLayer, ~StreamIdentifierLayer, ~StreamRelayLayer, ~StreamCoderLayer
//
module StreamPolicyLayer like IProtocolLayer
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
        streamIdentifier: <default("StreamIdentifierLayer")> like IProtocolLayer {
            @display("p=300,150");
        }
        streamRelay: <default("StreamRelayLayer")> like IProtocolLayer {
            @display("p=300,300");
        }
        streamCoder: <default("StreamCoderLayer")> like IProtocolLayer {
            @display("p=300,450");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> streamIdentifier.upperLayerIn;
        streamIdentifier.lowerLayerOut --> streamRelay.upperLayerIn;
        streamRelay.lowerLayerOut --> streamCoder.upperLayerIn;
        streamCoder.lowerLayerOut --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> streamCoder.lowerLayerIn;
        streamCoder.upperLayerOut --> streamRelay.lowerLayerIn;
        streamRelay.upperLayerOut --> streamIdentifier.lowerLayerIn;
        streamIdentifier.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}
