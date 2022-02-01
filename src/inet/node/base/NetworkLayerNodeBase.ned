//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.base;

import inet.common.MessageDispatcher;
import inet.networklayer.contract.INetworkLayer;

module NetworkLayerNodeBase extends LinkLayerNodeBase
{
    parameters:
        bool hasIpv4 = default(true);
        bool hasIpv6 = default(false);
        bool hasGn = default(false);
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        *.forwarding = this.forwarding;
        *.multicastForwarding = this.multicastForwarding;
        @figure[networkLayer](type=rectangle; pos=250,308; size=1000,134; fillColor=#00ff00; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[networkLayer.title](type=text; pos=1245,313; anchor=ne; text="network layer");
    submodules:
        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer if hasIpv4 {
            @display("p=375,375;q=queue");
        }
        ipv6: <default("Ipv6NetworkLayer")> like INetworkLayer if hasIpv6 {
            @display("p=525,375;q=queue");
        }
        generic: <default("")> like INetworkLayer if hasGn {
            @display("p=675,375;q=queue");
        }
        nl: MessageDispatcher {
            @display("p=750,450;b=1000,5,,,,1");
        }
    connections allowunconnected:
        ipv4.ifIn <-- nl.out++ if hasIpv4;
        ipv4.ifOut --> nl.in++ if hasIpv4;

        ipv6.ifIn <-- nl.out++ if hasIpv6;
        ipv6.ifOut --> nl.in++ if hasIpv6;

        generic.ifIn <-- nl.out++ if exists(generic);
        generic.ifOut --> nl.in++ if exists(generic);

        cb.out++ --> nl.in++;
        cb.in++ <-- nl.out++;

        llc.upperLayerOut --> nl.in++ if exists(llc);
        llc.upperLayerIn <-- nl.out++ if exists(llc);
}

