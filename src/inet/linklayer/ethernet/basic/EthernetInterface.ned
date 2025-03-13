//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.basic;

import inet.linklayer.contract.IEtherMac;
import inet.linklayer.contract.IEthernetInterface;
import inet.networklayer.common.NetworkInterface;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.ITrafficConditioner;


//
// Provides a standard Ethernet network interface implementation that supports
// both half-duplex and full-duplex operation modes. This module represents the
// traditional Ethernet interface model in the INET framework and serves as the
// foundation for Ethernet connectivity in network simulations.
//
// The interface architecture consists of:
// - Traffic conditioners: Optional components for traffic shaping and policing
//   on both ingress and egress paths
// - Packet queue: Buffers outgoing packets before transmission
// - MAC module: Handles medium access control, addressing, and framing
//
// Key features:
// - Configurable duplex mode (half or full)
// - Optional CSMA/CD support for half-duplex operation
// - Adjustable bitrate for different Ethernet standards (10Mbps to 10Gbps)
// - Flexible MAC address assignment (auto-generated or manually specified)
// - Frame Check Sequence (FCS) handling options
//
// The module automatically selects the appropriate MAC implementation based on
// the configured duplex mode and CSMA/CD settings. For half-duplex operation
// with CSMA/CD, it uses EthernetCsmaMacPhy; for full-duplex operation, it uses
// EthernetMacPhy.
//
// @see ~IEthernetInterface, ~EthernetCsmaMacPhy, ~EthernetMacPhy
//
module EthernetInterface extends NetworkInterface like IEthernetInterface
{
    parameters:
        string interfaceTableModule;
        string protocol = default("ethernetmac");
        string address @mutable = default("auto");   // MAC address as a hex string (12 hex digits), or
                                                     // "auto". "auto" values will be replaced by
                                                     // a generated MAC address in init stage 0.
        bool duplexMode = default(true);      // Selects full-duplex (true) or half-duplex (false) operation
        bool csmacdSupport = default(!duplexMode);  // By default, CSMA/CD is turned off, so only point-to-point duplex links are supported.
        double bitrate @unit(bps) = default(0 bps);
        string fcsMode @enum("declared","computed") = default("declared");
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.fcsMode = this.fcsMode;
        *.duplexMode = this.duplexMode;
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input cutthroughIn @loose;
        output cutthroughOut @loose;
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
        mac: <default(csmacdSupport ? "EthernetCsmaMacPhy" : "EthernetMacPhy")> like IEtherMac {
            parameters:
                @display("p=500,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> egressTC.in;
        egressTC.out --> queue.in;
        queue.out --> mac.upperLayerIn;

        mac.phys <--> { @display("m=s"); } <--> phys;

        mac.upperLayerOut --> ingressTC.in;
        ingressTC.out --> { @display("m=n"); } --> upperLayerOut;
}
