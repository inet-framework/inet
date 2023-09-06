//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ethernet.TenBaseT1S;

import inet.applications.contract.IApp;
import inet.common.MessageDispatcher;
import inet.linklayer.ethernet.EthernetPlcaInterface;
import inet.linklayer.ethernet.modular.EthernetLayer;
import inet.networklayer.common.InterfaceTable;
import inet.node.contract.IEthernetNetworkNode;
import inet.physicallayer.wired.ethernet.ISpi;
import inet.protocolelement.authentication.MessageAuthenticationCodeInserter;
import inet.queueing.common.PacketDelayer;

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
        encryption: PacketDelayer {
            @display("p=250,550");
        }
        ethernet: EthernetLayer {
            @display("p=250,700");
        }
        eth[numEthInterfaces]: EthernetPlcaInterface {
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
