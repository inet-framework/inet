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

    // cListener method: reacts to tedChangedSignal, emitted at the host level by Ted
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
