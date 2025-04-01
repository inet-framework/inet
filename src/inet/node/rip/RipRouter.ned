//
// Copyright (C) 2005 Andras Babos
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.rip;

import inet.node.inet.Router;

//
// A router that uses the Routing Information Protocol (RIP)
// version 2. Extends the standard Router module with RIP functionality enabled.
//
// RIP is a distance-vector routing protocol that uses hop count as its metric.
// It prevents routing loops by implementing a limit on the number of hops allowed
// in a path from source to destination (maximum 15 hops). RIPv2 adds support for
// CIDR addressing and subnet masks.
//
// @see ~OspfRouter, ~BgpRouter
//
module RipRouter extends Router
{
    parameters:
        hasRip = true;
}

