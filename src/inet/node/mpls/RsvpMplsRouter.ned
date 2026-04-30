//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.mpls;

import inet.networklayer.mpls.IIngressClassifier;
import inet.networklayer.rsvpte.RsvpTe;


//
// An RSVP-TE capable MPLS router.
//
// ~RsvpTe occupies the Transport layer; however, it is not a transport protocol
// itself. ~RsvpTe uses transport protocols to route packets. ~Ted is used
// to calculate shortest paths.
// Extends ~MplsRouterBase with the RSVP-TE signaling protocol.
//
// @see ~MplsRouterBase, ~LdpMplsRouter
//
module RsvpMplsRouter extends MplsRouterBase
{
    parameters:
        @labels(node,mpls-node);
        *.rsvpModule = default(absPath(".rsvp"));
        mpls.classifierModule = default(absPath(".classifier"));

    submodules:
        rsvp: RsvpTe {
            parameters:
                peers = parent.peers;
                classifierModule = "^.classifier";
                @display("p=300,80");
        }
        classifier: <default("RsvpClassifier")> like IIngressClassifier {
            parameters:
                @display("p=100,100;is=s");
        }

    connections:
        rsvp.ipOut --> tn.in++;
        rsvp.ipIn <-- tn.out++;
}

