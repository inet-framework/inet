//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.ipv4;

import inet.common.SimpleModule;

//
// Records changes in the routing tables (~Ipv4RoutingTable) and interface tables
// (~InterfaceTable) of all hosts and routers. The filename has to be specified
// in the routinglog-file configuration option that this module registers.
//
simple RoutingTableRecorder extends SimpleModule
{
    parameters:
        @class(RoutingTableRecorder);
        bool enabled = default(true);
        @display("i=block/control");
}

