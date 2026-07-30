//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.mpls;

import inet.networklayer.srmpls.SegmentRouting;
import inet.networklayer.srmpls.SrPolicy;

//
// A Segment Routing (SR-MPLS, RFC 8660) capable MPLS router.
//
// Extends ~MplsRouterBase with ~SegmentRouting (node-SID and adjacency-SID
// LIB programming, derived signaling-free from the ~Ted) as the label-
// programming "signaling protocol", and ~SrPolicy (segment-list label stack
// imposition) as the ingress classifier.
//
// @see ~MplsRouterBase, ~LdpMplsRouter, ~RsvpMplsRouter
//
module SrMplsRouter extends MplsRouterBase
{
    parameters:
        int srgbBase = default(16000);  // Segment Routing Global Block: first label of the block; see ~SegmentRouting
        int srgbSize = default(8000);   // Segment Routing Global Block: number of labels in the block; see ~SegmentRouting
        xml sidTable;                   // network-wide router-id -> node-SID index map; see ~SegmentRouting for the XML format; required, no default
        xml policies = default(xml("<policies/>")); // SR-policy table; see ~SrPolicy for the XML format
        mpls.classifierModule = default(absPath(".classifier"));
        // SR-MPLS has no Hello-based peer-liveness protocol of its own (unlike Ldp/RsvpTe, whose
        // Hello timeout is what normally calls Ted::setLinkState()); without this, a bare
        // physical link failure would be invisible to Ted and ~SegmentRouting would never
        // reroute. See ~Ted's "trackInterfaceState" parameter documentation.
        ted.trackInterfaceState = true;

    submodules:
        segmentRouting: SegmentRouting {
            parameters:
                srgbBase = parent.srgbBase;
                srgbSize = parent.srgbSize;
                sidTable = parent.sidTable;
                @display("p=700,180;is=s");
        }
        classifier: SrPolicy {
            parameters:
                segmentRoutingModule = "^.segmentRouting";
                policies = parent.policies;
                @display("p=500,180;is=s");
        }
}
