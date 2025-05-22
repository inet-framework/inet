//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.networklayer.ipv4;

import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.ipv4.IIpv4;


//
// Provides a simplified network layer for emulation that connects the simulation
// to a real network on the host computer. Uses ~ExtLowerIpv4 to send packets from
// the simulation to the real network and receive packets from the real network
// into the simulation using raw sockets.
//
// Using this network layer in a host or router implies that layers above IPv4
// are simulated, while layers below IPv4 are provided by the host operating system.
//
module ExtLowerIpv4NetworkLayer like INetworkLayer
{
    parameters:
        @display("i=block/fork");

    gates:
        input ifIn @labels(INetworkHeader);
        output ifOut @labels(INetworkHeader);
        input transportIn @labels(Ipv4ControlInfo/down);
        output transportOut @labels(Ipv4ControlInfo/up);

    submodules:
        ip: <default("ExtLowerIpv4")> like IIpv4 {
            parameters:
                @display("p=50,50");
        }

    connections allowunconnected:
        transportIn --> ip.transportIn;
        transportOut <-- ip.transportOut;
}

