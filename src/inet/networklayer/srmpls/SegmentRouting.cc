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

    for (int label : ownedLabels)
        lt->removeLibEntryIfExists(label);
    ownedLabels.clear();
    emit(sidEntriesInstalledSignal, (long)0);
}

void SegmentRouting::handleCrashOperation(LifecycleOperation *operation)
{
    cModule *host = getContainingNode(this);
    host->unsubscribe(tedChangedSignal, this);

    // LibTable is a plain SimpleModule, not lifecycle-aware -- it does NOT drop this module's
    // entries on its own just because SegmentRouting crashed, so they must be removed here.
    // Skipping this (leaving ownedLabels populated in the LIB but clearing our own bookkeeping)
    // would make the FIRST recomputeAndInstall() after a restart throw: it would try to
    // installReservedLabel() at labels that still exist from before the crash.
    for (int label : ownedLabels)
        lt->removeLibEntryIfExists(label);
    ownedLabels.clear();
}

void SegmentRouting::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    printSignalBanner(signalID, obj, details);

    ASSERT(signalID == tedChangedSignal);

    EV_INFO << "TED changed, recomputing node-SID label programming" << endl;

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

void SegmentRouting::recomputeAndInstall()
{
    // Wipe our own previously-installed entries first, so recomputation is idempotent and a
    // destination that just became unreachable doesn't leave a stale entry behind. Coexisting
    // Ldp/RsvpTe LIB entries are untouched -- ownedLabels only ever contains labels THIS module
    // installed.
    for (int label : ownedLabels)
        lt->removeLibEntryIfExists(label);
    ownedLabels.clear();

    const TeLinkStateInfoVector& topology = ted->getLinks();

    for (auto& elem : sidByRouter) {
        Ipv4Address destRouter = elem.first;
        int sid = elem.second;

        if (destRouter == routerId) // our own SID: no LIB entry needed, see class documentation
            continue;

        int label = srgbBase + sid;

        Ipv4AddressVector dest;
        dest.push_back(destRouter);
        Ipv4AddressVector path = ted->calculateShortestPath(dest, topology, 0.0, 7);

        if (path.size() < 2) {
            EV_WARN << "node " << destRouter << " (sid=" << sid << ", label=" << label << ") is currently unreachable, not installing a LIB entry" << endl;
            continue;
        }

        Ipv4Address nextHop = path[1];
        int outInterfaceId = resolveOutInterfaceId(nextHop);

        LabelOpVector outLabel;
        if (nextHop == destRouter) {
            // this router is the penultimate hop toward destRouter: PHP (RFC 8660 Section 2) --
            // same "pop, forward out a specific interface" entry Ldp already installs for its own
            // penultimate-hop-popping FECs (see Ldp::duAdvertiseToPeer's advertiseImplicitNull path).
            outLabel = LibTable::popLabel();
            EV_INFO << "node " << destRouter << " (sid=" << sid << ", label=" << label << "): directly connected, installing POP (PHP) via " << outInterfaceId << endl;
        }
        else {
            // transit: homogeneous SRGB means the label is unchanged network-wide
            outLabel = LibTable::swapLabel(label);
            EV_INFO << "node " << destRouter << " (sid=" << sid << ", label=" << label << "): transit via " << nextHop << ", installing SWAP via " << outInterfaceId << endl;
        }

        // installReservedLabel() (not installLibEntry()) because we need a LIB entry created
        // at this SPECIFIC label value (srgbBase+sid, not an auto-allocated one); the entry
        // is guaranteed absent at this point (removeLibEntryIfExists() above), so this can
        // never hit installReservedLabel()'s "already exists" guard.
        lt->installReservedLabel(label, LibTable::ANY_INTERFACE, outLabel, outInterfaceId);
        ownedLabels.insert(label);
    }

    emit(sidEntriesInstalledSignal, (long)ownedLabels.size());
}

} // namespace inet
