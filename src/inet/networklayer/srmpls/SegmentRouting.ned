//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.srmpls;

import inet.common.SimpleModule;

//
// Segment Routing (RFC 8660) node-SID label programming for the MPLS data
// plane. ~SegmentRouting turns a network-wide, statically-configured map of
// router-id -> node-SID index (the `sidTable` parameter -- modeling the SID
// advertisements that a real deployment would flood via OSPF-SR/IS-IS-SR
// extensions, RFC 8665/8667) into ~LibTable entries, so that every other
// router's node SID resolves, network-wide, to the SAME MPLS label
// (`srgbBase + sidIndex`, the Segment Routing Global Block property that
// makes label stacks work without per-hop signaling).
//
// For every OTHER router N with configured SID index `sidIndex(N)`, this
// module computes the shortest path toward N over the ~Ted (via
// ~Ted::calculateShortestPath, the same Bellman-Ford CSPF machinery
// ~RsvpTe's ingress path computation uses) and installs one ~LibTable entry
// for label `srgbBase + sidIndex(N)`:
// - if N is NOT a direct neighbour along the shortest path: SWAP to the
//   SAME label (homogeneous SRGB), outgoing via the interface toward the
//   next hop;
// - if N IS a direct neighbour (this router is the penultimate hop): POP
//   (implicit-null / PHP semantics, RFC 8660 Section 2) -- this is exactly
//   the "pop, then forward out a specific interface" pattern already used
//   by ~Ldp's own penultimate-hop-popping entries, so no changes to
//   ~LibTable or ~Mpls are needed.
// - if N is unreachable, no entry is installed for it (a later
//   ~tedChangedSignal update will add one once/if it becomes reachable).
//
// This router's OWN node SID needs no LIB entry: packets addressed to it
// arrive already popped, thanks to the neighbouring penultimate-hop router's
// PHP entry.
//
// The module recomputes and reinstalls its entries whenever the ~Ted
// changes (subscribed at the host level, like ~LinkStateRouting), removing
// its previously-owned entries first so that recomputation is idempotent
// and never touches LIB entries owned by coexisting ~Ldp/~RsvpTe signaling.
//
// Adjacency SIDs and the SR-policy ingress classifier (segment-list label
// stack imposition) are out of scope for this module; see F1's later phases.
//
// The `sidTable` XML format: the root element is <sr>, containing any
// number of <node> elements, each with:
// - router: the router's IPv4 router id (e.g. "10.1.1.1")
// - sid: this router's node-SID index (integer, 0 <= sid < srgbSize; must
//   be unique among all <node> entries, as must `router`)
//
// <pre>
// <sr>
//     <node router="10.1.1.1" sid="1"/>
//     <node router="10.1.1.2" sid="2"/>
// </sr>
// </pre>
//
simple SegmentRouting extends SimpleModule
{
    parameters:
        @class(SegmentRouting);
        string interfaceTableModule; // The path to the InterfaceTable module
        string routingTableModule;   // The path to the Ipv4RoutingTable module (routerId, interface resolution)
        string libTableModule;       // The path to the LibTable module whose entries this module programs
        string tedModule;            // The path to the Ted module this router's shortest paths are computed from
        int srgbBase = default(16000); // Segment Routing Global Block: first label of the block
        int srgbSize = default(8000);  // Segment Routing Global Block: number of labels in the block
        xml sidTable; // network-wide router-id -> node-SID index map, see class documentation for the XML format; required, no default
        @display("i=block/table2");
        @signal[sidEntriesInstalled](type=long); // emitted after every (re)computation, with the number of node-SID LIB entries currently owned by this module
        @statistic[sidEntriesInstalled](title="number of installed node-SID LIB entries"; record=last,vector; interpolationmode=none);
}
