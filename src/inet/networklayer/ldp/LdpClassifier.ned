//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ldp;

import inet.common.SimpleModule;
import inet.networklayer.mpls.IIngressClassifier;

//
// Ingress classifier for LDP-signaled label-switched paths.
//
// Fallback design note: rather than maintaining its own bind-time FEC/label
// table (the way ~RsvpClassifier does for RSVP-TE), this module delegates
// classification to the sibling ~Ldp module's classifyPacket() method by a
// direct C++ call. ~Ldp's classification decision is evaluated lazily,
// per packet, against the live fecList/fecDown state -- including a
// longest-prefix-match rule where the FIRST matching (longest) FEC entry
// wins the decision even when that entry currently has no downstream label
// mapping (in which case the packet falls back to regular L3 routing
// without ever considering a shorter-prefix FEC that might have a mapping).
// Faithfully re-deriving this ordering/tie-breaking behavior in an eagerly
// maintained, separately-owned table would risk subtle divergences from
// today's per-packet lookup, so the state split is deferred to a future
// workstream; this module only decouples the C++ interface and the router
// NED wiring (see D5 in the MPLS/LDP/RSVP-TE architecture-decoupling plan).
//
simple LdpClassifier extends SimpleModule like IIngressClassifier
{
    parameters:
        @class(LdpClassifier);
        string ldpModule; // path to the sibling ~Ldp module this classifier delegates to
        @display("i=block/classifier");
}
