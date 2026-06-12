//
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2005 Wei Yang, Ng
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Edited with OMNeT++ Academic/Noncommercial Edition.
//

package inet.networklayer.ipv6;

import inet.common.MessageDispatcher;
import inet.networklayer.configurator.contract.IIpv6NodeConfigurator;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.icmpv6.Icmpv6;
import inet.networklayer.icmpv6.Ipv6NeighbourDiscovery;
import inet.networklayer.mipv6.BindingCache;
import inet.networklayer.mipv6.BindingUpdateList;
import inet.networklayer.mipv6.Mipv6;

//
// Represents an IPv6 network layer (L3).
//
// The module has ports to connect to a higher layer (TCP, UDP) and
// several network interfaces.
//
module Ipv6NetworkLayer like INetworkLayer
{
    parameters:
        bool hasMipv6 = default(false);
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        *.mipv6Module = hasMipv6 ? absPath(".mipv6") : "";
        string interfaceTableModule;
        *.forwarding = this.forwarding;
        *.multicastForwarding = this.multicastForwarding;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.routingTableModule = default(absPath(".routingTable"));
        *.ipv6NeighbourDiscoveryModule = default(absPath(".neighbourDiscovery"));
        *.icmpv6Module = default(absPath(".icmpv6"));
        *.bindingUpdateListModule = default(absPath(".buList"));
        *.bindingCacheModule = default(absPath(".bindingCache"));
        @display("i=block/fork");

    gates:
        input ifIn @labels(INetworkHeader);
        output ifOut @labels(INetworkHeader);
        input transportIn @labels(Ipv6ControlInfo/down);
        output transportOut @labels(Ipv6ControlInfo/up);

    submodules:
        configurator: <default("Ipv6NodeConfigurator")> like IIpv6NodeConfigurator if typename != "" {
            parameters:
                @display("p=100,50;is=s");
        }
        routingTable: Ipv6RoutingTable {
            parameters:
                @display("p=100,100;is=s");
        }
        up: MessageDispatcher {
            parameters:
                @display("p=550,100;b=600,5");
        }
        icmpv6: Icmpv6 {
            parameters:
                @display("p=700,200");
        }
        lp: MessageDispatcher {
            parameters:
                @display("p=550,300;b=600,5");
        }
        neighbourDiscovery: Ipv6NeighbourDiscovery {
            parameters:
                @display("p=400,400");
        }
        // TODO rename to ip
        ipv6: Ipv6 {
            parameters:
                @display("p=550,400");
        }
        mipv6: Mipv6 if hasMipv6 {
            parameters:
                @display("p=700,500");
        }
        buList: BindingUpdateList if hasMipv6 {
            parameters:
                @display("p=850,450");
        }
        bindingCache: BindingCache if hasMipv6 {
            parameters:
                @display("p=850,550");
        }

    connections allowunconnected:  // FIXME remove 'nocheck'!
        transportIn --> { @display("m=n"); } --> up.in++;
        transportOut <-- { @display("m=n"); } <-- up.out++;

        up.out++ --> lp.in++;
        up.in++ <-- lp.out++;

        up.out++ --> icmpv6.transportIn;
        up.in++ <-- icmpv6.transportOut;

        icmpv6.ipv6Out --> lp.in++;
        icmpv6.ipv6In <-- lp.out++;

        lp.out++ --> ipv6.transportIn;
        lp.in++ <-- ipv6.transportOut;

        neighbourDiscovery.ipv6Out --> ipv6.ndIn;
        neighbourDiscovery.ipv6In <-- ipv6.ndOut;

        mipv6.fromIPv6 <-- lp.out++ if hasMipv6;
        mipv6.toIPv6 --> lp.in++ if hasMipv6;

        ipv6.queueOut --> { @display("m=s"); } --> ifOut;
        ipv6.queueIn <-- { @display("m=s"); } <-- ifIn;
}

