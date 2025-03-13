//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.common.MessageDispatcher;
import inet.protocolelement.common.ProtocolChecker;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketFlow;

//
// Implements a comprehensive IEEE 802.2 Logical Link Control (LLC) layer with
// a modular architecture that separates different protocol functions into distinct
// components. This compound module provides a more feature-rich alternative to
// the simpler ~Ieee8022Llc module.
//
// The LLC layer handles protocol multiplexing between the network layer and the
// MAC sublayer through Service Access Point (SAP) identifiers. This implementation
// supports both standard LLC headers and SubNetwork Access Protocol (SNAP) headers
// for extended protocol identification.
//
// Key components:
// - Socket support: Optional socket interface for direct LLC communication
// - Protocol classification: Directs packets to appropriate processing paths
// - LLC header processing: Insertion and checking of LLC headers
// - SNAP header processing: Insertion and checking of SNAP headers for extended protocol IDs
// - Message dispatching: Routes packets between different processing components
//
// The modular design allows for flexible configuration, with optional components
// that can be enabled or disabled based on requirements. The module can be
// configured to support direct socket communication with the LLC layer through
// the hasSocketSupport parameter.
//
// @see ~IIeee8022LlcLayer, ~Ieee8022Llc, ~Ieee8022LlcHeader, ~Ieee8022SnapHeader
//
module Ieee8022LlcLayer like IIeee8022LlcLayer
{
    parameters:
        bool hasSocketSupport = default(false);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        socketTable: Ieee8022LlcSocketTable if hasSocketSupport {
            @display("p=100,100");
        }
        socketCommandProcessor: <default(hasSocketSupport ? "Ieee8022LlcSocketCommandProcessor" : "")> like IPacketFlow {
            @display("p=400,100");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=400,200");
        }
        snapInserter: <default("Ieee8022SnapInserter")> like IIeee8022SnapInserter if typename != "" {
            @display("p=300,300");
        }
        m1: PacketMultiplexer {
            @display("p=300,400");
        }
        llcInserter: <default("Ieee8022LlcInserter")> like IIeee8022LlcInserter if typename != "" {
            @display("p=300,500");
        }
        m2: PacketMultiplexer {
            @display("p=400,600");
        }
        socketPacketProcessor: <default(hasSocketSupport ? "Ieee8022LlcSocketPacketProcessor" : "")> like IPacketFlow {
            @display("p=700,100");
        }
        protocolChecker: ProtocolChecker {
            @display("p=700,200");
        }
        llcChecker: <default("Ieee8022LlcChecker")> like IIeee8022LlcChecker {
            @display("p=600,500");
        }
        snapChecker: <default("Ieee8022SnapChecker")> like IIeee8022SnapChecker {
            @display("p=800,500");
        }
        dp: MessageDispatcher {
            @display("p=700,400;b=100,5,,,,1");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> socketCommandProcessor.in;
        socketCommandProcessor.out --> classifier.in;
        classifier.out++ --> snapInserter.in if exists(snapInserter);
        classifier.out++ --> m1.in++;
        snapInserter.out --> m1.in++ if exists(snapInserter);
        m1.out --> llcInserter.in;
        classifier.out++ --> m2.in++ if exists(llcInserter);
        llcInserter.out --> m2.in++ if exists(llcInserter);
        m2.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> dp.in++;
        dp.out++ --> llcChecker.in;
        dp.out++ --> snapChecker.in;
        llcChecker.out --> dp.in++;
        snapChecker.out --> dp.in++;
        dp.out++ --> protocolChecker.in;
        protocolChecker.out --> socketPacketProcessor.in;
        socketPacketProcessor.out --> { @display("m=n"); } --> upperLayerOut;
}
