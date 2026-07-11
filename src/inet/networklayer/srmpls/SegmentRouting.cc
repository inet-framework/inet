//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/srmpls/SegmentRouting.h"

#include <sstream>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(SegmentRouting);

simsignal_t SegmentRouting::sidEntriesInstalledSignal = registerSignal("sidEntriesInstalled");
simsignal_t SegmentRouting::adjSidEntriesInstalledSignal = registerSignal("adjSidEntriesInstalled");

void SegmentRouting::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);
        lt.reference(this, "libTableModule", true);
        ted.reference(this, "tedModule", true);

        srgbBase = par("srgbBase");
        srgbSize = par("srgbSize");
        if (srgbSize <= 0)
            throw cRuntimeError("SegmentRouting: srgbSize must be positive, but is %d", srgbSize);

        readSidTableFromXML(par("sidTable").xmlValue());

        tiLfa = par("tiLfa");
        maxRepairStackDepth = par("maxRepairStackDepth");

        WATCH(routerId);
        WATCH(srgbBase);
        WATCH(srgbSize);
        WATCH(tiLfa);
    }
}

void SegmentRouting::handleMessageWhenUp(cMessage *msg)
{
    throw cRuntimeError("SegmentRouting does not process messages, but received '%s'", msg->getName());
}

void SegmentRouting::handleStartOperation(LifecycleOperation *operation)
{
    // Read at start (not cached earlier), same rationale as LinkStateRouting::handleStartOperation:
    // routerId comes from the routing table module, which -- like Ted's own routerId -- is only
    // guaranteed valid once earlier-stage initialization of that module has completed, and this
    // is re-read on every (re)start so a restarted node doesn't keep a stale routerId.
    routerId = rt->getRouterId();

    cModule *host = getContainingNode(this);
    host->subscribe(tedChangedSignal, this);

    // tedChangedSignal alone is NOT enough to learn about topology information this router
    // only knows about via FLOODING: LinkStateRouting::processLINK_STATE_MESSAGE applies a
    // remote link-state update it just learned about (a brand-new link, or an existing one's
    // state changing) by calling Ted::addLink()/Ted::rebuildRoutingTable() DIRECTLY, never
    // through Ted::setLinkState() -- so tedChangedSignal (reserved for THIS router's own
    // locally-observed liveness flips, specifically so LinkStateRouting knows to re-flood them
    // to peers, see Ted::setLinkState()'s doc) never fires for it. Left unaddressed, this
    // module would only ever see its own directly-connected links (the snapshot available at
    // the very first recomputeAndInstall() call below, before any flooding has happened) and
    // would never install a transit (SWAP) entry for an indirect node, no matter how long the
    // simulation runs. Ted::rebuildRoutingTable() is, however, the ONLY thing that ever
    // touches this router's routing table, and it runs unconditionally on every Ted content
    // change regardless of origin (local flip or flooded update) -- so subscribing to
    // Ipv4RoutingTable's routeAddedSignal/routeDeletedSignal (emitted from every route it
    // adds/removes) catches every such rebuild, local or remote-triggered alike.
    // recomputeAndInstall() is idempotent (wipe-owned-then-reinstall), so being triggered
    // redundantly -- once per intermediate route add/delete rather than once per logical
    // topology change -- is wasteful but harmless.
    host->subscribe(routeAddedSignal, this);
    host->subscribe(routeDeletedSignal, this);

    // Ted's own initial local-link discovery (Ted::initializeTED(), also run from
    // handleStartOperation) does NOT emit tedChangedSignal -- only later changes (setLinkState()/
    // announceLinkChange()) do -- so an initial computation here is needed to pick up that first,
    // signal-less snapshot of the topology. This relies on Ted's handleStartOperation having
    // already run (submodule declaration order within the same lifecycle stage; see
    // MplsRouterBase.ned, where `ted` is declared before any module that depends on it).
    recomputeAndInstall();
}

void SegmentRouting::handleStopOperation(LifecycleOperation *operation)
{
    cModule *host = getContainingNode(this);
    host->unsubscribe(tedChangedSignal, this);
    host->unsubscribe(routeAddedSignal, this);
    host->unsubscribe(routeDeletedSignal, this);

    for (int label : ownedLabels)
        lt->removeLibEntryIfExists(label);
    ownedLabels.clear();
    emit(sidEntriesInstalledSignal, (long)0);

    for (auto& elem : adjSidByInterfaceId)
        lt->removeLibEntryIfExists(elem.second);
    adjSidByInterfaceId.clear();
    emit(adjSidEntriesInstalledSignal, (long)0);

    // The LIB entries themselves are already gone (removed just above), so there is nothing
    // left to explicitly revert via activateBackup(); just drop our own bookkeeping so a
    // restarted module doesn't think a backup is active when its LIB entry doesn't even exist.
    tiLfaActiveDestinations.clear();
}

void SegmentRouting::handleCrashOperation(LifecycleOperation *operation)
{
    cModule *host = getContainingNode(this);
    host->unsubscribe(tedChangedSignal, this);
    host->unsubscribe(routeAddedSignal, this);
    host->unsubscribe(routeDeletedSignal, this);

    // LibTable is a plain SimpleModule, not lifecycle-aware -- it does NOT drop this module's
    // entries on its own just because SegmentRouting crashed, so they must be removed here.
    // Skipping this (leaving ownedLabels populated in the LIB but clearing our own bookkeeping)
    // would make the FIRST recomputeAndInstall() after a restart throw: it would try to
    // installReservedLabel() at labels that still exist from before the crash.
    for (int label : ownedLabels)
        lt->removeLibEntryIfExists(label);
    ownedLabels.clear();

    // Same rationale for adjacency-SID entries (dynamically allocated, but still left behind
    // in the LIB by a crash unless explicitly removed here).
    for (auto& elem : adjSidByInterfaceId)
        lt->removeLibEntryIfExists(elem.second);
    adjSidByInterfaceId.clear();

    // See handleStopOperation()'s comment: the LIB entries are gone, only our own bookkeeping
    // needs clearing.
    tiLfaActiveDestinations.clear();
}

void SegmentRouting::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    printSignalBanner(signalID, obj, details);

    ASSERT(signalID == tedChangedSignal || signalID == routeAddedSignal || signalID == routeDeletedSignal);

    EV_INFO << "topology-affecting signal " << cComponent::getSignalName(signalID) << " received, recomputing node-SID label programming" << endl;

    recomputeAndInstall();

    // TI-LFA activation (Phase 3): tedChangedSignal is the ONLY one of these three signals that
    // carries a TedChangeInfo -- routeAddedSignal/routeDeletedSignal are plain Ipv4Route
    // notifications with no link-index payload, so own-link-change detection only makes sense
    // here. recomputeAndInstall() above has ALREADY wiped and reinstalled every owned LIB entry
    // (using the by-now-already-updated ted[] state, see Ted::setLinkState()'s ordering: it flips
    // ted[index].state on its very first line, before rebuildRoutingTable() fires any signal at
    // all -- so by the time ANY handler in this cascade runs, including this one, ted[] already
    // reflects the NEW topology). handleOwnLinkStateChange() itself reconstructs whatever
    // pre-failure information it needs by recomputing against a synthetic "link forced back up"
    // topology, rather than relying on a snapshot taken here -- see its own doc for why.
    if (signalID == tedChangedSignal) {
        const TedChangeInfo *info = check_and_cast<const TedChangeInfo *>(obj);
        for (size_t i = 0; i < info->getTedLinkIndicesArraySize(); i++) {
            int index = info->getTedLinkIndices(i);
            const TeLinkStateInfo& link = ted->getLink(index);
            if (link.advrouter == routerId)
                handleOwnLinkStateChange(link.linkid, link.state);
        }
    }
}

void SegmentRouting::readSidTableFromXML(const cXMLElement *sidTableXml)
{
    ASSERT(sidTableXml);
    if (strcmp(sidTableXml->getTagName(), "sr") != 0)
        throw cRuntimeError("SegmentRouting: sidTable's root element must be <sr>, but is <%s> at %s",
                sidTableXml->getTagName(), sidTableXml->getSourceLocation());

    std::set<int> sidsSeen;

    cXMLElementList list = sidTableXml->getChildrenByTagName("node");
    for (auto& elem : list) {
        const cXMLElement& entry = *elem;

        const char *routerStr = entry.getAttribute("router");
        if (!routerStr)
            throw cRuntimeError("SegmentRouting: <node> at %s is missing the mandatory 'router' attribute", entry.getSourceLocation());
        if (!Ipv4Address::isWellFormed(routerStr))
            throw cRuntimeError("SegmentRouting: <node> at %s has an invalid 'router' address: '%s'", entry.getSourceLocation(), routerStr);
        Ipv4Address router(routerStr);

        const char *sidStr = entry.getAttribute("sid");
        if (!sidStr)
            throw cRuntimeError("SegmentRouting: <node> at %s is missing the mandatory 'sid' attribute", entry.getSourceLocation());
        int sid = atoi(sidStr);
        if (sid < 0 || sid >= srgbSize)
            throw cRuntimeError("SegmentRouting: <node router=\"%s\"> at %s has sid=%d, out of the configured SRGB range [0, %d)",
                    router.str().c_str(), entry.getSourceLocation(), sid, srgbSize);

        if (!sidByRouter.emplace(router, sid).second)
            throw cRuntimeError("SegmentRouting: sidTable has more than one <node> entry for router '%s', at %s",
                    router.str().c_str(), entry.getSourceLocation());

        if (!sidsSeen.insert(sid).second)
            throw cRuntimeError("SegmentRouting: sidTable assigns sid=%d to more than one router (duplicate at %s)",
                    sid, entry.getSourceLocation());
    }
}

int SegmentRouting::resolveOutInterfaceId(Ipv4Address peerRouterId)
{
    Ipv4Address localIfAddr = ted->getInterfaceAddrByPeerAddress(peerRouterId);
    NetworkInterface *ie = rt->getInterfaceByAddress(localIfAddr);
    if (!ie)
        throw cRuntimeError("SegmentRouting: no local interface found with address %s (peer router %s)",
                localIfAddr.str().c_str(), peerRouterId.str().c_str());
    return ie->getInterfaceId();
}

bool SegmentRouting::resolveNextHop(Ipv4Address destRouter, int& outInterfaceId, bool& directlyConnected)
{
    const TeLinkStateInfoVector& topology = ted->getLinks();

    Ipv4AddressVector dest;
    dest.push_back(destRouter);
    Ipv4AddressVector path = ted->calculateShortestPath(dest, topology, 0.0, 7);

    if (path.size() < 2)
        return false;

    Ipv4Address nextHop = path[1];
    outInterfaceId = resolveOutInterfaceId(nextHop);
    directlyConnected = (nextHop == destRouter);
    return true;
}

void SegmentRouting::recomputeAndInstall()
{
    // Wipe our own previously-installed entries first, so recomputation is idempotent and a
    // destination that just became unreachable doesn't leave a stale entry behind. Coexisting
    // Ldp/RsvpTe LIB entries are untouched -- ownedLabels only ever contains labels THIS module
    // installed.
    for (int label : ownedLabels)
        lt->removeLibEntryIfExists(label);
    ownedLabels.clear();

    for (auto& elem : sidByRouter) {
        Ipv4Address destRouter = elem.first;
        int sid = elem.second;

        if (destRouter == routerId) // our own SID: no LIB entry needed, see class documentation
            continue;

        int label = srgbBase + sid;

        int outInterfaceId;
        bool directlyConnected;
        if (!resolveNextHop(destRouter, outInterfaceId, directlyConnected)) {
            EV_WARN << "node " << destRouter << " (sid=" << sid << ", label=" << label << ") is currently unreachable, not installing a LIB entry" << endl;
            continue;
        }

        LabelOpVector outLabel;
        if (directlyConnected) {
            // this router is the penultimate hop toward destRouter: PHP (RFC 8660 Section 2) --
            // same "pop, forward out a specific interface" entry Ldp already installs for its own
            // penultimate-hop-popping FECs (see Ldp::duAdvertiseToPeer's advertiseImplicitNull path).
            outLabel = LibTable::popLabel();
            EV_INFO << "node " << destRouter << " (sid=" << sid << ", label=" << label << "): directly connected, installing POP (PHP) via " << outInterfaceId << endl;
        }
        else {
            // transit: homogeneous SRGB means the label is unchanged network-wide
            outLabel = LibTable::swapLabel(label);
            EV_INFO << "node " << destRouter << " (sid=" << sid << ", label=" << label << "): transit, installing SWAP via " << outInterfaceId << endl;
        }

        // installReservedLabel() (not installLibEntry()) because we need a LIB entry created
        // at this SPECIFIC label value (srgbBase+sid, not an auto-allocated one); the entry
        // is guaranteed absent at this point (removeLibEntryIfExists() above), so this can
        // never hit installReservedLabel()'s "already exists" guard.
        lt->installReservedLabel(label, LibTable::ANY_INTERFACE, outLabel, outInterfaceId);
        ownedLabels.insert(label);
    }

    emit(sidEntriesInstalledSignal, (long)ownedLabels.size());

    recomputeAdjacencySids();

    if (tiLfa)
        computeTiLfaRepairs();
}

void SegmentRouting::recomputeAdjacencySids()
{
    const TeLinkStateInfoVector& topology = ted->getLinks();

    // Currently-up local links (this router's own outgoing interfaces), keyed by interface id.
    // TeLinkStateInfo entries with advrouter==routerId are exactly this router's own advertised
    // links, one per physical interface (see Ted::initializeTED()); `local` is that interface's
    // own address.
    std::map<int, Ipv4Address> upInterfaces; // interfaceId -> peer router id (informational)
    for (auto& link : topology) {
        if (link.advrouter != routerId || !link.state)
            continue;

        NetworkInterface *ie = rt->getInterfaceByAddress(link.local);
        if (!ie)
            throw cRuntimeError("SegmentRouting: no local interface found with address %s for adjacency-SID allocation", link.local.str().c_str());
        upInterfaces[ie->getInterfaceId()] = link.linkid;
    }

    // Withdraw adjacency SIDs for interfaces that are no longer up (or no longer present in the
    // topology at all); labels for interfaces that stay up are left untouched, see class doc.
    for (auto it = adjSidByInterfaceId.begin(); it != adjSidByInterfaceId.end(); ) {
        if (upInterfaces.find(it->first) == upInterfaces.end()) {
            lt->removeLibEntryIfExists(it->second);
            EV_INFO << "adjacency SID for interface " << ift->getInterfaceById(it->first)->getInterfaceName()
                    << " (label " << it->second << "): link is down, withdrawing" << endl;
            it = adjSidByInterfaceId.erase(it);
        }
        else
            ++it;
    }

    // Allocate adjacency SIDs for newly-up interfaces that don't already have one.
    for (auto& elem : upInterfaces) {
        int interfaceId = elem.first;
        if (adjSidByInterfaceId.count(interfaceId))
            continue; // already has a stable label from a previous computation

        // Directed "pop and forward out this specific interface" -- RFC 8660's adjacency-SID
        // forwarding action; see Mpls::processMplsPacketFromL2, which forwards via the resolved
        // outInterfaceId regardless of whether the pop exposed another label or plain IP,
        // exactly like an existing PHP entry -- no Mpls.cc changes needed (verified by reading
        // doStackOps()/processMplsPacketFromL2() before writing this, per the plan's Phase-0
        // audit). Auto-allocated (installLibEntry(-1,...), NOT installReservedLabel()) since
        // adjacency SIDs are locally significant labels, not drawn from the SRGB; LibTable's
        // maxLabel counter is shared with Ldp/RsvpTe's own dynamic allocations
        // (installLibEntry()/allocateLabel()), so collisions with their labels are structurally
        // impossible (see LibTable::installLibEntry()).
        int label = lt->installLibEntry(-1, LibTable::ANY_INTERFACE, LibTable::popLabel(), interfaceId);
        adjSidByInterfaceId[interfaceId] = label;
        EV_INFO << "adjacency SID for interface " << ift->getInterfaceById(interfaceId)->getInterfaceName()
                << " (peer " << elem.second << "): installed POP via " << interfaceId << ", label=" << label << endl;
    }

    emit(adjSidEntriesInstalledSignal, (long)adjSidByInterfaceId.size());
}

int SegmentRouting::getAdjSidLabel(int interfaceId) const
{
    auto it = adjSidByInterfaceId.find(interfaceId);
    return it != adjSidByInterfaceId.end() ? it->second : -1;
}

Ipv4AddressVector SegmentRouting::getOwnNeighbors(const TeLinkStateInfoVector& topology)
{
    Ipv4AddressVector neighbors;
    for (auto& link : topology) {
        if (link.advrouter == routerId && link.state)
            neighbors.push_back(link.linkid);
    }
    return neighbors;
}

TeLinkStateInfoVector SegmentRouting::withLinkExcluded(const TeLinkStateInfoVector& topology, Ipv4Address a, Ipv4Address b)
{
    TeLinkStateInfoVector result = topology;
    for (auto& link : result) {
        if ((link.advrouter == a && link.linkid == b) || (link.advrouter == b && link.linkid == a))
            link.state = false;
    }
    return result;
}

TeLinkStateInfoVector SegmentRouting::reversedTopology(const TeLinkStateInfoVector& topology)
{
    TeLinkStateInfoVector result = topology;
    for (auto& link : result) {
        Ipv4Address tmpRouter = link.advrouter;
        link.advrouter = link.linkid;
        link.linkid = tmpRouter;

        Ipv4Address tmpAddr = link.local;
        link.local = link.remote;
        link.remote = tmpAddr;
    }
    return result;
}

std::set<Ipv4Address> SegmentRouting::distancePreservingReachable(Ipv4Address root,
        const TeLinkStateInfoVector& fullTopology, const TeLinkStateInfoVector& filteredTopology)
{
    std::set<Ipv4Address> result;
    result.insert(root); // trivially at distance 0 from itself, in both topologies

    for (auto& elem : sidByRouter) {
        Ipv4Address candidate = elem.first;
        if (candidate == root)
            continue;

        double fullDist = -1, filteredDist = -1;
        bool reachableInFull = ted->getShortestPathCost(root, candidate, fullTopology, 0.0, 7, fullDist);
        bool reachableInFiltered = ted->getShortestPathCost(root, candidate, filteredTopology, 0.0, 7, filteredDist);

        if (!reachableInFull || !reachableInFiltered)
            continue; // unreachable in either topology: not a (simple) P-/Q-space member

        if (filteredDist <= fullDist)
            result.insert(candidate);
        // else: reachable, but only via a STRICTLY LONGER path once the filtered link is
        // excluded -- deliberately NOT included (see this method's header doc: relying on such
        // a path risks a transient micro-loop, exactly what P-space/Q-space rules out).
        // filteredDist can never be LESS than fullDist (removing a link can only lengthen or
        // preserve a shortest path), so "<=" and "==" are equivalent here; "<=" is used only to
        // be robust against floating-point noise from repeated additions.
    }

    return result;
}

bool SegmentRouting::findTiLfaRepair(Ipv4Address dest, Ipv4Address protectedNeighbor,
        const TeLinkStateInfoVector& topology, LabelOpVector& outRepair, int& outBackupOutInterfaceId,
        std::string& outDescription)
{
    Ipv4AddressVector ownNeighbors = getOwnNeighbors(topology);
    TeLinkStateInfoVector excludedTopology = withLinkExcluded(topology, routerId, protectedNeighbor);

    // P-space: nodes whose shortest-path cost from this router is UNCHANGED once the protected
    // link is excluded (distancePreservingReachable()'s definition -- see its doc for why a node
    // reachable only via a strictly LONGER alternate is excluded). Extended P-space additionally
    // includes such nodes reachable from each of this router's OTHER neighbors, over the SAME
    // topology pair (RFC 9855; increases the chance of a PQ hit).
    std::set<Ipv4Address> pSpace = distancePreservingReachable(routerId, topology, excludedTopology);
    for (auto& neighbor : ownNeighbors) {
        if (neighbor == protectedNeighbor)
            continue;
        std::set<Ipv4Address> extra = distancePreservingReachable(neighbor, topology, excludedTopology);
        pSpace.insert(extra.begin(), extra.end());
    }

    // Q-space of dest: nodes whose OWN shortest-path cost TO dest is unchanged once the
    // protected link is excluded -- computed via a reverse-SPT rooted at dest, over the reversed
    // topology pair (order of reverse-then-exclude vs exclude-then-reverse doesn't matter:
    // withLinkExcluded() matches the link in both directions either way).
    TeLinkStateInfoVector reversedFull = reversedTopology(topology);
    TeLinkStateInfoVector reversedExcluded = reversedTopology(excludedTopology);
    std::set<Ipv4Address> qSpace = distancePreservingReachable(dest, reversedFull, reversedExcluded);

    // Case 1: a PQ node exists -> single node-SID segment. Skip routerId itself (using our own
    // "node SID" as a repair segment is meaningless -- we're already here) AND protectedNeighbor
    // (the far end of the link that just failed/is being protected against): protectedNeighbor
    // can genuinely satisfy the distance-preserving P-space/Q-space tests (e.g. reachable from
    // some OTHER neighbor at unchanged cost via a path that never used the protected link at
    // all), but routing traffic for a DIFFERENT destination THROUGH the very node whose
    // adjacency to the PLR just changed relies on that node's own view of the network already
    // being consistent -- exactly the kind of assumption RFC 9855's P/Q-space rules exist to
    // avoid leaning on. The one destination this exclusion trivially still protects is
    // protectedNeighbor itself (dest == protectedNeighbor): Q-space always contains dest itself
    // (root-included, trivially), so excluding protectedNeighbor here just means that case falls
    // through to the next real PQ candidate instead of a degenerate self-referential push --
    // strictly safer, and no less correct (the destination's own node-SID is still what a
    // genuine PQ node ultimately delivers it to).
    for (auto& node : pSpace) {
        if (node == routerId || node == protectedNeighbor || !qSpace.count(node))
            continue;

        int sid = sidByRouter.at(node);
        int label = srgbBase + sid;
        LabelOp op;
        op.optcode = PUSH_OPER;
        op.label = label;
        outRepair = LabelOpVector{op};
        outBackupOutInterfaceId = resolveBackupOutInterfaceId(node, excludedTopology);
        std::ostringstream desc;
        desc << "[PUSH " << label << " (" << node << ")]";
        outDescription = desc.str();
        return true;
    }

    // Case 2: no PQ node -- look for a P-space node P directly adjacent (a real link, not the
    // protected one) to a Q-space node Q, and bridge with TWO node-SID segments [P, Q] (see class
    // documentation for why a second node-SID, not P's adjacency-SID).
    if (maxRepairStackDepth >= 2) {
        for (auto& link : topology) {
            if (!link.state)
                continue;
            if (link.advrouter == routerId && link.linkid == protectedNeighbor)
                continue; // the protected link itself can never be part of its own repair

            Ipv4Address p = link.advrouter;
            Ipv4Address q = link.linkid;
            if (p == q || !pSpace.count(p) || !qSpace.count(q))
                continue;
            if (p == routerId)
                continue; // that would just be case 1 with q as the PQ node -- already tried
            if (p == protectedNeighbor || q == protectedNeighbor)
                continue; // same reasoning as Case 1's protectedNeighbor exclusion above

            int pSid = sidByRouter.at(p);
            int qSid = sidByRouter.at(q);
            int pLabel = srgbBase + pSid;
            int qLabel = srgbBase + qSid;

            // Wire order: LibTable::pushLabel()/Mpls::doStackOps() insert each PUSH at the FRONT
            // of the packet's label stack, so the LAST op appended ends up on top -- P must be
            // processed FIRST (it's the immediate hop), so P's PUSH must be appended LAST.
            LabelOp opQ;
            opQ.optcode = PUSH_OPER;
            opQ.label = qLabel;
            LabelOp opP;
            opP.optcode = PUSH_OPER;
            opP.label = pLabel;

            outRepair = LabelOpVector{opQ, opP};
            outBackupOutInterfaceId = resolveBackupOutInterfaceId(p, excludedTopology);
            std::ostringstream desc;
            desc << "[PUSH " << pLabel << " (" << p << "), PUSH " << qLabel << " (" << q << ")]";
            outDescription = desc.str();
            return true;
        }
    }

    return false;
}

void SegmentRouting::computeTiLfaRepairs()
{
    tiLfaRepairByRouter.clear();
    tiLfaProtectedNeighborByRouter.clear();
    tiLfaBackupOutInterfaceByRouter.clear();

    const TeLinkStateInfoVector& topology = ted->getLinks();

    for (auto& elem : sidByRouter) {
        Ipv4Address dest = elem.first;
        if (dest == routerId)
            continue;

        // The protected link is whichever of THIS router's own links dest's CURRENT shortest
        // path leaves via -- exactly node-SID forwarding's own next hop for dest, computed the
        // same way recomputeAndInstall() does, but read out directly here (rather than through
        // resolveNextHop()) because the protected link's FAR END (path[1]) is exactly what's
        // needed, not just the resolved interface id.
        Ipv4AddressVector destVec;
        destVec.push_back(dest);
        Ipv4AddressVector path = ted->calculateShortestPath(routerId, destVec, topology, 0.0, 7);
        if (path.size() < 2) {
            EV_DETAIL << "TI-LFA repair for " << dest << ": UNPROTECTED (destination currently unreachable)" << endl;
            continue;
        }
        Ipv4Address protectedNeighbor = path[1];

        LabelOpVector repair;
        int backupOutInterfaceId;
        std::string description;
        if (findTiLfaRepair(dest, protectedNeighbor, topology, repair, backupOutInterfaceId, description)) {
            tiLfaRepairByRouter[dest] = repair;
            tiLfaProtectedNeighborByRouter[dest] = protectedNeighbor;
            tiLfaBackupOutInterfaceByRouter[dest] = backupOutInterfaceId;
            EV_DETAIL << "TI-LFA repair for " << dest << " via " << protectedNeighbor << ": " << description << endl;
        }
        else {
            EV_WARN << "TI-LFA: no repair found for " << dest << " (protected link to " << protectedNeighbor << ") within " << maxRepairStackDepth << " segments; leaving unprotected" << endl;
            EV_DETAIL << "TI-LFA repair for " << dest << " via " << protectedNeighbor << ": UNPROTECTED" << endl;
        }
    }
}

int SegmentRouting::resolveBackupOutInterfaceId(Ipv4Address firstHopTarget, const TeLinkStateInfoVector& excludedTopology)
{
    Ipv4AddressVector firstHopDest;
    firstHopDest.push_back(firstHopTarget);
    Ipv4AddressVector path = ted->calculateShortestPath(routerId, firstHopDest, excludedTopology, 0.0, 7);
    ASSERT(path.size() >= 2); // firstHopTarget is a P-space member, so it must be reachable over excludedTopology
    return resolveOutInterfaceId(path[1]);
}

void SegmentRouting::handleOwnLinkStateChange(Ipv4Address peerRouterId, bool up)
{
    if (!tiLfa)
        return;

    if (!up) {
        // Link to peerRouterId just went down. By this point recomputeAndInstall() (called just
        // before this, from receiveSignal()) has already reprogrammed every destination's
        // protectedNeighbor using the POST-failure ted[] -- Ted::setLinkState() flips
        // ted[index].state on its very first line, before any signal in this cascade fires, so
        // there is no point at which a signal handler still observes the pre-failure topology.
        // A snapshot taken earlier in this same cascade would therefore already be stale too.
        //
        // Instead of snapshotting, reconstruct "which destinations used to route via
        // peerRouterId" directly: run the shortest-path computation again over a topology where
        // this link is forced back to "up" (everything else left exactly as ted[] has it now,
        // post-failure) -- this reconstructed topology is also what gets passed to
        // findTiLfaRepair() as the reference "full" topology, since P-space/Q-space's
        // distance-preserving comparisons are only meaningful against a genuinely-up baseline:
        // passing the already-down real topology as the "full" reference would make excluding
        // the (already excluded) link a no-op, trivially satisfying the distance-preserving
        // check for every node and silently discarding RFC 9855's actual P/Q-space constraint.
        TeLinkStateInfoVector topologyWithLinkUp = ted->getLinks();
        for (auto& link : topologyWithLinkUp) {
            if ((link.advrouter == routerId && link.linkid == peerRouterId)
                    || (link.advrouter == peerRouterId && link.linkid == routerId))
                link.state = true;
        }

        for (auto& elem : sidByRouter) {
            Ipv4Address dest = elem.first;
            if (dest == routerId)
                continue;

            Ipv4AddressVector destVec;
            destVec.push_back(dest);
            Ipv4AddressVector preFailurePath = ted->calculateShortestPath(routerId, destVec, topologyWithLinkUp, 0.0, 7);
            if (preFailurePath.size() < 2 || preFailurePath[1] != peerRouterId)
                continue; // dest wasn't routed via this link before it failed

            LabelOpVector repair;
            int backupOutInterfaceId;
            std::string description;
            if (!findTiLfaRepair(dest, peerRouterId, topologyWithLinkUp, repair, backupOutInterfaceId, description))
                continue; // no repair available for this destination against this link

            int sid = sidByRouter.at(dest);
            int label = srgbBase + sid;
            lt->setBackup(label, repair, backupOutInterfaceId);
            lt->activateBackup(label, true);
            tiLfaActiveDestinations.insert(dest);
            EV_INFO << "TI-LFA: activated backup for " << dest << " (protected link to " << peerRouterId << " went down)" << endl;
        }
    }
    else {
        // Link to peerRouterId came back up: revert every destination whose backup was
        // activated because of exactly this link.
        for (auto it = tiLfaActiveDestinations.begin(); it != tiLfaActiveDestinations.end(); ) {
            Ipv4Address dest = *it;
            auto neighborIt = tiLfaProtectedNeighborByRouter.find(dest);
            if (neighborIt == tiLfaProtectedNeighborByRouter.end() || neighborIt->second != peerRouterId) {
                ++it;
                continue;
            }

            int sid = sidByRouter.at(dest);
            int label = srgbBase + sid;
            lt->activateBackup(label, false);
            EV_INFO << "TI-LFA: reverted backup for " << dest << " (protected link to " << peerRouterId << " came back up)" << endl;
            it = tiLfaActiveDestinations.erase(it);
        }
    }
}

} // namespace inet
