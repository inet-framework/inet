//
// Copyright (C) 2005 Andras Babos
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.ospfv3;

import inet.node.inet.Router;


//
// A router that uses the Open Shortest Path First version 3 (OSPFv3) routing
// protocol. It extends the standard Router module with OSPFv3 functionality and
// IPv6 support.
//
// OSPFv3 is the IPv6 adaptation of the OSPF routing protocol. While it maintains
// the same basic mechanisms and algorithms as OSPFv2, it has been modified to
// support IPv6 addressing and features. OSPFv3 operates independently from IPv6's
// routing extension headers.
//
// @see ~BgpRouter, ~OspfRouter
//
module Ospfv3Router extends Router
{
    parameters:
        hasIpv6 = true;
        *.routingTableModule6 = "^.ipv6.routingTable";
        hasOspf = true;
        ospf.typename = "Ospfv3";
}

