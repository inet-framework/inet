//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.ethernet;

import inet.applications.ethernet.EtherAppClient;
import inet.applications.ethernet.EtherAppServer;
import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.linklayer.ieee8022.IIeee8022Llc;
import inet.networklayer.common.InterfaceTable;


//
// An example host with one Ethernet port and a traffic generator that
// generates request-reply traffic directly over Ethernet. This host model
// does not contain higher layer protocols (IP, TCP). By default it is
// configured to use half-duplex MAC (CSMA/CD).
//
module EthernetHost
{
    parameters:
        bool hasStatus = default(false);
        bool csmacdSupport = default(true);  // by default use CSMA/CD
        string fcsMode @enum("declared","computed") = default("declared");
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/pc2");
        eth.csmacdSupport = this.csmacdSupport;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.fcsMode = this.fcsMode;
        eth.encap.typename = "";
        llc.registerProtocol = true;
    gates:
        inout ethg @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=100,100;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,240;is=s");
        }
        cli: EtherAppClient {
            parameters:
                interface = "eth";
                @display("p=350,100");
        }
        srv: EtherAppServer {
            parameters:
                @display("p=550,100");
        }
        dp: MessageDispatcher {
            parameters:
                @display("p=450,150;b=500,5,,,,1");
        }
        llc: <default("Ieee8022Llc")> like IIeee8022Llc {
            parameters:
                @display("p=450,200");
        }
        encap: <default("EthernetEncapsulation")> like IEthernetLayer if typename != "" {
            parameters:
                registerProtocol = true;
                @display("p=350,300");
        }
        ethernet: <default("")> like IEthernetLayer if typename != "" {
            @display("p=550,300");
        }
        li: MessageDispatcher {
            parameters:
                @display("p=450,350;b=500,5");
        }
        eth: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=350,400,row,150;q=txQueue");
        }
    connections:
        cli.out --> dp.in++;
        dp.out++ --> cli.in;
        srv.out --> dp.in++;
        dp.out++ --> srv.in;

        dp.out++ --> llc.upperLayerIn;
        llc.upperLayerOut --> dp.in++;
        llc.lowerLayerOut --> encap.upperLayerIn if exists(llc) && exists(encap);
        encap.upperLayerOut --> llc.lowerLayerIn if exists(llc) && exists(encap);
        encap.lowerLayerOut --> li.in++ if exists(encap);
        li.out++ --> encap.lowerLayerIn if exists(encap);
        llc.lowerLayerOut --> ethernet.upperLayerIn if exists(llc) && exists(ethernet);
        ethernet.upperLayerOut --> llc.lowerLayerIn if exists(llc) && exists(ethernet);
        ethernet.lowerLayerOut --> li.in++ if exists(ethernet);
        li.out++ --> ethernet.lowerLayerIn if exists(ethernet);

        li.out++ --> eth.upperLayerIn;
        eth.upperLayerOut --> li.in++;

        eth.phys <--> { @display("m=s"); } <--> ethg;
}

