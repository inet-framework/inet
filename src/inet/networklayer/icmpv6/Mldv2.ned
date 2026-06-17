//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.icmpv6;

import inet.common.SimpleModule;

//
// Implements MLDv2 (RFC 3810): Multicast Listener Discovery version 2 for IPv6 --
// the IPv6 equivalent of ~Igmpv3, and the source-filtering successor to ~Mldv1.
// Beyond plain group membership, MLDv2 lets a host request a group's traffic
// while INCLUDING or EXCLUDING a specific set of sources -- the basis for
// Source-Specific Multicast (SSM). A single module performs both the host
// listener role and the multicast-router (querier) role; the router role is
// active only on multicast routers (i.e. when IPv6 multicast forwarding is
// enabled on the node).
//
// MLDv2 messages (ICMPv6 types 130/143, see ~Mldv2Query and ~Mldv2Report) are
// carried as ICMPv6 and exchanged with the IPv6 layer through the `ipIn`/`ipOut`
// gates, which attach to the `lp` MessageDispatcher in ~Ipv6NetworkLayer. On the
// host side, membership and source filters follow the interface's multicast
// socket state (Ipv6InterfaceData), delivered to this module through the
// `ipv6MulticastChange` notification.
//
// <b>Host behavior</b> (RFC 3810 Section 5/6)
//
// For each (interface, group) the host keeps a filter mode (INCLUDE or EXCLUDE)
// and a source list. When the application changes its membership or filter, the
// host sends a State-Change Report carrying the relevant Multicast Address
// Records (ALLOW_NEW_SOURCES, BLOCK_OLD_SOURCES, CHANGE_TO_INCLUDE_MODE,
// CHANGE_TO_EXCLUDE_MODE). In response to a General, Multicast-Address-Specific,
// or Multicast-Address-and-Source-Specific Query it schedules a Current-State
// Report (MODE_IS_INCLUDE / MODE_IS_EXCLUDE) after a random delay bounded by the
// Query's Maximum Response Code. Reports are sent to the all-MLDv2-routers
// address (ff02::16); General Queries go to the all-nodes address (ff02::1). The
// link-scope all-nodes group (ff02::1) is never reported.
//
// <b>Router (querier) behavior</b> (RFC 3810 Section 7)
//
// A querier sends periodic General Queries (every `queryInterval`) and, when a
// group or source is leaving, Multicast-Address-Specific and
// Multicast-Address-and-Source-Specific Queries (`lastMemberQueryCount` of them,
// `lastMemberQueryInterval` apart). Per (interface, group) it keeps a filter
// mode, a group timer, and per-source timers; together these determine which
// sources are forwarded onto the link (pushed into Ipv6InterfaceData via
// setMulticastListeners). Where several routers share a link, the one with the
// lowest IP address is the querier; another router takes over only if it hears no
// General Query for `otherQuerierPresentInterval`.
//
// When `enabled` is false the module is inert: it sends nothing and runs no
// querier (it stays instantiated and registered, but performs no MLD activity).
//
// <b>Parity gaps</b> (matching the IPv4 ~Igmpv3 this module is translated from):
// state-change Reports are sent once, not retransmitted [Robustness Variable]-1
// more times; Multicast-Address-Specific Queries after a leave are not
// retransmitted `lastMemberQueryCount` times; and MLDv1<->MLDv2 interoperation
// (parsing/answering older-version messages) is not implemented -- unexpected
// message versions are logged and dropped.
//
// @see ~IMld, ~Mldv1, ~Igmpv3, ~Mldv2Query, ~Mldv2Report, Ipv6InterfaceData
//
simple Mldv2 extends SimpleModule like IMld
{
    parameters:
        @class(Mldv2);
        bool enabled = default(true);  // when false the module is inert (sends nothing, runs no querier)
        string interfaceTableModule;   // path to the InterfaceTable module
        string routingTableModule;     // path to the Ipv6RoutingTable module
        string checksumMode @enum("declared","computed") = default("declared");  // how the ICMPv6 checksum is handled
        int    robustnessVariable = default(2);  // loss tolerance; only supplies the defaults of the params marked (RV) below
        double queryInterval @unit(s) = default(125s);  // querier: interval between periodic General Queries
        double queryResponseInterval @unit(s) = default(10s);  // querier: max delay a host may wait before answering a General Query
        double groupMembershipInterval @unit(s) = default((robustnessVariable * queryInterval) + queryResponseInterval);  // querier: how long a group stays active without a new Report (RV)
        double otherQuerierPresentInterval @unit(s) = default((robustnessVariable * queryInterval) + (queryResponseInterval / 2));  // querier: wait this long for another querier before taking over (RV)
        double startupQueryInterval @unit(s) = default(queryInterval / 4);  // querier: interval between the initial General Queries at startup
        int    startupQueryCount = default(robustnessVariable);  // querier: number of General Queries sent at startup (RV)
        double lastMemberQueryInterval @unit(s) = default(1s);  // querier: interval between Multicast-Address-/-and-Source-Specific Queries after a leave
        int    lastMemberQueryCount = default(robustnessVariable);  // querier: number of such Queries sent after a leave (RV)
        double unsolicitedReportInterval @unit(s) = default(10s);  // host: delay between repeated State-Change Reports
        @display("i=block/cogwheel");
        @selfMessageKinds(inet::Mldv2TimerKind);

    gates:
        input ipIn @labels(Ipv6ControlInfo/up);   // delivered MLD (ICMPv6) packets
        output ipOut @labels(Ipv6ControlInfo/down); // to ~Ipv6
}
