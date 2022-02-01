//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.contract.IPacketFlow;

//
// This module implements the IEEE 802.1Q protocol encapsulation/decapsulation.
// It also provides socket support so applications can use the protocol directly.
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
