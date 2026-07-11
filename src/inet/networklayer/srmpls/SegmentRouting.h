//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SEGMENTROUTING_H
#define __INET_SEGMENTROUTING_H

#include <map>
#include <set>

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/networklayer/ted/Ted.h"
#include "inet/routing/base/RoutingProtocolBase.h"

namespace inet {

/**
 * Segment Routing (RFC 8660) node-SID label programming; see SegmentRouting.ned
 * for the full design writeup and the sidTable XML format.
 */
class INET_API SegmentRouting : public RoutingProtocolBase, public cListener
{
  protected:
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<LibTable> lt;
    ModuleRefByPar<Ted> ted;

    int srgbBase = 0;
    int srgbSize = 0;

    // TI-LFA (RFC 9855, Workstream F2). Off by default: computeTiLfaRepairs() is only ever
    // called from recomputeAndInstall() when tiLfa is true, so every existing network (which
    // never sets it) is completely unaffected -- no repair lists are ever computed, and
    // Phase 3's activation wiring (not yet written) has nothing to activate.
    bool tiLfa = false;
    int maxRepairStackDepth = 3;

    // Per-destination TI-LFA repair label stack, computed by computeTiLfaRepairs() whenever
    // tiLfa is true and recomputeAndInstall() runs. A destination with NO entry here (or an
    // empty LabelOpVector) is UNPROTECTED: no repair within maxRepairStackDepth was found (see
    // computeTiLfaRepairs()'s doc).
    std::map<Ipv4Address, LabelOpVector> tiLfaRepairByRouter;

    // dest -> the protected neighbor (this router's current next hop toward dest) that
    // tiLfaRepairByRouter[dest] guards against. Populated in lock-step with
    // tiLfaRepairByRouter by computeTiLfaRepairs(); consumed by handleOwnLinkStateChange() to
    // decide which destinations' backups to (de)activate when a specific local link flips.
    std::map<Ipv4Address, Ipv4Address> tiLfaProtectedNeighborByRouter;

    // dest -> the local interface id this router should forward the repaired packet out of,
    // i.e. the next hop toward the repair's first/outermost segment target (the PQ node in
    // Case 1; P in Case 2), computed over the protected-link-excluded topology. Populated in
    // lock-step with tiLfaRepairByRouter by computeTiLfaRepairs(); this is exactly the
    // backupOutInterfaceId LibTable::setBackup() needs.
    std::map<Ipv4Address, int> tiLfaBackupOutInterfaceByRouter;

    // Destinations whose TI-LFA backup is currently ACTIVE (LibTable::activateBackup(label,
    // true) has been called and not yet reverted), maintained by handleOwnLinkStateChange().
    std::set<Ipv4Address> tiLfaActiveDestinations;

    // parsed from the "sidTable" XML parameter: router id -> node-SID index
    std::map<Ipv4Address, int> sidByRouter;

    Ipv4Address routerId;

    // inLabels of the LIB entries currently installed by this module (so that
    // recomputation can remove exactly its own entries and never touch
    // coexisting Ldp/RsvpTe LIB entries)
    std::set<int> ownedLabels;

    // Adjacency-SID bookkeeping, kept deliberately separate from the node-SID `ownedLabels`
    // above (different lifecycle: dynamically allocated via LibTable::installLibEntry(-1,...),
    // never removed+reinstalled wholesale on every tedChangedSignal the way node-SIDs are --
    // see recomputeAdjacencySids()). Keyed by the LOCAL interface id of the adjacency (this
    // router's own outgoing link), which is also the key SrPolicy's `adj:` segments resolve
    // through getAdjSidLabel().
    std::map<int, int> adjSidByInterfaceId;

    static simsignal_t sidEntriesInstalledSignal;
    static simsignal_t adjSidEntriesInstalledSignal;

  public:
    SegmentRouting() {}

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // cListener method: reacts to tedChangedSignal (Ted, local liveness flips) and to
    // routeAddedSignal/routeDeletedSignal (Ipv4RoutingTable, emitted by EVERY route
    // Ted::rebuildRoutingTable() installs/removes -- including ones triggered by
    // LinkStateRouting learning a REMOTE topology change via flooding, which does NOT emit
    // tedChangedSignal; see handleStartOperation()'s subscription comment for why both are
    // needed).
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // static configuration
    virtual void readSidTableFromXML(const cXMLElement *sidTable);

    // Removes every LIB entry currently owned by this module, then recomputes node-SID
    // forwarding for every OTHER router in sidByRouter from the Ted's current topology and
    // (re)installs the resulting LIB entries. Idempotent; safe to call repeatedly (on every
    // tedChangedSignal) and on startup. Also drives adjacency-SID (re)computation, see
    // recomputeAdjacencySids().
    virtual void recomputeAndInstall();

    // Resolves the local outgoing interface id toward directly-connected peer router
    // `peerRouterId` (as identified by Ted::getInterfaceAddrByPeerAddress()).
    virtual int resolveOutInterfaceId(Ipv4Address peerRouterId);

    // Per-local-link adjacency-SID allocation (RFC 8660 Section 3.3): for every one of THIS
    // router's own outgoing Ted links that is currently up, ensures a dynamically-allocated
    // (installLibEntry(-1,...), never installReservedLabel()) LIB entry exists that pops the
    // label and forwards out that specific interface -- "cross this adjacency, then continue
    // label-switching with whatever's beneath" (or, if nothing's beneath, deliver the exposed
    // plain IP packet out that same interface; Mpls::processMplsPacketFromL2 already handles
    // both cases via the shared outInterfaceId, the same way it does for existing PHP entries,
    // so no Mpls.cc changes are needed -- verified by reading doStackOps()/
    // processMplsPacketFromL2() before writing this).
    //
    // Unlike recomputeAndInstall()'s node-SID handling (full wipe + reinstall every time,
    // affordable because installReservedLabel() always reproduces the SAME label value), this
    // method only touches links whose up/down state actually changed since the last call: a
    // link that stays up keeps its dynamically-allocated label stable across unrelated TED
    // churn elsewhere in the network (wiping-and-reinstalling everything unconditionally would
    // otherwise hand out a new, ever-growing label number for that link on every unrelated
    // tedChangedSignal, which would both be wasteful and needlessly invalidate any in-flight
    // assumption about that link's label).
    virtual void recomputeAdjacencySids();

    // TI-LFA (RFC 9855), pure computation -- see SegmentRouting.ned for the full design
    // writeup. Called from recomputeAndInstall() (right after node-SID entries are
    // (re)installed) whenever tiLfa is true. For every OTHER router D in sidByRouter whose
    // current shortest path from this router (the PLR) leaves via one of THIS router's own
    // links L (i.e. every destination this router is a potential point-of-local-repair for):
    // computes P-space (nodes reachable from the PLR without traversing L; "extended" P-space
    // additionally includes nodes reachable from each of the PLR's other neighbors without
    // traversing L) and Q-space of D (nodes whose shortest path TO D does not traverse L,
    // computed via a reversed-topology SPT rooted at D). If a PQ node (P-space ∩ Q-space)
    // exists, the repair is that node's own node-SID (single segment). Otherwise, tries a
    // P-space node P that is topologically ADJACENT to some Q-space node Q (a real physical
    // link, not the protected one) and uses [node-SID(P), node-SID(Q)] (two segments) as the
    // bridge -- see the .ned doc for why this model uses a SECOND NODE-SID here rather than a
    // P-node's own adjacency-SID (the textbook RFC 9855 §5 form): adjacency SIDs in this model
    // are locally significant and never flooded (the same limitation SrPolicy's "adj:" segments
    // document), so the PLR can only ever resolve label values for ITS OWN local adjacencies,
    // never a third-party P-node's -- two globally-resolvable node-SIDs sidestep that entirely.
    // If neither a PQ node nor an adjacent P/Q pair exists within maxRepairStackDepth segments,
    // the destination is left unprotected (no entry in tiLfaRepairByRouter) and an EV_WARN is
    // logged. Every computed repair (or lack thereof) is also logged via EV_DETAIL in the
    // format "TI-LFA repair for <dest> via <protectedNeighbor>: <segment list>" (or "UNPROTECTED"
    // for the no-repair case), which is what the module tests assert against.
    virtual void computeTiLfaRepairs();

    // Core of computeTiLfaRepairs(), factored out so handleOwnLinkStateChange() (Phase 3) can
    // also ask "what would the repair be if THIS SPECIFIC link were the protected one", for a
    // link/destination pair that computeTiLfaRepairs()'s own current-shortest-path-driven loop
    // would no longer pick (e.g. once that link has already failed and traffic has moved off
    // it). Returns true and fills outRepair/outBackupOutInterfaceId/outDescription if a Case-1
    // (PQ node) or Case-2 (P/Q bridge, within maxRepairStackDepth) repair exists; returns false
    // (outputs untouched) otherwise.
    virtual bool findTiLfaRepair(Ipv4Address dest, Ipv4Address protectedNeighbor,
            const TeLinkStateInfoVector& topology, LabelOpVector& outRepair,
            int& outBackupOutInterfaceId, std::string& outDescription);

    // Helper for computeTiLfaRepairs(): resolves the local outgoing interface id toward the
    // NEXT HOP on the shortest path (over `excludedTopology`, i.e. with the protected link
    // already excluded) from this router to `firstHopTarget` -- the repair's first/outermost
    // segment target (the PQ node in Case 1; P in Case 2). This is exactly the interface the
    // repaired, backup-labeled packet must actually be forwarded out of.
    virtual int resolveBackupOutInterfaceId(Ipv4Address firstHopTarget, const TeLinkStateInfoVector& excludedTopology);

    // TI-LFA activation trigger (Phase 3): reacts to a LOCAL link's up/down transition (the
    // peer at the far end is peerRouterId) by activating/reverting the precomputed backups that
    // specifically guard against this link. Called from receiveSignal() for tedChangedSignal,
    // AFTER recomputeAndInstall() has already run for this same signal. No-op if tiLfa is false.
    //
    // Ted::setLinkState() flips ted[index].state on its very first line, before
    // rebuildRoutingTable() fires any signal at all -- so by the time ANY handler in this
    // cascade runs (including this one), ted[] already reflects the POST-failure topology; a
    // snapshot taken earlier in the same cascade would already be stale too (confirmed
    // empirically: an earlier snapshot-based design silently never found anything to activate).
    // So for a link going DOWN (up==false), this method does not rely on any snapshot: it
    // reconstructs a topology with exactly this link forced back to "up" (everything else left
    // as ted[] has it now, post-failure), uses that to find which destinations used to route
    // via peerRouterId, and passes that SAME reconstructed topology to findTiLfaRepair() as the
    // reference "full" topology -- P-space/Q-space's distance-preserving comparisons need a
    // genuinely-up baseline; the real post-failure ted[] would make excluding the
    // (already-excluded) link a no-op, trivially satisfying the check for every node and
    // silently discarding RFC 9855's actual constraint. For a link coming back UP, no
    // reconstruction is needed:
    // recomputeAndInstall() just restored the ORIGINAL protectedNeighbor mapping, so the current
    // (member) tiLfaProtectedNeighborByRouter already matches what revert needs.
    virtual void handleOwnLinkStateChange(Ipv4Address peerRouterId, bool up);

    // Nodes N (root included, trivially) whose shortest-path COST from `root` is UNCHANGED
    // between `fullTopology` (the real, unfiltered topology) and `filteredTopology` (typically
    // the same topology with the protected link excluded) -- i.e. N's shortest path does not
    // actually depend on the excluded link at all. This is RFC 9855's P-space/Q-space
    // membership test: a node that is only reachable via a STRICTLY LONGER alternate once the
    // protected link is excluded is deliberately NOT included here, since relying on such a
    // path risks a transient micro-loop (the very failure mode P-space/Q-space is designed to
    // rule out) -- see Ted::getShortestPathCost(). Used to compute both P-space (root=this
    // router or a neighbor, filteredTopology=protected-link-excluded) and Q-space (root=D,
    // both topologies reversed) -- see computeTiLfaRepairs(). Linear in the number of known
    // routers (two getShortestPathCost() calls per candidate); fine for the small topologies
    // TI-LFA in this model targets, and only ever runs on tedChangedSignal/routeAdded/
    // routeDeletedSignal, never per-packet.
    virtual std::set<Ipv4Address> distancePreservingReachable(Ipv4Address root,
            const TeLinkStateInfoVector& fullTopology, const TeLinkStateInfoVector& filteredTopology);

    // Returns a copy of `topology` with the (bidirectional) link between `a` and `b` marked
    // down (state=false in both directions) -- used to compute P-space/Q-space "as if the
    // protected link didn't exist", mirroring recomputeAndInstall()'s own topology-filtering
    // idiom (that one filters by removing this module's owned entries; this one filters the
    // topology copy itself before handing it to calculateShortestPath()).
    virtual TeLinkStateInfoVector withLinkExcluded(const TeLinkStateInfoVector& topology, Ipv4Address a, Ipv4Address b);

    // Returns a copy of `topology` with every entry's direction reversed (advrouter<->linkid,
    // local<->remote): a shortest-path computation rooted at D over this reversed copy gives,
    // for every node X, the cost of the ORIGINAL topology's shortest path FROM X TO D -- which
    // is exactly the reverse-SPT computation Q-space needs (RFC 9855's Q-space is defined in
    // terms of paths *toward* D, not *from* the PLR).
    virtual TeLinkStateInfoVector reversedTopology(const TeLinkStateInfoVector& topology);

    // This router's own direct, currently-up neighbors (routers with a link where
    // advrouter==routerId and state==true), used for extended P-space and for the
    // adjacent-P/Q-pair search in computeTiLfaRepairs().
    virtual Ipv4AddressVector getOwnNeighbors(const TeLinkStateInfoVector& topology);

  public:
    // Read-only access to the parsed sidTable's router -> node-SID index map, and to the
    // configured SRGB base, for SrPolicy's (Step 2) segment-list resolution: SrPolicy validates
    // and resolves "node:<sid>" segments against this data rather than duplicating sidTable
    // parsing/config.
    virtual const std::map<Ipv4Address, int>& getSidByRouter() const { return sidByRouter; }
    virtual int getSrgbBase() const { return srgbBase; }

    // Resolves the local outgoing interface toward `destRouter` over the current Ted topology
    // (the same CSPF computation recomputeAndInstall() uses for node-SID programming), and
    // reports via `directlyConnected` whether destRouter is THIS router's next hop along that
    // path (i.e. whether THIS router is the penultimate hop and would itself apply PHP for
    // destRouter's own node SID). Used both internally (recomputeAndInstall()) and by SrPolicy
    // (Step 2) to canonicalize an SR policy's first segment: if directlyConnected is true,
    // pushing that segment's label would be pointless (see SrPolicy.cc's class documentation).
    // Returns false if destRouter is currently unreachable.
    virtual bool resolveNextHop(Ipv4Address destRouter, int& outInterfaceId, bool& directlyConnected);

    // Returns the adjacency-SID label currently allocated for local interface `interfaceId`, or
    // -1 if none exists right now (the interface is down, unknown, or not yet computed). Same-
    // node only, per RFC 8660's "adjacency SIDs are locally significant" (see class doc).
    virtual int getAdjSidLabel(int interfaceId) const;
};

} // namespace inet

#endif
