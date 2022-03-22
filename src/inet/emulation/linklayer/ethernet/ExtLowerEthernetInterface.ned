//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.linklayer.ethernet;

import inet.emulation.common.ExtInterface;
import inet.linklayer.contract.IEthernetInterface;

//
// This module provides an Ethernet network interface suitable for emulation.
// The lower part of the network interface is realized in the real world using
// a real ethernet socket of the host computer which is running the simulation.
//
module ExtLowerEthernetInterface extends ExtInterface like IEthernetInterface
{
    parameters:
        string protocol = default("ethernetmac");
        double bitrate @unit(bps) = default(nan bps);
    gates:
        input cutthroughIn @loose;
        output cutthroughOut @loose;
        inout phys @loose;
    submodules:
        socket: ExtEthernetSocket {
            parameters:
                @display("p=300,200");
        }
    connections allowunconnected:
        upperLayerIn --> { @display("m=n"); } --> socket.upperLayerIn;
        upperLayerOut <-- { @display("m=n"); } <-- socket.upperLayerOut;
}
