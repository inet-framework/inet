//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.mpls;

import inet.networklayer.ldp.Ldp;


//
// An LDP-capable MPLS router.
//
// LDP Capable Routers are the main building blocks for an LDP MPLS network.
// Extends ~MplsRouterBase with the LDP signaling protocol.
//
// @see ~MplsRouterBase, ~RsvpMplsRouter
//
module LdpMplsRouter extends MplsRouterBase
{
    parameters:
        double holdTime @unit(s) = default(15s);
        double helloInterval @unit(s) = default(5s);
        hasTcp = default(true);
        hasUdp = default(true);
        mpls.classifierModule = default(absPath(".ldp"));

    submodules:
        ldp: Ldp {
            parameters:
                holdTime = parent.holdTime;
                helloInterval = parent.helloInterval;
                @display("p=450,80");
        }

    connections:
        ldp.socketIn <-- at.out++;
        ldp.socketOut --> at.in++;
}

