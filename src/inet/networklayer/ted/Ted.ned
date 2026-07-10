//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ted;

import inet.common.SimpleModule;

//
// Traffic Engineering Database. Stores network topology with detailed
// link information, including total and reserved bandwidths.
//
// ~Ted is filled in and kept up-to-date via link state routing protocols,
// like ~LinkStateRouting or (in the future) OSPF-TE.
//
// Ted exposes several public methods for querying the current network state
// and allows ~RsvpTe and individual applications to calculate feasible LSPs
// meeting the chosen bandwidth criteria.
//
// Known constraints of the current implementation:
// - Topology discovery (at initialization) walks the gate chain to each
//   directly connected node; anything behind a switch, hub, access point, or
//   other non-routing device is invisible to this module (no TED entry is
//   created for that link).
// - Every interface participating in the TED must have a network-unique
//   IPv4 address; if it does not, initialization fails with a
//   cRuntimeError (MPLS/LDP/RSVP-TE assume unique interface addresses
//   throughout).
//
// Routing-table ownership (~installRoutes~):
// - Default (~installRoutes~ = true, today's behavior): Ted unconditionally
//   owns the node's IPv4 routing table. Whenever the link-state database
//   changes, it deletes every non-multicast route and reinstalls a full set
//   of `/32` host routes derived from the TED, sourced as IRoute::OSPF. Do
//   not combine this mode with another source of routes beyond interface
//   addressing (e.g. the configurator's static routes, or another routing
//   protocol such as ~Ospfv2) in the same router -- Ted will wipe them out
//   on the next rebuild.
// - ~installRoutes~ = false: Ted never touches the routing table; it only
//   tracks link-state/TE information for CAC and signaling purposes (LDP
//   FEC binding and RSVP-TE loose-hop resolution still work off whatever
//   routes are present in the table -- installed by an IGP, static `.rt`
//   files, or ~Ipv4NetworkConfigurator~ -- since LDP/RSVP only ever read
//   the routing table, they don't care who populated it). Use this mode to
//   let Ted coexist with another routing protocol or with
//   configurator-assigned routes.
//
simple Ted extends SimpleModule
{
    parameters:
        @class(Ted);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;  // The path to the RoutingTable module
        bool installRoutes = default(true); // Whether Ted installs/maintains host routes in the routing table from its link-state database (see class documentation for the coexistence story)
        xml linkAttributes = default(xml("<links/>")); // optional per-interface TE-attribute overrides, e.g. <links><link interface="eth0" teMetric="10" adminGroup="0x3" srlgs="7 12"/></links>; unlisted interfaces get teMetric=0 (falls back to `metric`) and adminGroup=0/no SRLGs
        @display("i=block/table");
        @signal[tedChanged](type=inet::TedChangeInfo); // emitted by setLinkState()/announceLinkChange() whenever a link's liveness or bandwidth changes; ~LinkStateRouting subscribes at the host level to flood the change
}
