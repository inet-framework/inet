//
// Copyright (C) 2011 CoCo Communications
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4;

import inet.common.SimpleModule;

//
// Implements IGMPv3 (RFC 3376), the source-filtering version of the Internet
// Group Management Protocol for IPv4. Beyond plain group membership (see
// ~Igmpv2), IGMPv3 lets a host request a group's traffic while INCLUDING or
// EXCLUDING a specific set of sources -- the basis for Source-Specific
// Multicast. A single module performs both the host and the multicast-router
// (querier) role.
//
// IGMP messages (see ~IgmpMessage) are exchanged with the IPv4 layer through the
// `ipIn`/`ipOut` gates. A multicast routing protocol that also uses IGMP can be
// attached to `routerIn`/`routerOut`. On the host side, membership and source
// filters follow the interface's multicast socket state (Ipv4InterfaceData),
// delivered to this module through the `ipv4MulticastChange` notification.
//
// <b>Host behavior</b>
//
// For each (interface, group) the host keeps a filter mode (INCLUDE or EXCLUDE)
// and a source list. When the application changes its membership or filter, the
// host sends a State-Change Report carrying the relevant group records
// (ALLOW_NEW_SOURCES, BLOCK_OLD_SOURCES, CHANGE_TO_INCLUDE/EXCLUDE_MODE). In
// response to a General, Group-Specific, or Group-and-Source-Specific Query it
// schedules a Current-State Report (MODE_IS_INCLUDE / MODE_IS_EXCLUDE) after a
// random delay bounded by the query's Maximum Response Time. Unlike IGMPv2,
// IGMPv3 has no report suppression; reports are sent to the all-IGMPv3-routers
// address (224.0.0.22).
//
// <b>Router (querier) behavior</b>
//
// A querier sends periodic General Queries (every `queryInterval`) and, when a
// group or source is leaving, Group-Specific and Group-and-Source-Specific
// Queries (`lastMemberQueryCount` of them, `lastMemberQueryInterval` apart). Per
// (interface, group) it keeps a filter mode, a group timer, and per-source
// timers; together these determine which sources are forwarded onto the link.
// Where several routers share a link, the one with the lowest IP address is the
// querier; another router takes over only if it hears no General Query for
// `otherQuerierPresentInterval`.
//
// When `enabled` is false the module is inert.
//
// @see ~Igmpv2, ~IIgmp, ~IgmpMessage
//
simple Igmpv3 extends SimpleModule like IIgmp
{
    parameters:
        @class(Igmpv3);
        bool enabled = default(true);  // when false the module is inert (sends nothing, runs no querier)
        string interfaceTableModule;   // path to the InterfaceTable module
        string routingTableModule;     // path to the IIpv4RoutingTable module
        string checksumMode @enum("declared","computed") = default("declared");  // how the IGMP checksum is handled
        int    robustnessVariable = default(2);  // loss tolerance; only supplies the defaults of the params marked (RV) below
        double queryInterval @unit(s) = default(125s);  // querier: interval between periodic General Queries
        double queryResponseInterval @unit(s) = default(10s);  // querier: max delay a host may wait before answering a General Query
        double groupMembershipInterval @unit(s) = default((robustnessVariable * queryInterval) + queryResponseInterval);  // querier: how long a group stays active without a new Report (RV)
        double otherQuerierPresentInterval @unit(s) = default((robustnessVariable * queryInterval) + (queryResponseInterval / 2));  // querier: wait this long for another querier before taking over (RV)
        double startupQueryInterval @unit(s) = default(queryInterval / 4);  // querier: interval between the initial General Queries at startup
        int    startupQueryCount = default(robustnessVariable);  // querier: number of General Queries sent at startup (RV)
        double lastMemberQueryInterval @unit(s) = default(1s);  // querier: interval between Group-/Group-and-Source-Specific Queries after a leave
        int    lastMemberQueryCount = default(robustnessVariable);  // querier: number of such Queries sent after a leave (RV)
        double unsolicitedReportInterval @unit(s) = default(10s);  // host: delay between repeated State-Change Reports
        //double version1RouterPresentInterval @unit(s) = default(400s);
        @display("i=block/cogwheel");
        @selfMessageKinds(inet::IgmpTimerKind);

    gates:
        input ipIn @labels(Ipv4ControlInfo/up);  // Delivered IGMP packets
        output ipOut @labels(Ipv4ControlInfo/down); // To ~Ipv4

        input routerIn @labels(Ipv4ControlInfo/up);
        output routerOut @labels(Ipv4ControlInfo/down);
}

