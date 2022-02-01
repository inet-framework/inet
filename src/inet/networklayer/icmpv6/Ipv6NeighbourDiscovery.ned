//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.icmpv6;

//
// Implements IPv6 Neighbour Discovery.
//
// An overview of the Ipv6 implementation in the INET Framework is
// provided <a href="ipv6overview.html">here</a>.
//
// @see ~Ipv6, ~Ipv6RoutingTable, ~Icmpv6
//
simple Ipv6NeighbourDiscovery
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string crcMode @enum("declared", "computed") = default("declared");
        string icmpv6Module;
        string xmipv6Module;
        string routingTableModule;
        double minIntervalBetweenRAs @unit(s) = default(30ms); //minRtrAdvInterval:  0.03 sec for MIPv6 , declared as parameter to facilitate testing without recompiling (Zarrar 15.07.07)
        double maxIntervalBetweenRAs @unit(s) = default(70ms);  //MaxrtrAdvInterval: 0.07 sec for MIPv6, declared as parameter to facilitate testing without recompiling (Zarrar 15.07.07)
        @display("i=block/network");
        @signal[startDad](type=long); // emits value=1
        @statistic[startDad](title="DAD started";record=count,vector);
    gates:
        input ipv6In;
        output ipv6Out;
}

