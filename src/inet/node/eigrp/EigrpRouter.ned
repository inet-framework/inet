//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
// Copyright (C) 2005 Andras Babos
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//               2010 Zoltan Bojthe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// @author Jan Bloudicek
// @author Jan Zavrel
// @author Vit Rek
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

package inet.node.eigrp;

import inet.node.inet.Router;

//
// A router that uses the Enhanced Interior Gateway Routing Protocol (EIGRP). It
// extends the standard Router module with EIGRP functionality and dual-stack
// (IPv4/IPv6) support.
//
// EIGRP is an advanced distance-vector routing protocol developed by Cisco Systems.
// It combines features of both distance-vector and link-state routing protocols,
// providing fast convergence, loop-free operation, and efficient use of bandwidth
// and processing resources.
//
// @see ~OspfRouter, ~RipRouter
//
module EigrpRouter extends Router
{
    parameters:
        hasEigrp = true;
        hasIpv6 = default(true);
        *.routingTableModule = default("^.^.ipv4.routingTable");
        *.routingTableModule6 = default("^.^.ipv6.routingTable");
}
