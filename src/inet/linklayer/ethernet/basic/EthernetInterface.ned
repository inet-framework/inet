//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet;

import inet.linklayer.contract.IEtherMac;
import inet.linklayer.contract.IEthernetInterface;
import inet.networklayer.common.NetworkInterface;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.ITrafficConditioner;


//
// This module represents an Ethernet network interface.
//
module EthernetInterface extends NetworkInterface like IEthernetInterface
{
    parameters:
        string interfaceTableModule;
        string protocol = default("ethernetmac");
        string address @mutable = default("auto");   // MAC address as hex string (12 hex digits), or
                                                     // "auto". "auto" values will be replaced by
                                                     // a generated MAC address in init stage 0.
        bool duplexMode = default(true);      // selects full-duplex (true) or half-duplex (false) operation
        bool csmacdSupport = default(!duplexMode);  // by default CSMA/CD is turned off, so only point-to-point duplex links are supported.
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
        mac: <default(csmacdSupport ? "EthernetCsmaMac" : "EthernetMac")> like IEtherMac {
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

