//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.linklayer.ethernet;

import inet.emulation.common.ExtInterface;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetLayer;

//
// This module provides an Ethernet network interface suitable for emulation.
// The lower part of the network interface is realized in the real world using
// a real ethernet socket of the host computer which is running the simulation.
//
module ExtLowerEthernetInterface extends ExtInterface like IEthernetInterface
{
    parameters:
        double bitrate @unit(bps) = default(nan bps);
    gates:
        input cutthroughIn @loose;
        output cutthroughOut @loose;
        inout phys @loose;
    submodules:
        encap: <default("EthernetEncapsulation")> like IEthernetLayer {
            parameters:
                @display("p=300,100");
        }
        socket: ExtEthernetSocket {
            parameters:
                @display("p=300,200");
        }
    connections allowunconnected:
        upperLayerIn --> { @display("m=n"); } --> encap.upperLayerIn;
        upperLayerOut <-- { @display("m=n"); } <-- encap.upperLayerOut;
        encap.lowerLayerOut --> socket.upperLayerIn;
        encap.lowerLayerIn <-- socket.upperLayerOut;
}
