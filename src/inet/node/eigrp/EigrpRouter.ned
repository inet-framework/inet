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

module EigrpRouter extends Router
{
    parameters:
        hasEigrp = true;
        hasIpv6 = default(true);
        *.routingTableModule = default("^.^.ipv4.routingTable");
        *.routingTableModule6 = default("^.^.ipv6.routingTable");
}

