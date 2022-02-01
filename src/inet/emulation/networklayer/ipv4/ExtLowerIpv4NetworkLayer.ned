//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.networklayer.ipv4;

import inet.common.MessageDispatcher;
import inet.networklayer.configurator.ipv4.Ipv4NodeConfigurator;
import inet.networklayer.contract.IArp;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.ipv4.IIgmp;
import inet.networklayer.ipv4.IIpv4;
import inet.networklayer.ipv4.Icmp;
import inet.networklayer.ipv4.Ipv4NatTable;
import inet.networklayer.ipv4.Ipv4RoutingTable;

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

