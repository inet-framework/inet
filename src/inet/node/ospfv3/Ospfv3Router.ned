//
// Copyright (C) 2005 Andras Babos
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.ospfv3;

import inet.node.inet.Router;


//
// An OSPFv3 router.
//
module Ospfv3Router extends Router
{
    parameters:
        hasIpv6 = true;
        *.routingTableModule6 = "^.ipv6.routingTable";
        hasOspf = true;
        ospf.typename = "Ospfv3";
}

