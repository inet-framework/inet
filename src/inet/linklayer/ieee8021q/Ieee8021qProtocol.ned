//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.linklayer.contract.IIeee8021qLayer;
import inet.queueing.contract.IPacketFlow;

//
// Implements the core IEEE 802.1Q protocol functionality for VLAN tagging and
// frame processing. This module handles the encapsulation and decapsulation of
// Ethernet frames with IEEE 802.1Q tags, enabling VLAN separation and priority-based
// traffic handling.
//
// The module processes frames in both directions:
// - For outgoing frames: Adds appropriate VLAN tags based on configuration or
//   socket commands
// - For incoming frames: Extracts and interprets VLAN tags, then forwards frames
//   to the appropriate destination
//
// Key features:
// - Support for different VLAN tag types (C-TAG, S-TAG, etc.) through the vlanTagType
//   parameter
// - Optional socket interface allowing applications to directly control VLAN
//   parameters for their traffic
// - Modular design with separate components for tag insertion/checking and
//   socket processing
//
// The module consists of several components:
// - Socket table: Maintains socket registrations when socket support is enabled
// - Socket command processor: Processes commands from applications
// - Tag header inserter: Adds IEEE 802.1Q tags to outgoing frames
// - Tag header checker: Extracts and validates tags from incoming frames
// - Socket packet processor: Delivers incoming frames to registered applications
//
// This implementation supports the Ethertype Protocol Discrimination (EPD) method
// for frame identification and can be configured to work with different VLAN tag
// formats as specified in the IEEE 802.1Q standard.
//
// @see ~Ieee8021qLayer, ~Ieee8021qTagEpdHeaderInserter, ~Ieee8021qTagEpdHeaderChecker
//
module Ieee8021qProtocol like IIeee8021qLayer
{
    parameters:
        string vlanTagType = default("c");
        bool hasSocketSupport = default(false);
        *.vlanTagType = default(this.vlanTagType);
        *.socketTableModule = default(absPath(".socketTable"));
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        socketTable: Ieee8021qSocketTable if hasSocketSupport {
            @display("p=100,100");
        }
        socketCommandProcessor: <default(hasSocketSupport ? "Ieee8021qSocketCommandProcessor" : "")> like IPacketFlow {
            @display("p=300,100");
        }
        qTagHeaderInserter: <default("Ieee8021qTagEpdHeaderInserter")> like IIeee8021qTagEpdHeaderInserter {
            @display("p=300,200");
        }
        socketPacketProcessor: <default(hasSocketSupport ? "Ieee8021qSocketPacketProcessor" : "")> like IPacketFlow {
            @display("p=600,100");
        }
        qTagHeaderChecker: <default("Ieee8021qTagEpdHeaderChecker")> like IIeee8021qTagEpdHeaderChecker {
            @display("p=600,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> socketCommandProcessor.in;
        socketCommandProcessor.out --> qTagHeaderInserter.in;
        qTagHeaderInserter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> qTagHeaderChecker.in;
        qTagHeaderChecker.out --> socketPacketProcessor.in;
        socketPacketProcessor.out --> { @display("m=n"); } --> upperLayerOut;
}
