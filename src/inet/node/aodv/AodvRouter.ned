//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.aodv;

import inet.node.inet.AdhocHost;
import inet.routing.aodv.Aodv;

//
// It models a ~WirelessHost extended with ~Aodv submodule.
//
// AODV is an on-demand MANET routing protocol. Each node
// works similar to a 'router': they manage their own routing
// tables, send request for routes and consult the IP layer via
// Netfilter hooks for data forwarding.
//
// Scenerios where AODV routing is desired use this module as a
// host module.
//
// The AODV v.2 (known as Dymo) is also implemented: ~Dymo,
// ~DymoRouter.
//
// @see ~Aodv, ~Dymo, ~DymoRouter.
//
module AodvRouter extends AdhocHost
{
    submodules:
        aodv: Aodv {
            @display("p=825,226");
        }
    connections:
        aodv.socketOut --> at.in++;
        aodv.socketIn <-- at.out++;
}

