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
// Implements a specialized Ethernet network interface for 10BASE-T1S multidrop
// links using the Physical Layer Collision Avoidance (PLCA) protocol as defined
// in IEEE 802.3cg-2019. This interface enables multiple nodes to share a single
// physical medium while coordinating access to prevent collisions.
//
// The interface architecture consists of three key components:
// - Ethernet MAC: Handles MAC-level framing and addressing
// - Ethernet PLCA: Coordinates medium access using the PLCA protocol
// - Ethernet PHY: Manages physical layer signaling and transmission
//
// The PLCA protocol provides deterministic access to the shared medium by
// implementing a round-robin token passing scheme at the physical layer.
// This allows multiple nodes to communicate over a single wire without the
// performance penalties of traditional CSMA/CD collision handling.
//
// When the PLCA module is present, the interface operates as a full 10BASE-T1S
// network interface supporting multidrop connections. If the PLCA module is
// omitted, the interface falls back to standard half-duplex Ethernet CSMA/CD
// operation. In multidrop configurations, nodes are typically connected using
// ~WireJunction modules and ~EthernetLink channels.
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
