//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.linklayer.contract.IEthernetLayer;
import inet.protocolelement.contract.IProtocolHeaderChecker;
import inet.protocolelement.contract.IProtocolHeaderInserter;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;

//
// Modular implementation of the Ethernet protocol stack that processes incoming
// and outgoing Ethernet frames. Handles MAC header insertion and checking, frame
// padding to minimum size, and Frame Check Sequence (FCS) calculation and verification.
// Optionally provides socket support for direct Ethernet communication from
// applications. Connects to upper layer protocols and lower layer physical interfaces
// through standardized gates.
//
// The processing pipeline for outgoing packets includes:
// - Socket command processing (when socket support is enabled)
// - MAC header insertion with source and destination addresses
// - Padding insertion to ensure minimum frame size (60 bytes)
// - FCS calculation and insertion
//
// The processing pipeline for incoming packets includes:
// - FCS verification to detect transmission errors
// - MAC header checking and address filtering
// - Socket packet delivery (when socket support is enabled)
//
// @see ~EthernetMacHeaderInserter, ~EthernetMacHeaderChecker, ~EthernetFcsInserter, ~EthernetFcsChecker, ~LayeredEthernetInterface
//
module EthernetLayer like IEthernetLayer
{
    parameters:
        string interfaceTableModule;
        bool hasSocketSupport = default(false);
        *.interfaceTableModule = default(this.interfaceTableModule);
        *.socketTableModule = default(absPath(".socketTable"));
        paddingInserter.minLength = default(60B);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        socketTable: EthernetSocketTable if hasSocketSupport {
            @display("p=100,100");
        }
        socketCommandProcessor: <default(hasSocketSupport ? "EthernetSocketCommandProcessor" : "")> like IPacketFlow {
            @display("p=300,100");
        }
        macHeaderInserter: <default("EthernetMacHeaderInserter")> like IProtocolHeaderInserter {
            @display("p=300,200");
        }
        paddingInserter: <default("PaddingInserter")> like IPacketFlow {
            @display("p=300,300");
        }
        fcsInserter: <default("EthernetFcsInserter")> like IPacketFlow {
            @display("p=300,400");
        }
        socketPacketProcessor: <default(hasSocketSupport ? "EthernetSocketPacketProcessor" : "")> like IPacketFlow {
            @display("p=600,100");
        }
        macHeaderChecker: <default("EthernetMacHeaderChecker")> like IProtocolHeaderChecker {
            @display("p=600,200");
        }
        fcsChecker: <default("EthernetFcsChecker")> like IPacketFilter {
            @display("p=600,400");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> socketCommandProcessor.in;
        socketCommandProcessor.out --> macHeaderInserter.in;
        macHeaderInserter.out --> paddingInserter.in;
        paddingInserter.out --> fcsInserter.in;
        fcsInserter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> fcsChecker.in;
        fcsChecker.out --> macHeaderChecker.in;
        macHeaderChecker.out --> socketPacketProcessor.in;
        socketPacketProcessor.out --> { @display("m=n"); } --> upperLayerOut;
}
