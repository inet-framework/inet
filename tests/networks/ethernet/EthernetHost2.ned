//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.tests.ethernet;

import inet.applications.ethernet.EtherTrafGen;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.linklayer.ieee8022.IIeee8022Llc;
import inet.networklayer.common.InterfaceTable;


//
// An example host with one Ethernet interface and an Ethernet traffic
// generator directly connected to it. This host model does not contain
// higher layer protocols (IP, TCP). By default is configured to use
// half-duplex MAC (CSMA/CD).
//
module EthernetHost2
{
    parameters:
        bool csmacdSupport = default(true);  // by default use CSMA/CD
        string fcsMode @enum("declared","computed") = default("declared");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.fcsMode = this.fcsMode;
        eth.csmacdSupport = this.csmacdSupport;
        llc.registerProtocol = true;
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/pc2");
    gates:
        inout ethg @labels(EtherFrame-conn);
    submodules:
        app: EtherTrafGen {
            parameters:
                @display("p=200,100");
                interface = "eth";
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=70,200;is=s");
        }
        llc: <default("Ieee8022Llc")> like IIeee8022Llc {
            parameters:
                @display("p=200,200");
        }
        ethernet: <default("EthernetEncapsulation")> like IEthernetLayer if typename != "" {
            @display("p=200,300");
        }
        eth: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=200,400,row,150;q=txQueue");
        }
    connections:
        app.out --> llc.upperLayerIn;
        app.in <-- llc.upperLayerOut;
        llc.lowerLayerOut --> ethernet.upperLayerIn;
        llc.lowerLayerIn <-- ethernet.upperLayerOut;
        ethernet.lowerLayerOut --> eth.upperLayerIn;
        ethernet.lowerLayerIn <-- eth.upperLayerOut;
        eth.phys <--> { @display("m=s"); } <--> ethg;
}

