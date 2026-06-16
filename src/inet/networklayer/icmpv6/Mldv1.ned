//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.icmpv6;

import inet.common.SimpleModule;

//
// Implements MLDv1 (RFC 2710): Multicast Listener Discovery for IPv6 -- the
// IPv6 equivalent of ~Igmpv2. A single module performs both the host listener
// role and the router querier role; the router role is active only on multicast
// routers (i.e. when IPv6 multicast forwarding is enabled on the node).
//
// MLD messages (ICMPv6 types 130/131/132, see ~MldMessage) are exchanged with
// the IPv6 layer through the `ipIn`/`ipOut` gates, which connect to the network
// layer's message dispatcher. Host membership is driven by the
// `ipv6MulticastGroupJoinedSignal` / `ipv6MulticastGroupLeftSignal`
// notifications emitted by Ipv6InterfaceData when an application joins or
// leaves an IPv6 multicast group.
//
// <b>Host behavior</b> (RFC 2710 Section 5)
//
// When an interface joins a multicast group, the host sends an unsolicited
// Multicast Listener Report to the group address, repeated after
// `unsolicitedReportInterval`. On receiving a General or Multicast-Address-
// Specific Query, the host enters the Delaying Listener state and schedules a
// Report after a random delay in [0, Maximum Response Delay]; if it first
// hears another node's Report for the group, it cancels its own (report
// suppression) and returns to the Idle Listener state. When the interface
// leaves a group for which this host was the last reporter, it sends a
// Multicast Listener Done to the all-routers address (ff02::2). Per-(interface,
// group) state is one of Non-Listener, Delaying Listener, or Idle Listener. The
// link-scope all-nodes group (ff02::1) is never reported.
//
// <b>Router (querier) behavior</b> (RFC 2710 Section 6)
//
// On a multicast router this module acts as the single querier on each attached
// link (querier election is not modelled): it sends periodic General Queries
// (every `queryInterval`), records the listeners reported on each interface in
// Ipv6InterfaceData (`reportedMulticastGroups`), and on receiving a Done sends
// group-specific Queries (called Multicast-Address-Specific Queries in RFC 2710;
// `lastListenerQueryCount` of them, spaced `lastListenerQueryInterval` apart) and
// removes the group if no Report arrives before the last-listener timeout.
// Per-(interface, group) state is one of No Listeners Present, Listeners Present,
// or Checking Listeners.
//
// When `enabled` is false the module is inert: it sends no Report or Done on
// group join/leave, runs no querier, and drops any inbound MLD message (it stays
// instantiated and registered, but performs no MLD protocol activity).
//
// Querier election, source filtering (SSM), MLD snooping, and MLDv2 are not
// implemented.
//
// @see ~IMld, ~Igmpv2, ~MldMessage, Ipv6InterfaceData
//
simple Mldv1 extends SimpleModule like IMld
{
    parameters:
        @class(Mldv1);
        string interfaceTableModule;  // path to the InterfaceTable module
        string routingTableModule;    // path to the Ipv6RoutingTable module
        string checksumMode @enum("declared","computed") = default("declared");  // how the ICMPv6 checksum is handled
        bool enabled = default(true);  // when false, MLD is inert on this node (see above)
        double unsolicitedReportInterval @unit(s) = default(10s);  // host: delay before the repeated Report after a join
        int robustnessVariable = default(2);  // loss tolerance; only supplies the defaults of the three params marked (RV) below
        double queryInterval @unit(s) = default(125s);  // querier: interval between periodic General Queries
        double queryResponseInterval @unit(s) = default(10s);  // querier: max delay a host may wait before answering a General Query
        double multicastListenerInterval @unit(s) = default((robustnessVariable * queryInterval) + queryResponseInterval);  // querier: how long a group stays "has listeners" without a new Report (RV)
        double startupQueryInterval @unit(s) = default(queryInterval / 4);  // querier: interval between the initial General Queries at startup
        int startupQueryCount = default(robustnessVariable);  // querier: number of General Queries sent at startup (RV)
        double lastListenerQueryInterval @unit(s) = default(1s);  // querier: interval between the group-specific Queries sent after a Done
        int lastListenerQueryCount = default(robustnessVariable);  // querier: number of group-specific Queries sent after a Done (RV)
        @display("i=block/cogwheel");

    gates:
        input ipIn @labels(Ipv6ControlInfo/up);
        output ipOut @labels(Ipv6ControlInfo/down);
}
