//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.srmpls;

import inet.common.SimpleModule;
import inet.networklayer.mpls.IIngressClassifier;

//
// SR-policy ingress classifier (RFC 8660): generalizes ~StaticIngressClassifier's
// single-label push to a full, ordered segment-list label stack.
//
// Where ~StaticIngressClassifier's static FEC table maps a destination prefix
// straight to one label, ~SrPolicy's `policies` XML table maps a destination
// prefix to an ORDERED list of segments -- each either a node SID (identified
// by its network-wide sid index, resolved against the sibling
// ~SegmentRouting module's sidTable) or an adjacency SID (identified by a
// LOCAL interface name, resolved against that same sibling's dynamically
// allocated adjacency-SID table, see ~SegmentRouting::getAdjSidLabel()).
//
// The `policies` XML format: the root element is <policies>, containing any
// number of <policy> elements, each a leaf element with:
// - dest: destination prefix's network address (e.g. "10.3.3.0")
// - netmask: the prefix's netmask (e.g. "255.255.255.0")
// - segments: a whitespace-separated, ORDERED list of segment tokens, each
//   either "node:<sidIndex>" or "adj:<interfaceName>"
//
// <pre>
// <policies>
//     <policy dest="10.3.3.0" netmask="255.255.255.0" segments="node:3 node:5"/>
//     <policy dest="10.9.9.0" netmask="255.255.255.0" segments="node:3 adj:ppp2"/>
// </policies>
// </pre>
//
// On a lookup, the entry with the longest matching netmask wins (standard
// longest-prefix match, like ~StaticIngressClassifier); if no entry matches,
// this classifier reports "no mapping" and ~Mpls falls back to regular L3
// routing for that packet.
//
// Segment-list convention: segments are listed LEFT-TO-RIGHT in the SAME
// order the network processes them, i.e. segments[0] is the OUTERMOST label
// (the active segment, on top of the stack, the one the very first LSR
// downstream looks at) and the last listed segment is the INNERMOST/bottom
// one. This matches RFC 8660's Segment List (SL) convention (SL[0] is the
// active segment). Concretely, for `segments="node:3 node:5"` the pushed
// stack (top to bottom) is [label(node:3), label(node:5)] -- note this is
// the OPPOSITE of the order LibTable::pushLabel() ops are appended to an
// outLabel vector consumed by ~Mpls::doStackOps() (each PUSH op inserts at
// the front of the packet's label stack, so the LAST op processed ends up on
// top): ~SrPolicy therefore builds its PUSH ops in REVERSE segment-list
// order, so segments[0]'s label is pushed LAST and ends up outermost.
//
// First-segment canonicalization: the outgoing interface is always resolved
// from segments[0] (via ~SegmentRouting::resolveNextHop() for a "node:"
// segment, or directly for an "adj:" one), and that segment's label is
// DROPPED from the pushed stack -- never pushed at all -- whenever pushing
// it would be pointless:
// - "node:" segments: if segments[0]'s next hop from THIS router already IS
//   that segment's own node (this router is the penultimate hop), the very
//   next router would apply PHP and expects the packet already popped
//   (~SegmentRouting installs no LIB entry for its OWN node SID, for exactly
//   this reason -- see ~SegmentRouting.cc).
// - "adj:" segments: ALWAYS dropped when first, because (see Limitations
//   below) an "adj:" segment always refers to one of THIS router's OWN
//   local links -- forwarding out that interface already IS "crossing" it;
//   the corresponding LIB entry lives on this SAME router and ingress-
//   classified traffic never consults the LIB at all (~Mpls bypasses
//   LibTable for locally-imposed stacks), so pushing it would leave an
//   unpoppable label on the wire.
// If dropping segments[0]'s label leaves NO further segments (a single-
// segment policy whose one segment was directly connected/local), this
// classifier reports "no mapping" (~Mpls falls back to regular L3 routing)
// rather than imposing an empty label stack, which ~Mpls's non-empty-
// outLabel invariant forbids.
//
// Limitations (deliberate simplifications of this model, see F1's plan):
// - "adj:<interfaceName>" ALWAYS resolves against the CO-LOCATED
//   ~SegmentRouting module's OWN local adjacency-SID table -- i.e. it can
//   only describe one of THIS router's own links. There is no adjacency-SID
//   advertisement/flooding modeled (real deployments flood these via
//   OSPF-SR/IS-IS-SR extensions), so a downstream router's adjacency
//   SIDs are not knowable here. Consequently "adj:" only has well-defined,
//   supported semantics as a policy's FIRST (or sole) segment; using it at
//   a later position resolves to some label THIS router happens to have
//   allocated for one of ITS OWN links, which is essentially always the
//   WRONG label for whatever router actually receives it at that point in
//   the path, and will very likely make that router blackhole the packet
//   (a "fail safe" outcome: the packet is dropped, not misrouted). Keep
//   cross-node adjacency-SID steering out of policies; use node-SIDs for
//   multi-hop explicit paths instead (see F1's "explicit path" example).
// - if a matched policy's first segment is currently unreachable (a "node:"
//   target with no path) or a "later" adjacency segment is currently
//   withdrawn (its link is down), this classifier reports "no mapping",
//   which ~Mpls resolves via a fallback to regular L3 routing -- NOT a
//   drop. Distinguishing "policy explicitly wants this traffic dropped when
//   its steered path is unavailable" from "no policy applies, use normal
//   routing" would need either an ~IIngressClassifier contract extension or
//   an explicit drop path in ~Mpls, which is out of scope for this module;
//   this is a known, documented gap versus true SR-TE blackholing behavior.
//
simple SrPolicy extends SimpleModule like IIngressClassifier
{
    parameters:
        @class(SrPolicy);
        string interfaceTableModule;   // The path to the InterfaceTable module; used to resolve "adj:<interfaceName>" segments to interface ids
        string segmentRoutingModule;   // The path to the sibling ~SegmentRouting module this classifier resolves "node:"/"adj:" segments through
        xml policies = default(xml("<policies/>"));
        @display("i=block/classifier");
}
