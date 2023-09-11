//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet;

import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.basic.EthernetCsmaMac;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wired.ethernet.EthernetCsmaPhy;
import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.ITrafficConditioner;

//
// This module represents an Ethernet PLCA capable network interface.
//
module EthernetPlcaInterface extends NetworkInterface like IEthernetInterface
{
    parameters:
        string interfaceTableModule;
        string protocol = default("ethernetmac");
        string address @mutable = default("auto");   // MAC address as hex string (12 hex digits), or
                                                     // "auto". "auto" values will be replaced by
                                                     // a generated MAC address in init stage 0.
        double bitrate @unit(bps) = default(0bps);
        string fcsMode @enum("declared","computed") = default("declared");
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.fcsMode = this.fcsMode;
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(EthernetSignal);
    submodules:
        egressTC: <default("")> like ITrafficConditioner {
            parameters:
                @display("p=300,100");
        }
        ingressTC: <default("")> like ITrafficConditioner {
            parameters:
                @display("i=block/uparrow;p=700,100");
        }
        queue: <default("EthernetQueue")> like IPacketQueue {
            parameters:
                packetCapacity = default(1000);
                @display("p=300,250");
        }
        mac: EthernetCsmaMac {
            parameters:
                @display("p=500,300");
        }
        plca: <default("EthernetPlca")> like IProtocolLayer {
            parameters:
                @display("p=500,450");
        }
        phy: EthernetCsmaPhy {
            parameters:
                @display("p=500,600");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> egressTC.in;
        egressTC.out --> queue.in;
        queue.out --> mac.upperLayerIn;

        mac.lowerLayerOut --> plca.upperLayerIn;
        plca.lowerLayerOut --> phy.upperLayerIn;

        phy.upperLayerOut --> plca.lowerLayerIn;
        plca.upperLayerOut --> mac.lowerLayerIn;

        phy.phys <--> { @display("m=s"); } <--> phys;

        mac.upperLayerOut --> ingressTC.in;
        ingressTC.out --> { @display("m=n"); } --> upperLayerOut;
}

