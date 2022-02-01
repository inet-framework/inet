//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.ipv6;

import inet.node.inet.Router;

//
// IPv6 router.
//
// See ~Router for configuration options.
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

