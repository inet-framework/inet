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
// Multicast-Address-Specific Queries (`lastListenerQueryCount` of them, spaced
// `lastListenerQueryInterval` apart) and removes the group if no Report arrives
// before the last-listener timeout. Per-(interface, group) state is one of No
// Listeners Present, Listeners Present, or Checking Listeners.
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
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string checksumMode @enum("declared","computed") = default("declared");
        bool enabled = default(true);
        // Test-only: when true, schedules a single MLD Report send at t=0.5s
        // so the MLD_smoke.test can verify end-to-end dispatcher flow without
        // a real state machine. MUST remain default(false) so it is inert for
        // all production scenarios and other tests.
        bool sendTestMessage = default(false);
        // Test-only: when true, simulates leaving all joined groups at t=5s by calling
        // multicastGroupLeft() directly. This bypasses the UDP leaveMulticastGroup path
        // (which is not implemented for IPv6 in NetworkInterface::changeMulticastGroupMembership).
        // Used in MLD_host_basic.test to observe the HOST-04 (Done to ff02::2) behavior.
        // MUST remain default(false).
        bool sendTestLeave = default(false);
        // Test-only: if non-empty, Mldv1 directly calls multicastGroupJoined() for this
        // IPv6 multicast address on every multicast-capable interface during INITSTAGE_NETWORK_LAYER.
        // This bypasses the UDP addMulticastAddressToInterface path (which uses assignAddress
        // for IPv6 instead of joinMulticastGroup and thus does not fire ipv6MulticastGroupJoinedSignal).
        // Used in MLD_host_basic.test to join a group and observe HOST-01/02/03/04 behavior.
        // MUST remain default("") (empty = inactive) so it is inert for production scenarios.
        string testMulticastGroup = default("");
        // RFC 2710 §7.2 Unsolicited Report Interval: time between unsolicited
        // Multicast Listener Reports sent on join (default 10s per RFC 2710).
        double unsolicitedReportInterval @unit(s) = default(10s);
        // RFC 2710 §7.3 Robustness Variable: tunable for expected packet loss.
        int robustnessVariable = default(2);
        // RFC 2710 §7.4 Query Interval: time between periodic General Queries.
        double queryInterval @unit(s) = default(125s);
        // RFC 2710 §7.5 Query Response Interval: max response delay for General Queries (also used as maxRespDelay).
        double queryResponseInterval @unit(s) = default(10s);
        // RFC 2710 §7.1 Multicast Listener Interval: duration a router keeps state without a Report.
        double multicastListenerInterval @unit(s) = default((robustnessVariable * queryInterval) + queryResponseInterval);
        // RFC 2710 §7.6.1 Startup Query Interval: interval between startup queries.
        double startupQueryInterval @unit(s) = default(queryInterval / 4);
        // RFC 2710 §7.6.2 Startup Query Count: number of startup General Queries.
        int startupQueryCount = default(robustnessVariable);
        // RFC 2710 §7.8 Last Listener Query Interval: time between MAS Queries on Done.
        double lastListenerQueryInterval @unit(s) = default(1s);
        // RFC 2710 §7.9 Last Listener Query Count: number of MAS Queries on Done.
        int lastListenerQueryCount = default(robustnessVariable);
        @display("i=block/cogwheel");

    gates:
        input ipIn @labels(Ipv6ControlInfo/up);
        output ipOut @labels(Ipv6ControlInfo/down);
}
