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
import inet.linklayer.contract.IEthernetLayer;
import inet.linklayer.contract.IIeee8022Llc;
import inet.networklayer.common.InterfaceTable;


//
// A simplified network device that communicates directly over Ethernet without
// higher layer protocols like IP or TCP. It provides a basic host model for
// Ethernet-only simulations.
//
// This module includes Ethernet application client and server components that generate
// and respond to traffic at the Ethernet level. The host uses IEEE 802.2 LLC (Logical
// Link Control) for data link layer functionality and can be configured to use
// CSMA/CD (Carrier Sense Multiple Access with Collision Detection) for media access
// control in half-duplex mode.
//
// @see ~EthernetSwitch, ~EthernetHub, ~StandardHost
//
module EthernetHost
{
    parameters:
        bool hasStatus = default(false);
        bool csmacdSupport = default(true);  // By default, use CSMA/CD
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

