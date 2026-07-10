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
// - Ted unconditionally owns the node's IPv4 routing table: whenever the
//   link-state database changes, it deletes every non-multicast route and
//   reinstalls a full set of host routes derived from the TED. There is
//   currently no option to let another routing protocol (e.g., ~Ospfv2)
//   coexist with an active ~Ted in the same router.
//
simple Ted extends SimpleModule
{
    parameters:
        @class(Ted);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;  // The path to the RoutingTable module
        @display("i=block/table");
}
