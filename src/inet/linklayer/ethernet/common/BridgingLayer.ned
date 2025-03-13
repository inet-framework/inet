//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.protocolelement.contract.IProtocolLayer;
import inet.protocolelement.processing.IProcessingDelayLayer;

//
// Implements a modular bridging layer that provides comprehensive layer 2 services
// for Ethernet networks. This compound module integrates multiple protocol elements
// to support advanced bridging functionality including frame forwarding, interface
// selection, VLAN processing, and Time-Sensitive Networking (TSN) features.
//
// The bridging layer consists of several protocol sublayers arranged in a processing
// pipeline, each handling specific aspects of layer 2 functionality:
// - Processing delay: Simulates the processing time required for frame handling
// - Direction reverser: Handles bidirectional packet flow through the layer
// - Stream identifier: Identifies and classifies traffic streams
// - Stream relay: Forwards streams between interfaces
// - Stream filter: Filters streams based on configured rules
// - Stream coder: Encodes/decodes stream-specific information
// - Interface relay: Selects appropriate outgoing interfaces
// - VLAN policy: Applies VLAN tagging and filtering policies
//
// This modular architecture allows for flexible configuration of bridging behavior
// by enabling or disabling specific sublayers or replacing them with custom
// implementations. The module can be used in various network devices that require
// advanced layer 2 functionality, such as bridges, switches, and routers with
// integrated switching capabilities.
//
// @see ~MacRelayUnit, ~MacForwardingTable, ~InterfaceRelayLayer
//
module BridgingLayer like IProtocolLayer
{
    parameters:
        string macTableModule = default("");
        string interfaceTableModule; // Relative module path of the interface table
        *.macTableModule = default(absPath(this.macTableModule));
        *.interfaceTableModule = default(this.interfaceTableModule);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        processingDelayLayer: <default("")> like IProcessingDelayLayer {
            @display("p=300,150");
        }
        directionReverser: <default("DirectionReverserLayer")> like IProtocolLayer {
            @display("p=300,300");
        }
        streamIdentifier: <default("")> like IProtocolLayer {
            @display("p=300,450");
        }
        streamRelay: <default("")> like IProtocolLayer {
            @display("p=300,600");
        }
        streamFilter: <default("")> like IProtocolLayer {
            @display("p=300,750");
        }
        streamCoder: <default("")> like IProtocolLayer {
            @display("p=300,900");
        }
        interfaceRelay: <default("InterfaceRelayLayer")> like IProtocolLayer {
            @display("p=300,1050");
        }
        vlanPolicy: <default("")> like IProtocolLayer {
            @display("p=300,1200");
        }
    connections:
        upperLayerIn --> processingDelayLayer.upperLayerIn;
        processingDelayLayer.lowerLayerOut --> directionReverser.upperLayerIn;
        directionReverser.lowerLayerOut --> streamIdentifier.upperLayerIn;
        streamIdentifier.lowerLayerOut --> streamRelay.upperLayerIn;
        streamRelay.lowerLayerOut --> streamFilter.upperLayerIn;
        streamFilter.lowerLayerOut --> streamCoder.upperLayerIn;
        streamCoder.lowerLayerOut --> interfaceRelay.upperLayerIn;
        interfaceRelay.lowerLayerOut --> vlanPolicy.upperLayerIn;
        vlanPolicy.lowerLayerOut --> lowerLayerOut;

        lowerLayerIn --> vlanPolicy.lowerLayerIn;
        vlanPolicy.upperLayerOut --> interfaceRelay.lowerLayerIn;
        interfaceRelay.upperLayerOut --> streamCoder.lowerLayerIn;
        streamCoder.upperLayerOut --> streamFilter.lowerLayerIn;
        streamFilter.upperLayerOut --> streamRelay.lowerLayerIn;
        streamRelay.upperLayerOut --> streamIdentifier.lowerLayerIn;
        streamIdentifier.upperLayerOut --> directionReverser.lowerLayerIn;
        directionReverser.upperLayerOut --> processingDelayLayer.lowerLayerIn;
        processingDelayLayer.upperLayerOut --> upperLayerOut;
}
