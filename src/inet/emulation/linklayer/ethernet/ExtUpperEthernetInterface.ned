//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.linklayer.ethernet;

import inet.emulation.common.ExtInterface;
import inet.linklayer.contract.IEtherMac;
import inet.linklayer.contract.IEthernetInterface;

//
// This module provides an Ethernet network interface suitable for emulation.
// The upper part of the network interface is realized in the real world using
// a real TAP device of the host computer which is running the simulation.
//
module ExtUpperEthernetInterface extends ExtInterface like IEthernetInterface
{
    parameters:
        bool csmacdSupport = default(false);  // by default CSMA/CD is turned off, so only point-to-point duplex links are supported.
        double bitrate @unit(bps) = default(nan bps);
    gates:
        input cutthroughIn @loose;
        output cutthroughOut @loose;
        inout phys @labels(EthernetSignal);
    submodules:
        tap: ExtEthernetTapDevice {
            parameters:
                @display("p=300,100");
        }
        mac: <default(csmacdSupport ? "EthernetCsmaMac" : "EthernetMac")> like IEtherMac {
            parameters:
                queue.packetCapacity = 10000;
                @display("p=300,200");
        }
    connections allowunconnected:
        tap.lowerLayerOut --> { @display("m=s"); } --> mac.upperLayerIn;
        tap.lowerLayerIn <-- { @display("m=s"); } <-- mac.upperLayerOut;
        mac.phys <--> { @display("m=s"); } <--> phys;
}

