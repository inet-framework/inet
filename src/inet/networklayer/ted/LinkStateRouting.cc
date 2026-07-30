//
// Copyright (C) 2005 Vojtech Janota, Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ted/LinkStateRouting.h"

#include <algorithm>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ted/Ted.h"

namespace inet {

Define_Module(LinkStateRouting);

LinkStateRouting::LinkStateRouting()
{
}

LinkStateRouting::~LinkStateRouting()
{
    cancelAndDelete(announceMsg);
}

// Recursively tests whether a network node runs link-state routing (i.e. is an
// RSVP-TE peer we should flood our TE link-state to).
static bool moduleContainsLinkStateRouting(cModule *module)
{
    for (cModule::SubmoduleIterator it(module); !it.end(); ++it) {
        cModule *submodule = *it;
        if (dynamic_cast<LinkStateRouting *>(submodule) != nullptr)
            return true;
        if (moduleContainsLinkStateRouting(submodule))
            return true;
    }
    return false;
}

void LinkStateRouting::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        // Bind the Ted module reference early (LOCAL stage), even though the
        // rest of this module's own registration-time setup below happens at
        // INITSTAGE_ROUTING_PROTOCOLS: RoutingProtocolBase::initialize(stage)
        // (the call just above) can itself invoke handleStartOperation() from
        // INSIDE that call, at the very same ROUTING_PROTOCOLS stage -- i.e.
        // BEFORE the rest of this function's stage==ROUTING_PROTOCOLS branch
        // below has run. handleStartOperation() reads tedmod->getLinks() to
        // compute the peer snapshot, so tedmod must already be bound by then.
        tedmod.reference(this, "tedModule", true);
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        // listen for TED modifications
        cModule *host = getContainingNode(this);
        host->subscribe(tedChangedSignal, this);

        WATCH(routerId);
        WATCH(peerIfAddrs);
        WATCH(announceMsg);
        registerProtocol(Protocol::linkStateRouting, gate("ipOut"), gate("ipIn"));
    }
}

void LinkStateRouting::handleStartOperation(LifecycleOperation *operation)
{
    // routerId is re-read from the routing table on every (re)start, same as
    // tedmod->getLinks() below and as Ted::initializeTED() does for its own
    // routerId -- by the time this runs (ROUTING_PROTOCOLS stage, or a real
    // restart), the routing table module's own earlier-stage initialization
    // has already completed, so this is safe despite not being cached at an
    // earlier stage of THIS module.
    IIpv4RoutingTable *rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);
    routerId = rt->getRouterId();

    // Determine the local interfaces facing our RSVP-TE peers, and store their
    // addresses in peerIfAddrs[]. With peers="auto" (the default) they are
    // derived from the TED -- every directly-connected neighbour that also runs
    // link-state routing (skipping hosts and non-TE routers); otherwise "peers"
    // is a space-separated list of the peer-facing interface names.
    //
    // Recomputing this snapshot on every start (not just once, at time 0) is
    // the restart-safety fix: previously this ran only from initialize(), so a
    // restarted node kept flooding with a stale peerIfAddrs snapshot and,
    // because this module wasn't lifecycle-aware at all, never noticed it had
    // been stopped and restarted in between.
    peerIfAddrs.clear();
    const char *peers = par("peers");
    if (!strcmp(peers, "auto")) {
        for (auto& link : tedmod->getLinks()) {
            if (link.advrouter != routerId) // not one of our local links
                continue;
            if (link.linkid.isUnspecified()) // neighbour has no router id (e.g. a host)
                continue;
            cModule *peerNode = L3AddressResolver().findHostWithAddress(L3Address(link.remote));
            if (peerNode != nullptr && moduleContainsLinkStateRouting(peerNode))
                peerIfAddrs.push_back(link.local);
        }
    }
    else {
        cStringTokenizer tokenizer(peers);
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        const char *token;
        while ((token = tokenizer.nextToken()) != nullptr) {
            peerIfAddrs.push_back(CHK(ift->findInterfaceByName(token))->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
        }
    }

    // schedule (re-)start of flooding link state info
    ASSERT(announceMsg == nullptr);
    announceMsg = new cMessage("announce");
    scheduleAt(simTime() + exponential(0.01), announceMsg);
}

void LinkStateRouting::handleStopOperation(LifecycleOperation *operation)
{
    cancelAndDelete(announceMsg);
    announceMsg = nullptr;
    peerIfAddrs.clear();
}

void LinkStateRouting::handleCrashOperation(LifecycleOperation *operation)
{
    cancelAndDelete(announceMsg);
    announceMsg = nullptr;
    peerIfAddrs.clear();
}

void LinkStateRouting::handleMessageWhenUp(cMessage *msg)
{
    if (msg == announceMsg) {
        delete announceMsg;
        announceMsg = nullptr;
        sendToPeers(tedmod->getLinks(), true, Ipv4Address());
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "ipIn")) {
        EV_INFO << "Processing message from Ipv4: " << msg << endl;
        Ipv4Address sender = check_and_cast<Packet *>(msg)->getTag<L3AddressInd>()->getSrcAddress().toIpv4();
        processLINK_STATE_MESSAGE(check_and_cast<Packet *>(msg), sender);
    }
    else
        ASSERT(false);
}

void LinkStateRouting::handleMessageWhenDown(cMessage *msg)
{
    // Like ~RsvpTe (see its own handleMessageWhenDown for the full rationale),
    // ~LinkStateRouting is wired directly into the Ipv4 protocol dispatch -- there is no
    // socket to close on stop, so a peer that hasn't yet heard about this node's shutdown
    // can still deliver a "link state" flood message after this module has already gone
    // down (found empirically: rsvpte_graceful_shutdown.test's downstream neighbor's
    // in-flight LinkStateMsg arrives shortly after the shutdown). The base class's default
    // handleMessageWhenDown() assumes that cannot happen and throws; there is nothing
    // useful this module can do with such a message (its peer snapshot is already
    // cleared), so just drop it, the same way the data plane drops traffic for a downed
    // protocol. A self-message arriving while down would still indicate a real bug (e.g. a
    // timer clear() failed to cancel), so that diagnostic is preserved via the base class.
    if (msg->isSelfMessage()) {
        RoutingProtocolBase::handleMessageWhenDown(msg);
        return;
    }
    EV_INFO << "LinkStateRouting is down, dropping '" << msg->getName() << "'" << endl;
    delete msg;
}

void LinkStateRouting::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    printSignalBanner(signalID, obj, details);

    ASSERT(signalID == tedChangedSignal);

    EV_INFO << "TED changed\n";

    const TedChangeInfo *d = check_and_cast<const TedChangeInfo *>(obj);

    unsigned int k = d->getTedLinkIndicesArraySize();

    ASSERT(k > 0);

    // build linkinfo list
    std::vector<TeLinkStateInfo> links;
    for (unsigned int i = 0; i < k; i++) {
        unsigned int index = d->getTedLinkIndices(i);

        tedmod->updateTimestamp(index);
        links.push_back(tedmod->getLink(index));
    }

    sendToPeers(links, false, Ipv4Address());
}

void LinkStateRouting::processLINK_STATE_MESSAGE(Packet *pk, Ipv4Address sender)
{
    EV_INFO << "received LINK_STATE message from " << sender << endl;

    const auto& msg = pk->peekAtFront<LinkStateMsg>();
    TeLinkStateInfoVector forward;

    unsigned int n = msg->getLinkInfoArraySize();

    bool change = false; // in topology

    // loop through every link in the message
    for (unsigned int i = 0; i < n; i++) {
        const TeLinkStateInfo& link = msg->getLinkInfo(i);

        TeLinkStateInfo *match;

        // process link if we haven't seen this already and timestamp is newer
        if (tedmod->checkLinkValidity(link, match)) {
            ASSERT(link.sourceId == link.advrouter.getInt());

            EV_INFO << "new information found" << endl;

            if (!match) {
                // and we have no info on this link so far, store it as it is
                tedmod->addLink(link);
                change = true;
            }
            else {
                // copy over the information from it
                if (match->state != link.state) {
                    match->state = link.state;
                    change = true;
                }
                match->messageId = link.messageId;
                match->sourceId = link.sourceId;
                match->timestamp = link.timestamp;
                for (int i = 0; i < 8; i++)
                    match->UnResvBandwidth[i] = link.UnResvBandwidth[i];
                match->MaxBandwidth = link.MaxBandwidth;
                match->metric = link.metric;
                // D3: TE attributes ride along with the rest of the link-state
                // record, same as metric/bandwidth above (all current examples
                // leave these at their all-zero defaults, so this is a no-op
                // in practice today; it matters once linkAttributes is used).
                match->teMetric = link.teMetric;
                match->adminGroup = link.adminGroup;
                match->srlgsCount = link.srlgsCount;
                for (unsigned int i = 0; i < link.srlgsCount; i++)
                    match->srlgs[i] = link.srlgs[i];
            }

            forward.push_back(link);
        }
    }

    if (change)
        tedmod->rebuildRoutingTable();

    if (msg->getRequest()) {
        sendToPeer(sender, tedmod->getLinks(), false);
    }

    if (forward.size() > 0) {
        sendToPeers(forward, false, sender);
    }

    delete pk;
}

void LinkStateRouting::sendToPeers(const std::vector<TeLinkStateInfo>& list, bool req, Ipv4Address exceptPeer)
{
    EV_INFO << "sending LINK_STATE message to peers" << endl;

    // send one LinkStateMsg per own link (advrouter==routerId), to every peer except exceptPeer
    for (auto& elem : tedmod->getLinks()) {
        if (elem.advrouter != routerId)
            continue;

        if (elem.linkid == exceptPeer)
            continue;

        if (!elem.state)
            continue;

        if (!contains(peerIfAddrs, elem.local))
            continue;

        // send a copy
        sendToPeer(elem.linkid, list, req);
    }
}

void LinkStateRouting::sendToPeer(Ipv4Address peer, const std::vector<TeLinkStateInfo>& list, bool req)
{
    EV_INFO << "sending LINK_STATE message to " << peer << endl;

    Packet *pk = new Packet("link state");
    const auto& out = makeShared<LinkStateMsg>();

    out->setLinkInfoArraySize(list.size());
    for (unsigned int j = 0; j < list.size(); j++)
        out->setLinkInfo(j, list[j]);

    out->setRequest(req);
    // Message header (4: command + flags + count) + one 164-byte TeLinkStateInfo
    // record per link (see LinkStateRoutingSerializer.cc's format v2 layout,
    // which this formula must stay in sync with -- the serializer is the
    // source of truth). Was 116 bytes/record (format v1) before D3 added the
    // teMetric/adminGroup/srlgs fields; before any serializer existed it was a
    // bare B(72) * count with no header and a per-record width that didn't
    // correspond to any real field encoding.
    B length = B(4) + B(164) * out->getLinkInfoArraySize();
    out->setChunkLength(length);
    pk->insertAtBack(out);

    sendToIP(pk, peer);
}

void LinkStateRouting::sendToIP(Packet *msg, Ipv4Address destAddr)
{
    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::linkStateRouting);
    msg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::linkStateRouting);
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    msg->addTagIfAbsent<L3AddressReq>()->setDestAddress(destAddr);
    msg->addTagIfAbsent<L3AddressReq>()->setSrcAddress(routerId);
    send(msg, "ipOut");
}

} // namespace inet

