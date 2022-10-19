//
// Copyright (C) 2011 CoCo Communications
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4;

//
// Imlementation of IGMPv2 protocol. Multicast routers use IGMP
// to learn which groups have members on each of their attached
// physical networks.
//
// Hosts and routers are communicating with each other by sending
// IGMP messages (see ~IgmpMessage). The IGMP messages are
// encapsulated and transported by the IP module connected to
// the 'ipOut' and 'ipIn' gates. If a multicast routing
// protocol module, which also uses IGMP messages (e.g. DVMRP),
// is connected to the 'routerOut' and 'routerIn' gates, it can
// send and receive IGMP messages through the IGMP module.
//
// This module implements both IGMPv2 host and router logic
// as specified in RFC 2236.
//
// <b>Host behaviour</b>
//
// When an interface joins to a multicast group, the host
// will send a Membership Report immediately to the group address.
// This report is repeated after 'unsolicetedReportInterval' to
// cover the possibility of the first report being lost.
//
// When a host's interface leaves a multicast group, and it was
// the last host that sent a Membership Report for that group,
// it will send a Leave Group message to the all-routers multicast
// group (224.0.0.2).
//
// This module also responds to IGMP Queries. When the host
// receives a Group-Specific Query on an interface that belongs
// to that group, then it will set a timer to a random value
// between 0 and Max Response Time of the Query. If the timer
// expires before the host observe a Membership Report sent
// by other hosts, then the host sends an IGMPv2 Membership Report.
// When the host receives a General Query on an interface,
// a timer is initialized and a report is sent for each group
// membership of the interface.
//
// <b>Router behaviour</b>
//
// Multicast routers maintains a list for each interface containing
// the multicast groups that have listeners on that interface.
// This list is updated when IGMP Membership Reports and Leave Group
// messages arrive, or when a timer expires since the last Query.
//
// When multiple routers are connected to the same link, the one with
// the smallest IP address will be the Querier. When other routers
// observe that they are Non-Queriers (by receiving an IGMP Query
// with a lower source address), they stop sending IGMP Queries
// until 'otherQuerierPresentInterval' elapsed since the last
// received query.
//
// Routers periodically ('queryInterval') send a General Query
// on each attached network for which this router is a Querier.
// On startup the router sends 'startupQueryCount' queries
// separated by 'startupQueryInterval'. A General Query
// has unspecified Group Address field, a Max Response Time
// field set to 'queryResponseInterval', and is sent to the
// all-systems multicast address (224.0.0.1).
//
// When a router receives a Membership Report, it will add the
// reported group to the list of multicast group memberships.
// At the same time it will set a timer for the membership
// to 'groupMembershipInterval'. Repeated reports restart
// the timer. If the timer expires, the router assumes
// that the group has no local members, and multicast traffic
// is no more forwarded to that interface.
//
// When a Querier receives a Leave Group message for a group,
// it sends a Group-Specific Query to the group being left.
// It repeats the Query 'lastMemberQueryCount' times in
// separated by 'lastMemberQueryInterval' until a Membership
// Report is received. If no Report received, then the router
// assumes that the group has no local members.
//
// <b>Disabling IGMP</b>
//
// The IPv4 ~Ipv4NetworkLayer contains an instance of the IGMP
// (~Igmpv2) module. IGMP can be turned off by setting the 'enabled'
// parameter to false. When disabled, then no IGMP message
// is generated, and incoming IGMP messages are ignored.
//
simple Igmpv2 like IIgmp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string crcMode @enum("declared","computed") = default("declared");
        bool enabled = default(true);
        int    robustnessVariable = default(2); // IGMP is roboust to (roboustnessVariable-1) packet loss
        double queryInterval @unit(s) = default(125s);
        double queryResponseInterval @unit(s) = default(10s);
        double groupMembershipInterval @unit(s) = default((robustnessVariable * queryInterval) + queryResponseInterval);
        double otherQuerierPresentInterval @unit(s) = default((robustnessVariable * queryInterval) + (queryResponseInterval / 2));
        double startupQueryInterval @unit(s) = default(queryInterval / 4);
        int    startupQueryCount = default(robustnessVariable);
        double lastMemberQueryInterval @unit(s) = default(1s);
        int    lastMemberQueryCount = default(robustnessVariable);
        double unsolicitedReportInterval @unit(s) = default(10s);
//      double version1RouterPresentInterval @unit(s) = default(400s);
        @display("i=block/cogwheel");
        @selfMessageKinds(inet::IgmpTimerKind);

    gates:
        input ipIn @labels(Ipv4ControlInfo/up);  // delivered IGMP packets
        output ipOut @labels(Ipv4ControlInfo/down); // to ~Ipv4

        input routerIn @labels(Ipv4ControlInfo/up);
        output routerOut @labels(Ipv4ControlInfo/down);
}

