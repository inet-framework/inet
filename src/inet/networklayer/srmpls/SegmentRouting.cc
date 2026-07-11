//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/srmpls/SegmentRouting.h"

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

        WATCH(routerId);
        WATCH(srgbBase);
        WATCH(srgbSize);
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
}

void SegmentRouting::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    printSignalBanner(signalID, obj, details);

    ASSERT(signalID == tedChangedSignal || signalID == routeAddedSignal || signalID == routeDeletedSignal);

    EV_INFO << "topology-affecting signal " << cComponent::getSignalName(signalID) << " received, recomputing node-SID label programming" << endl;

    recomputeAndInstall();
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

} // namespace inet
