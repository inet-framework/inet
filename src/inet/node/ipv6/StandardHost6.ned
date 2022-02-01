//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.ipv6;

import inet.node.inet.StandardHost;

//
// IPv6 host with TCP, SCTP and UDP layers and applications.
// see ~StandardHost for configuration.
//
module StandardHost6 extends StandardHost
{
    parameters:
        @display("i=device/pc2");
        xml routes = default(xml("<routingTable/>"));
        ipv6.routingTable.routes = this.routes;
        hasIpv4 = default(false);
        hasIpv6 = default(true);
        *.routingTableModule = "^.ipv6.routingTable";
}

