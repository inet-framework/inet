//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetMacLayer;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wired.ethernet.IEthernetPhyLayer;

//
// This module represents an Ethernet network interface with cut-through support.
//
module EthernetCutthroughInterface extends NetworkInterface like IEthernetInterface
{
    parameters:
        string protocol = default("ethernetmac");
        bool broadcast = default(true);
        bool multicast = default(true);
        string address @mutable = default("auto");
        double bitrate @unit(bps);
        string interfaceTableModule;
        string macTableModule;
        *.bitrate = default(this.bitrate);
        *.macTableModule = default(this.macTableModule);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(EthernetSignal);
    submodules:
        macLayer: <default("EthernetMacLayer")> like IEthernetMacLayer {
            parameters:
                @display("p=400,100");
        }
        phyLayer: <default("EthernetStreamThroughPhyLayer")> like IEthernetPhyLayer {
            parameters:
                cutthroughSource.typename = default("EthernetCutthroughSource");
                cutthroughSink.typename = default("EthernetCutthroughSink");
                @display("p=400,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> macLayer.upperLayerIn;
        macLayer.lowerLayerOut --> phyLayer.upperLayerIn;
        phyLayer.lowerLayerOut --> { @display("m=s"); } --> phys$o;

        phys$i --> { @display("m=s"); } --> phyLayer.lowerLayerIn;
        phyLayer.upperLayerOut --> macLayer.lowerLayerIn;
        macLayer.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}

