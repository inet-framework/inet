//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.networklayer.ipv4;

import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.ipv4.IIpv4;


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

