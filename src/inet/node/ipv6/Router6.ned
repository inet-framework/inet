//
// Copyright (C) 2000 Institut für Telematik, Universität Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.ipv6;

import inet.node.inet.Router;

//
// An IPv6 router for next-generation Internet Protocol networks. It
// extends the standard Router module by configuring it to use IPv6 as the
// network layer protocol while disabling IPv4 by default.
//
// @see ~StandardHost6
//
module Router6 extends Router
{
    parameters:
        hasIpv4 = default(false);
        hasIpv6 = default(true);
        xml routes = default(xml("<routingTable/>"));
        ipv6.routingTable.routes = this.routes;
        *.routingTableModule = "^.ipv6.routingTable";
}

