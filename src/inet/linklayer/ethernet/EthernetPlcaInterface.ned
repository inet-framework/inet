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
// This module implements a network interface that is suitable for use in Ethernet
// 10BASE-T1S multidrop links. Such a multidrop link uses the Ethernet Phyisical
// Layer Collision Avoidance (PLCA) protocol. The protocol is defined in the IEEE
// 802.3cg-2019 standard. This network interface can be used in any network node
// module (e.g. ~StandardHost) that allows replacing its network interfaces. All
// network interfaces on the same multidrop link must use this module.
//
// This module contains separate Ethernet MAC, Ethernet PLCA, and Ethernet PHY
// protocol modules. If the PLCA module is omitted, then this module implements
// a half-duplex Ethernet CSMA/CD network interface. If the PLCA module is present,
// then this module implements an Ethernet 10BASE-T1S network interface that
// supports multidrop links. In a multidrop link network nodes can be connected
// using ~WireJunction modules and ~EthernetLink channels.
//
// @see ~EthernetPlcaHost, ~EthernetPlca, ~EthernetCsmaMac, ~EthernetCsmaPhy, ~EthernetLink, ~WireJunction
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

