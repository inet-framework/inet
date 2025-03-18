//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// @author Jan Zavrel (honza.zavrel96@gmail.com)
// @author Jan Bloudicek (jbloudicek@gmail.com)
// @author Vit Rek (rek@kn.vutbr.cz)
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

//TODO review
package inet.routing.eigrp.tables;

import inet.common.SimpleModule;

//
// Maintains the IPv4-specific EIGRP topology table, which stores all routes
// learned from neighbors. Contains both feasible successors (routes used for
// forwarding) and non-feasible successors (potential backup routes). Implements
// the DUAL (Diffusing Update Algorithm) for loop-free path selection and
// convergence.
//
simple EigrpIpv4TopologyTable extends SimpleModule
{
    parameters:
        @class(EigrpIpv4TopologyTable);
    @display("i=block/table");
}
