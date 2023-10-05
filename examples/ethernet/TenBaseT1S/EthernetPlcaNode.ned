//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ethernet.TenBaseT1S;

import inet.applications.contract.IApp;
import inet.common.MessageDispatcher;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.networklayer.common.InterfaceTable;
import inet.node.contract.IEthernetNetworkNode;
import inet.physicallayer.wired.ethernet.ISpi;
import inet.protocolelement.authentication.MessageAuthenticationCodeInserter;
import inet.queueing.contract.IPacketDelayer;


//
// This module implements an Ethernet network node that is suitable for use in
// Ethernet 10BASE-T1S multidrop links. Such a multidrop link uses the Ethernet
// Phyisical Layer Collision Avoidance (PLCA) protocol. The protocol is defined
// in the IEEE 802.3cg-2019 standard.
//
// This module is a bare bones, simplified network node. It can be configured with
// multiple applications, and it also contains additional modules for simulating
// SPI delay, encryption delay, and message authentication code header overhead.
//
module EthernetPlcaNode like IEthernetNetworkNode
{
    parameters:
        int numApps = default(0);
        int numEthInterfaces = default(1);
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        @networkNode;
        @display("i=device/card");
    gates:
        inout ethg[numEthInterfaces];
    submodules:
        interfaceTable: InterfaceTable {
            @display("p=100,100;is=s");
        }
        app[numApps]: <> like IApp {
            @display("p=250,100,row,150");
        }
        at: MessageDispatcher {
            @display("p=250,175;b=300,5,,,,1");
        }
        spi: <default("Spi2")> like ISpi {
            @display("p=250,250");
        }
        authentication: MessageAuthenticationCodeInserter {
            @display("p=250,400");
        }
        encryption: <default("PacketDelayer")> like IPacketDelayer {
            @display("p=250,550");
        }
        ethernet: <default("EthernetLayer")> like IEthernetLayer {
            @display("p=250,700");
        }
        eth[numEthInterfaces]: <default("EthernetPlcaInterface")> like IEthernetInterface {
            @display("p=250,850");
        }
    connections allowunconnected:
        for i=0..numApps-1 {
            app[i].socketOut --> at.in++;
            app[i].socketIn <-- at.out++;
        }

        at.out++ --> spi.in;
        spi.out --> authentication.in;
        authentication.out --> encryption.in;
        encryption.out --> ethernet.upperLayerIn;
        ethernet.lowerLayerOut --> eth[0].upperLayerIn;

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> { @display("m=s"); } <--> eth[i].phys;
        }

        eth[0].upperLayerOut --> ethernet.lowerLayerIn;
        ethernet.upperLayerOut --> at.in++;
}
