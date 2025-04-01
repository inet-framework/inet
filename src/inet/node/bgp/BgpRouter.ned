//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.bgp;

import inet.node.inet.Router;

//
// A router that supports Border Gateway Protocol version 4 (BGPv4) along with Open Shortest
// Path First version 4 (OSPFv4). It extends the standard Router module with BGP and OSPF
// functionality enabled.
//
// BGP is the core routing protocol of the Internet that maintains paths to other autonomous
// systems. It makes routing decisions based on network policies, rule-sets, and path attributes
// rather than purely technical metrics. BGP is designed to exchange routing and reachability
// information among autonomous systems on the Internet.
//
// The combination of BGP for external routing and OSPF for internal routing is a common
// configuration in real-world networks, particularly for Internet Service Providers and
// large enterprise networks.
//
// @see ~OspfRouter
//
module BgpRouter extends Router
{
    parameters:
        hasBgp = true;
        hasOspf = true;
}
