//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.emulation.networklayer.ipv4;

import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.ipv4.IIpv4;


//
// Provides a simplified network layer for emulation that connects the simulation
// to a real network on the host computer. Uses ExtLowerIpv4 to send packets from
// the simulation to the real network and receive packets from the real network
// into the simulation using raw sockets. Unlike ExtUpperIpv4NetworkLayer, it
// doesn't include components like routing table or ARP.
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

