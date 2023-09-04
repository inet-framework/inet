//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ethernet.TenBaseT1Snew;

import inet.applications.contract.IApp;
import inet.linklayer.ethernet.EthernetPlcaInterface;
import inet.linklayer.ethernet.modular.EthernetLayer;
import inet.networklayer.common.InterfaceTable;
import inet.physicallayer.wired.ethernet.Spi2;
import inet.protocolelement.authentication.MessageAuthenticationCodeInserter;
import inet.queueing.common.PacketDelayer;

module TenBaseT1SNode
{
    parameters:
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        @networkNode;
        @display("i=device/card");
    gates:
        inout ethg;
    submodules:
        interfaceTable: InterfaceTable {
            @display("p=100,100;is=s");
        }
        app: <> like IApp {
            @display("p=250,100");
        }
        spi: Spi2 {
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
        eth: EthernetPlcaInterface {
            @display("p=250,850");
        }
    connections allowunconnected:
        app.socketOut --> spi.in;
        spi.out --> authentication.in;
        authentication.out --> encryption.in;
        encryption.out --> ethernet.upperLayerIn;
        ethernet.lowerLayerOut --> eth.upperLayerIn;
        eth.phys <--> { @display("m=s"); } <--> ethg;
}
