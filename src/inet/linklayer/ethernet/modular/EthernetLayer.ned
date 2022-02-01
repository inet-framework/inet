//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.protocolelement.contract.IProtocolHeaderChecker;
import inet.protocolelement.contract.IProtocolHeaderInserter;
import inet.queueing.contract.IPacketFlow;

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
        socketPacketProcessor: <default(hasSocketSupport ? "EthernetSocketPacketProcessor" : "")> like IPacketFlow {
            @display("p=600,100");
        }
        macHeaderChecker: <default("EthernetMacHeaderChecker")> like IProtocolHeaderChecker {
            @display("p=600,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> socketCommandProcessor.in;
        socketCommandProcessor.out --> macHeaderInserter.in;
        macHeaderInserter.out --> paddingInserter.in;
        paddingInserter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> macHeaderChecker.in;
        macHeaderChecker.out --> socketPacketProcessor.in;
        socketPacketProcessor.out --> { @display("m=n"); } --> upperLayerOut;
}
