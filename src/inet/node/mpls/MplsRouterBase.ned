//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.mpls;

import inet.common.MessageDispatcher;
import inet.networklayer.mpls.LibTable;
import inet.networklayer.mpls.Mpls;
import inet.networklayer.ted.LinkStateRouting;
import inet.networklayer.ted.Ted;
import inet.node.inet.Router;

//
// Base module for MPLS-capable routers. Extends the standard Router with the
// MPLS data plane (Mpls, LibTable) and the Traffic Engineering Database (Ted,
// LinkStateRouting) common to all MPLS signaling protocols.
//
// The MPLS layer is inserted between the network-layer dispatcher (nl) and the
// link-layer bridge dispatcher (cb) using @reconnect, so that all IP↔interface
// traffic flows through the MPLS label-switching engine.
//
// Derived modules add only the signaling-protocol-specific submodules and
// connections (e.g. LdpMplsRouter adds Ldp, RsvpMplsRouter adds RsvpTe).
//
// @see ~LdpMplsRouter, ~RsvpMplsRouter
//
module MplsRouterBase extends Router
{
    parameters:
        @display("i=abstract/router");
        string peers = default("auto"); // RSVP peers; "auto" derives them from the TED (see ~RsvpTe)
        string routerId = default("auto");
        *.routingTable.routerId = this.routerId;
        *.tedModule = default(absPath(".ted"));
        *.libTableModule = default(absPath(".libTable"));

    submodules:
        ted: Ted {
            @display("p=100,400;is=s");
        }
        linkStateRouting: LinkStateRouting {
            peers = parent.peers;
            @display("p=900,76");
        }
        mpls: Mpls {
            @display("p=500,480");
        }
        libTable: LibTable {
            @display("p=100,200;is=s");
        }
        ml: MessageDispatcher {
            @display("p=700,560;b=1000,5,,,,1");
        }

    connections:
        // Insert MPLS between nl (network layer dispatcher) and cb (link-layer bridge).
        //
        // In NetworkLayerNodeBase, the cb↔nl wires are always at index 0:
        //   nl.in[0]  ← cb.out[1]   (upward: link → IP)
        //   nl.out[0] → cb.in[1]    (downward: IP → link)
        //
        // In the standard MPLS router case (no llc, no bridging):
        //   cb.out[0] → bl (LinkLayerNodeBase)
        //   cb.out[1] → nl.in[0]  ← intercept here
        //   cb.in[1]  ← nl.out[0] ← intercept here

        // Wire A (upward: link → MPLS → nl):
        //   Old: cb.out[1] → nl.in[0]
        //   New: cb.out[1] → ml → mpls.upperLayerIn → ... → nl.in[0]
        mpls.upperLayerOut --> { @reconnect; } --> nl.in[0];  // reconnect nl.in[0] source from mpls
        cb.out[1] --> ml.in++;                                 // cb.out[1] is now dangling

        // Wire B (downward: nl → MPLS → link):
        //   Old: nl.out[0] → cb.in[1]
        //   New: nl.out[0] → mpls.upperLayerIn → ... → ml → cb.in[1]
        nl.out[0] --> { @reconnect; } --> mpls.upperLayerIn;  // reconnect nl.out[0] dest to mpls
        ml.out++ --> cb.in[1];                                 // cb.in[1] is now dangling

        // MPLS ↔ ml (new connections, not reconnects):
        mpls.lowerLayerOut --> ml.in++;
        ml.out++ --> mpls.lowerLayerIn;

        // LinkStateRouting ↔ transport dispatcher:
        linkStateRouting.ipOut --> tn.in++;
        tn.out++ --> linkStateRouting.ipIn;
}

