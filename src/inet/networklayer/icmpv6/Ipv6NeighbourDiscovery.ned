//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.icmpv6;

import inet.common.SimpleModule;

//
// Implements IPv6 Neighbor Discovery.
//
// An overview of the IPv6 implementation in the INET Framework is
// provided <a href="ipv6overview.html">here</a>.
//
// @see ~Ipv6, ~Ipv6RoutingTable, ~Icmpv6
//
simple Ipv6NeighbourDiscovery extends SimpleModule
{
    parameters:
        @class(Ipv6NeighbourDiscovery);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string checksumMode @enum("declared", "computed") = default("declared");
        string icmpv6Module;
        string mipv6Module;
        string routingTableModule;
        double minIntervalBetweenRAs @unit(s) = default(200s); // RFC 4861: MinRtrAdvInterval (0.33 * MaxRtrAdvInterval)
        double maxIntervalBetweenRAs @unit(s) = default(600s); // RFC 4861: MaxRtrAdvInterval
        double advReachableTime @unit(s) = default(3600s); // advertised reachable time in RAs; hosts use it as base for NUD
        int dupAddrDetectTransmits = default(1); // RFC 4862 Section 5.1: number of NS sent for DAD (0 = disable DAD)
        bool optimisticDad = default(false); // RFC 4429 Optimistic DAD: if true, an autoconfigured address may be used as a source address while DAD is still in progress; if false (RFC 4862), packets that would use it are deferred until DAD completes
        bool detectL2Movement = default(true); // if true (default), (re)start Router Discovery (send Router Solicitations) whenever an interface associates at the link layer (e.g. a wireless handover to a new access point); lets a mobile node detect movement and obtain a new care-of address immediately, instead of relying on frequent unsolicited Router Advertisements (cf. MIPv6 fast RAs). Set to false to restore the legacy behaviour (movement detected only from periodic Router Advertisements)
        double retransTimer @unit(s) = default(1s); // RFC 4861 Section 10: delay between retransmitted NS messages
        double baseReachableTime @unit(s) = default(30s); // RFC 4861 Section 10: base value for computing random ReachableTime
        volatile double routerBootupTime @unit(s) = default(uniform(0s, 0.3s)); // random startup delay before a router assigns its link-local address; routers boot faster than hosts so that RAs are available by the time hosts solicit
        volatile double hostBootupTime @unit(s) = default(uniform(0.4s, 1s)); // random startup delay before a host assigns its link-local address
        @display("i=block/network");
        @signal[startDad](type=long); // Emits value=1
        @signal[dadCompleted](type=long); // Emits value=1
        @signal[dadFailed](type=long); // Emits value=1
        @statistic[startDad](title="DAD started";record=count,vector);
        @statistic[dadCompleted](title="DAD completed";record=count,vector);
        @statistic[dadFailed](title="DAD failed";record=count,vector);
    gates:
        input ipv6In;
        output ipv6Out;
}

