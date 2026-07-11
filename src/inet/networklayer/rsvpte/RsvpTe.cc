//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/rsvpte/RsvpTe.h"

#include <cstdlib>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/XMLUtils.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/pcep/Pcc.h"
#include "inet/networklayer/rsvpte/Utils.h"
#include "inet/networklayer/ted/Ted.h"

namespace inet {

#define PATH_ERR_UNFEASIBLE        1
#define PATH_ERR_PREEMPTED         2
#define PATH_ERR_NEXTHOP_FAILED    3

// ResvErr error codes (model-local, like the PATH_ERR_* codes above -- not RFC 2205
// Appendix D wire values, just this switch's own dispatch key). Reservation-time
// failures only (RFC 2205 Section 3.5): admission failure at Path/PSB creation time
// is reported via PathErr/PATH_ERR_UNFEASIBLE instead, since that happens before any
// reservation exists to report a ResvErr about.
#define RESV_ERR_PREEMPTED         1

namespace {

// RFC 2205 Section 3.1.2: the common RSVP message header is 8 bytes; every
// object is a 4-byte object header plus a body. The per-object byte counts
// below are the OBJECT TOTAL (header + body) for the C-Types this model
// implements (RFC 3209 LSP_TUNNEL_IPV4 for SESSION/SENDER_TEMPLATE, RFC 2210
// Int-Serv Controlled-Load for SENDER_TSPEC/FLOWSPEC, RFC 2205 IPv4 for
// ERROR_SPEC). These are used to compute honest (non-guessed) message
// lengths; they are intentionally kept object-granular so a future wire
// serializer (Workstream E) can validate against them.
constexpr int RSVP_COMMON_HEADER_BYTES = 8;
constexpr int SESSION_OBJECT_BYTES = 16;
constexpr int RSVP_HOP_OBJECT_BYTES = 12;
constexpr int TIME_VALUES_OBJECT_BYTES = 8;
constexpr int LABEL_REQUEST_OBJECT_BYTES = 8;
constexpr int SENDER_TEMPLATE_OBJECT_BYTES = 12; // also used for FILTER_SPEC (same C-Type)
// RFC 2210 Section 3.1 (SENDER_TSPEC) / 3.2.1 (FLOWSPEC, Controlled-Load): the
// Int-Serv content is exactly 8 32-bit words (message header + per-service
// header + per-parameter header + the 5 token-bucket parameters r/b/p/m/M) =
// 32 bytes, plus the 4-byte outer RSVP object header = 36. Corrected from an
// earlier 40 (Workstream E, Phase 2 commit 3): the wire serializer's object
// layout table exposed a 1-word (4-byte) overcount against the RFC diagram --
// verified directly against RFC 2210 text, not re-derived from guesswork.
constexpr int SENDER_TSPEC_OBJECT_BYTES = 36;
constexpr int FLOWSPEC_OBJECT_BYTES = 36;
constexpr int LABEL_OBJECT_BYTES = 8;
constexpr int STYLE_OBJECT_BYTES = 8;
constexpr int ERROR_SPEC_OBJECT_BYTES = 12;
constexpr int HELLO_OBJECT_BYTES = 12;
constexpr int ERO_RRO_OBJECT_HEADER_BYTES = 4;
constexpr int ERO_RRO_SUBOBJECT_BYTES = 8; // IPv4 prefix subobject
// RFC 2961 Section 5.4 (refresh reduction, Workstream C8): MESSAGE_ID / MESSAGE_ID_ACK
// object header(4) + Epoch(4) + Message_Identifier(4) = 12 bytes each; only ever
// present when the sending router's "refreshReduction" parameter is on.
constexpr int MESSAGE_ID_OBJECT_BYTES = 12;
constexpr int MESSAGE_ID_ACK_OBJECT_BYTES = 12;

B computeEroLength(const EroVector& ero)
{
    return ero.empty() ? B(0) : B(ERO_RRO_OBJECT_HEADER_BYTES + ERO_RRO_SUBOBJECT_BYTES * ero.size());
}

B computeRroLength(const Ipv4AddressVector& rro)
{
    return rro.empty() ? B(0) : B(ERO_RRO_OBJECT_HEADER_BYTES + ERO_RRO_SUBOBJECT_BYTES * rro.size());
}

// One FILTER_SPEC + FLOWSPEC + LABEL (+ RRO if present) per flow, shared by
// Resv (commit 10) and ResvTear/ResvErr (commit 12), all of which carry a
// FlowDescriptorVector.
B computeFlowDescriptorListLength(const FlowDescriptorVector& flows)
{
    B length = B(0);
    for (auto& flow : flows)
        length += B(SENDER_TEMPLATE_OBJECT_BYTES) + B(FLOWSPEC_OBJECT_BYTES) + B(LABEL_OBJECT_BYTES) + computeRroLength(flow.RRO);
    return length;
}

B computeHelloMessageLength()
{
    return B(RSVP_COMMON_HEADER_BYTES + HELLO_OBJECT_BYTES);
}

// C8: 0 bytes when absent -- same "omitted entirely" convention as computeEroLength()/
// computeRroLength() above.
B computeMessageIdLength(bool hasMessageId)
{
    return hasMessageId ? B(MESSAGE_ID_OBJECT_BYTES) : B(0);
}

B computeMessageIdAckLength(bool hasMessageIdAck)
{
    return hasMessageIdAck ? B(MESSAGE_ID_ACK_OBJECT_BYTES) : B(0);
}

B computePathMessageLength(const EroVector& ero, bool hasMessageId = false, bool hasMessageIdAck = false)
{
    return B(RSVP_COMMON_HEADER_BYTES + SESSION_OBJECT_BYTES + RSVP_HOP_OBJECT_BYTES + TIME_VALUES_OBJECT_BYTES
             + LABEL_REQUEST_OBJECT_BYTES + SENDER_TEMPLATE_OBJECT_BYTES + SENDER_TSPEC_OBJECT_BYTES)
           + computeEroLength(ero) + computeMessageIdLength(hasMessageId) + computeMessageIdAckLength(hasMessageIdAck);
}

// C8: an Srefresh's own length -- CommonHdr + MESSAGE_ID, plus an optional piggybacked
// MESSAGE_ID_ACK/_NACK.
B computeSrefreshMessageLength(bool hasMessageIdAck)
{
    return B(RSVP_COMMON_HEADER_BYTES) + computeMessageIdLength(true) + computeMessageIdAckLength(hasMessageIdAck);
}

B computePathTearMessageLength()
{
    // The model's PathTear carries SESSION, RSVP_HOP and SENDER_TEMPLATE only (no SENDER_TSPEC).
    return B(RSVP_COMMON_HEADER_BYTES + SESSION_OBJECT_BYTES + RSVP_HOP_OBJECT_BYTES + SENDER_TEMPLATE_OBJECT_BYTES);
}

B computePathErrorMessageLength()
{
    // RFC 2205 A.4: PathErr carries SESSION, ERROR_SPEC and a sender descriptor
    // (SENDER_TEMPLATE + SENDER_TSPEC); no RSVP_HOP.
    return B(RSVP_COMMON_HEADER_BYTES + SESSION_OBJECT_BYTES + ERROR_SPEC_OBJECT_BYTES
             + SENDER_TEMPLATE_OBJECT_BYTES + SENDER_TSPEC_OBJECT_BYTES);
}

B computeResvMessageLength(const FlowDescriptorVector& flows, bool hasMessageId = false, bool hasMessageIdAck = false)
{
    return B(RSVP_COMMON_HEADER_BYTES + SESSION_OBJECT_BYTES + RSVP_HOP_OBJECT_BYTES + TIME_VALUES_OBJECT_BYTES + STYLE_OBJECT_BYTES)
           + computeFlowDescriptorListLength(flows) + computeMessageIdLength(hasMessageId) + computeMessageIdAckLength(hasMessageIdAck);
}

B computeResvTearMessageLength(const FlowDescriptorVector& flows)
{
    // RFC 2205 A.5: no TIME_VALUES in ResvTear (unlike Resv).
    return B(RSVP_COMMON_HEADER_BYTES + SESSION_OBJECT_BYTES + RSVP_HOP_OBJECT_BYTES + STYLE_OBJECT_BYTES)
           + computeFlowDescriptorListLength(flows);
}

B computeResvErrorMessageLength(const FlowDescriptorVector& flows)
{
    // RFC 2205 A.7: SESSION, RSVP_HOP, ERROR_SPEC, STYLE, flow descriptor list; no TIME_VALUES.
    return B(RSVP_COMMON_HEADER_BYTES + SESSION_OBJECT_BYTES + RSVP_HOP_OBJECT_BYTES + ERROR_SPEC_OBJECT_BYTES + STYLE_OBJECT_BYTES)
           + computeFlowDescriptorListLength(flows);
}

} // namespace

Define_Module(RsvpTe);

simsignal_t RsvpTe::lspEstablishedSignal = registerSignal("lspEstablished");
simsignal_t RsvpTe::psbCountSignal = registerSignal("psbCount");
simsignal_t RsvpTe::rsbCountSignal = registerSignal("rsbCount");

using namespace xmlutils;

RsvpTe::RsvpTe()
{
}

void RsvpTe::emitPsbCount()
{
    emit(psbCountSignal, (long)PSBList.size());
}

void RsvpTe::emitRsbCount()
{
    emit(rsbCountSignal, (long)RSBList.size());
}

RsvpTe::~RsvpTe()
{
    // TODO cancelAndDelete timers in all data structures
    for (auto& psb : PSBList) {
        cancelAndDelete(psb.timerMsg);
        cancelAndDelete(psb.timeoutMsg);
    }
    for (auto& rsb : RSBList) {
        cancelAndDelete(rsb.refreshTimerMsg);
        cancelAndDelete(rsb.commitTimerMsg);
        cancelAndDelete(rsb.timeoutMsg);
    }
    for (auto& hello : HelloList) {
        cancelAndDelete(hello.timer);
        cancelAndDelete(hello.timeout);
    }
    if (reoptimizeTimerMsg)
        cancelAndDelete(reoptimizeTimerMsg);
}

void RsvpTe::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        tedmod.reference(this, "tedModule", true);
        rt.reference(this, "routingTableModule", true);
        ift.reference(this, "interfaceTableModule", true);
        lt.reference(this, "libTableModule", true);
        rpct.reference(this, "classifierModule", true);

        maxPsbId = 0;
        maxRsbId = 0;
        maxSrcInstance = 0;
        maxLspId = 0;
        refreshInterval = par("refreshInterval");
        stateLifetimeFactor = par("stateLifetimeFactor");
        retryInterval = par("retryInterval");
        advertiseImplicitNull = par("advertiseImplicitNull");
        computeEro = par("computeEro");
        computationMode = par("computationMode").stdstringValue();
        if (computationMode == "pce")
            pccmod.reference(this, "pccModule", true);
        reoptimizeInterval = par("reoptimizeInterval");
        if (reoptimizeInterval > SIMTIME_ZERO)
            reoptimizeTimerMsg = new ReoptimizeTimerMsg("reoptimize timer");
        refreshReduction = par("refreshReduction");
        maxMessageId = 0;
        messageIdEpoch = 0;
        // C8: a fresh epoch each run (like a real router's would change across
        // restarts). Only drawn when refreshReduction is actually on -- an
        // unconditional RNG draw here would consume a random number on every run
        // regardless of this feature, shifting every subsequent uniform(0.5, 1.5) *
        // refreshInterval jitter draw in refreshPath()/refreshResv() and moving the
        // fingerprint of every unrelated (refreshReduction=false) example/showcase.
        if (refreshReduction)
            messageIdEpoch = static_cast<uint32_t>(intuniform(1, 0x7fffffff));
        WATCH(maxPsbId);
        WATCH(maxRsbId);
        WATCH(maxSrcInstance);
        WATCH(maxLspId);
        WATCH(maxMessageId);
        WATCH(refreshInterval);
        WATCH(stateLifetimeFactor);
        WATCH(retryInterval);
        WATCH(routerId);
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        // process traffic configuration
        readTrafficFromXML(par("traffic"));
        registerProtocol(Protocol::rsvpTe, gate("ipOut"), gate("ipIn"));
    }
}

int RsvpTe::getInLabel(const SessionObj& session, const SenderTemplateObj& sender)
{
    unsigned int index;
    ResvStateBlock *rsb = findRSB(session, sender, index);
    if (!rsb)
        return -1;

    return rsb->inLabelVector[index];
}

void RsvpTe::createPath(const SessionObj& session, const SenderTemplateObj& sender)
{
    if (findPSB(session, sender)) {
        EV_INFO << "path (PSB) already exists, doing nothing" << endl;
        return;
    }

    // find entry in traffic database

    auto sit = findSession(session);

    if (sit == traffic.end()) {
        EV_INFO << "session not found in traffic database, path won't be created" << endl;
        return;
    }

    auto pit = findPath(&(*sit), sender);

    if (pit == sit->paths.end()) {
        EV_INFO << "path doesn't belong to this session according to our database, doing nothing" << endl;
        return;
    }

    PathStateBlock *psb = createIngressPSB(*sit, *pit);
    if (psb) {
        // PSB successfully created, send path message downstream
        scheduleRefreshTimer(psb, 0.0);
    }
    else {
        EV_INFO << "ingress PSB couln't be created" << endl;

        // inform the owner of this path
        sendPathNotify(pit->owner, sit->sobj, pit->sender, PATH_UNFEASIBLE, 0.0);

        // remove non-permanent path
        if (!pit->permanent) {
            EV_INFO << "removing path from traffic database" << endl;

            sit->paths.erase(pit);
        }
        else {
            EV_INFO << "path is permanent, we will try again later" << endl;

            sendPathNotify(getId(), sit->sobj, pit->sender, PATH_RETRY, retryInterval);
        }
    }
}

void RsvpTe::readTrafficFromXML(const cXMLElement *traffic)
{
    ASSERT(traffic);
    ASSERT(!strcmp(traffic->getTagName(), "sessions"));
    checkTags(traffic, "session");
    cXMLElementList list = traffic->getChildrenByTagName("session");
    for (auto& elem : list)
        readTrafficSessionFromXML(elem);
}

EroVector RsvpTe::readTrafficRouteFromXML(const cXMLElement *route)
{
    checkTags(route, "node lnode");

    EroVector ERO;

    for (cXMLElement *hop = route->getFirstChild(); hop; hop = hop->getNextSibling()) {
        EroObj h;
        if (!strcmp(hop->getTagName(), "node")) {
            h.L = false;
            h.node = L3AddressResolver().resolve(hop->getNodeValue()).toIpv4();
        }
        else if (!strcmp(hop->getTagName(), "lnode")) {
            h.L = true;
            h.node = L3AddressResolver().resolve(hop->getNodeValue()).toIpv4();
        }
        else {
            ASSERT(false);
        }
        ERO.push_back(h);
    }

    return ERO;
}

void RsvpTe::readTrafficSessionFromXML(const cXMLElement *session)
{
    checkTags(session, "tunnel_id endpoint setup_pri holding_pri paths");

    TrafficSession newSession;

    newSession.sobj.Tunnel_Id = getParameterIntValue(session, "tunnel_id");
    newSession.sobj.Extended_Tunnel_Id = routerId.getInt();
    newSession.sobj.DestAddress = getParameterIPAddressValue(session, "endpoint");

    auto sit = findSession(newSession.sobj);

    bool merge;

    if (sit != traffic.end()) {
        // session already exits, add new paths

        merge = true;

        ASSERT(!getUniqueChildIfExists(session, "holding_pri") || getParameterIntValue(session, "holding_pri") == sit->sobj.holdingPri);
        ASSERT(!getUniqueChildIfExists(session, "setup_pri") || getParameterIntValue(session, "setup_pri") == sit->sobj.setupPri);

        newSession.sobj.setupPri = sit->sobj.setupPri;
        newSession.sobj.holdingPri = sit->sobj.holdingPri;

        sit->sobj = newSession.sobj;
    }
    else {
        // session not found, create new

        merge = false;

        newSession.sobj.setupPri = getParameterIntValue(session, "setup_pri", 7);
        newSession.sobj.holdingPri = getParameterIntValue(session, "holding_pri", 7);
    }

    const cXMLElement *paths = getUniqueChild(session, "paths");
    checkTags(paths, "path");

    cXMLElementList list = paths->getChildrenByTagName("path");
    for (auto path : list) {
        checkTags(path, "sender lspid bandwidth route permanent owner include_any exclude_any "
                        "burst peak_rate min_policed_unit max_packet_size");

        int lspid = getParameterIntValue(path, "lspid");

        // C7: keep the make-before-break Lsp_Id allocator (maxLspId) past every
        // operator-configured id, whether it arrives via the initial traffic.xml or a later
        // add-session, so a generated replacement id can never collide with one of these.
        if (lspid > maxLspId)
            maxLspId = lspid;

        std::vector<TrafficPath>::iterator pit;

        TrafficPath newPath;

        newPath.sender.SrcAddress = getParameterIPAddressValue(path, "sender", routerId);
        newPath.sender.Lsp_Id = lspid;

        // make sure path doesn't exist yet

        if (merge) {
            pit = findPath(&(*sit), newPath.sender);
            if (pit != sit->paths.end()) {
                EV_DETAIL << "path " << lspid << " already exists in this session, doing nothing" << endl;
                continue;
            }
        }
        else {
            pit = findPath(&newSession, newPath.sender);
            if (pit != newSession.paths.end()) {
                EV_INFO << "path " << lspid << " already exists in this session, doing nothing" << endl;
                continue;
            }
        }

        const char *str = getParameterStrValue(path, "owner", "");
        if (strlen(str)) {
            cModule *mod = getModuleByPath(str);
            newPath.owner = mod->getId();
        }
        else {
            newPath.owner = getId();
        }

        newPath.permanent = getParameterBoolValue(path, "permanent", true);

        newPath.tspec.req_bandwidth = getParameterDoubleValue(path, "bandwidth", 0.0);

        // C11 (RFC 2210 token-bucket Tspec): carried/propagated/recorded end to end
        // (see IntServ.msg's SenderTspecObj) but NOT consulted by admission control,
        // which stays req_bandwidth-only (RsvpTe::doCACCheck()) -- see RsvpTe.ned's
        // doc comment for this model limitation. Default 0 when absent, like every
        // other optional path XML attribute here.
        newPath.tspec.burst = getParameterDoubleValue(path, "burst", 0.0);
        newPath.tspec.peakRate = getParameterDoubleValue(path, "peak_rate", 0.0);
        newPath.tspec.minPolicedUnit = getParameterIntValue(path, "min_policed_unit", 0);
        newPath.tspec.maxPacketSize = getParameterIntValue(path, "max_packet_size", 0);

        const cXMLElement *route = getUniqueChildIfExists(path, "route");
        if (route)
            newPath.ERO = readTrafficRouteFromXML(route);

        // C6/D3: optional CSPF affinity constraints, hex-encoded admin-group bitmasks (e.g.
        // "0x3"); only consulted by createIngressPSB() when computeEro is on and this path's
        // ERO ends up empty/all-loose. strtoul's base-0 auto-detects the "0x" prefix (same
        // parsing Ted::initializeTED() already uses for the "linkAttributes" adminGroup XML
        // attribute).
        newPath.includeAny = strtoul(getParameterStrValue(path, "include_any", "0"), nullptr, 0);
        newPath.excludeAny = strtoul(getParameterStrValue(path, "exclude_any", "0"), nullptr, 0);

        if (merge) {
            EV_INFO << "adding new path into an existing session" << endl;

            sit->paths.push_back(newPath);
        }
        else {
            EV_INFO << "adding new path into new session" << endl;

            newSession.paths.push_back(newPath);
        }

        // schedule path creation

        sendPathNotify(getId(), newSession.sobj, newPath.sender, PATH_RETRY, 0.0);
    }

    if (!merge) {
        EV_INFO << "adding new session into database" << endl;

        traffic.push_back(newSession);
    }
}

std::vector<RsvpTe::TrafficPath>::iterator RsvpTe::findPath(TrafficSession *session, const SenderTemplateObj& sender)
{
    auto it = session->paths.begin();
    for (; it != session->paths.end(); it++) {
        if (it->sender == sender)
            break;
    }
    return it;
}

// Recursively tests whether a network node contains an RSVP-TE module.
static bool moduleContainsRsvp(cModule *module)
{
    for (cModule::SubmoduleIterator it(module); !it.end(); ++it) {
        cModule *submodule = *it;
        if (dynamic_cast<RsvpTe *>(submodule) != nullptr)
            return true;
        if (moduleContainsRsvp(submodule))
            return true;
    }
    return false;
}

bool RsvpTe::peerRunsRsvp(Ipv4Address peerInterface)
{
    cModule *peerNode = L3AddressResolver().findHostWithAddress(L3Address(peerInterface));
    return peerNode != nullptr && moduleContainsRsvp(peerNode);
}

void RsvpTe::setupHello()
{
    routerId = rt->getRouterId();

    helloInterval = par("helloInterval");
    helloTimeout = par("helloTimeout");

    const char *peers = par("peers");
    if (!strcmp(peers, "auto")) {
        // Derive the RSVP peers automatically from the Traffic Engineering
        // Database (TED), which has already discovered every directly-connected
        // neighbour by walking the topology -- analogous to how a real RSVP-TE
        // speaker learns its adjacencies from the IGP-TE. Set up a HELLO with each
        // directly-connected neighbour that actually runs RSVP (this skips plain
        // IP routers and end hosts, which would never answer a HELLO).
        for (auto& link : tedmod->getLinks()) {
            if (link.advrouter != routerId) // not one of our local links
                continue;
            if (link.linkid.isUnspecified()) // neighbour has no router id (e.g. a host)
                continue;
            if (!peerRunsRsvp(link.remote)) // neighbour does not speak RSVP
                continue;
            addHelloPeer(link.linkid);
        }
    }
    else {
        cStringTokenizer tokenizer(peers);
        const char *token;
        while ((token = tokenizer.nextToken()) != nullptr) {
            Ipv4Address peer = tedmod->getPeerByLocalAddress(CHK(ift->findInterfaceByName(token))->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
            addHelloPeer(peer);
        }
    }
}

void RsvpTe::addHelloPeer(Ipv4Address peer)
{
    HelloState h;

    h.timer = new HelloTimerMsg("hello timer");
    h.timer->setPeer(peer);

    h.timeout = new HelloTimeoutMsg("hello timeout");
    h.timeout->setPeer(peer);

    h.peer = peer;

    if (helloInterval > 0.0) {
        // peer is down until we know he is ok

        h.ok = false;
    }
    else {
        // don't use HELLO at all, consider all peers running all the time

        h.ok = true;
    }

    HelloList.push_back(h);

    if (helloInterval > 0.0) {
        startHello(peer, exponential(helloInterval));
    }
}

void RsvpTe::startHello(Ipv4Address peer, simtime_t delay)
{
    EV_INFO << "scheduling hello start in " << delay << " seconds" << endl;

    HelloState *h = findHello(peer);
    ASSERT(h);

    ASSERT(!h->timer->isScheduled());
    ASSERT(!h->timeout->isScheduled());
    ASSERT(!h->ok);

    h->srcInstance = ++maxSrcInstance;
    h->dstInstance = 0;
    h->request = true;
    h->ack = false;

    scheduleAfter(delay, h->timer);
}

void RsvpTe::removeHello(HelloState *h)
{
    cancelEvent(h->timeout);
    cancelEvent(h->timer);

    delete h->timeout;
    delete h->timer;

    for (auto it = HelloList.begin(); it != HelloList.end(); it++) {
        if (it->peer == h->peer) {
            HelloList.erase(it);
            return;
        }
    }
    ASSERT(false);
}

void RsvpTe::sendPathNotify(int handler, const SessionObj& session, const SenderTemplateObj& sender, int status, simtime_t delay)
{
    if (handler < 0)
        return; // handler not specified

    cModule *mod = getSimulation()->getModule(handler);

    if (!mod)
        return; // handler no longer exists

    PathNotifyMsg *msg = new PathNotifyMsg("path notify");

    msg->setSession(session);
    msg->setSender(sender);
    msg->setStatus(status);

    if (handler == getId())
        scheduleAfter(delay, msg);
    else
        sendDirect(msg, delay, 0, mod, "from_rsvp");
}

void RsvpTe::processHELLO_TIMEOUT(HelloTimeoutMsg *msg)
{
    Ipv4Address peer = msg->getPeer();

    EV_INFO << "hello timeout, considering " << peer << " failed" << endl;

    // update hello state (set to failed and turn hello off)

    HelloState *hello = findHello(peer);
    ASSERT(hello);
    hello->ok = false;
    ASSERT(!hello->timeout->isScheduled());
    cancelEvent(hello->timer);

    // report the link as down; Ted decides whether that's actually a
    // change (and rebuilds/announces accordingly)

    unsigned int index = tedmod->linkIndex(routerId, peer);
    tedmod->setLinkState(index, false);

    // send PATH_ERROR for existing paths

    for (auto& elem : PSBList) {
        if (elem.OutInterface == tedmod->getLink(index).local)
            sendPathErrorMessage(&(elem), PATH_ERR_NEXTHOP_FAILED);
    }
}

void RsvpTe::processHELLO_TIMER(HelloTimerMsg *msg)
{
    Ipv4Address peer = msg->getPeer();

    HelloState *h = findHello(peer);
    ASSERT(h);

    Packet *pk = new Packet("hello message");
    const auto& hMsg = makeShared<RsvpHelloMsg>();

    hMsg->setSrcInstance(h->srcInstance);
    hMsg->setDstInstance(h->dstInstance);

    hMsg->setRequest(h->request);
    hMsg->setAck(h->ack);

    hMsg->setChunkLength(computeHelloMessageLength());
    pk->insertAtBack(hMsg);

    sendToIP(pk, peer);

    h->ack = false;

    scheduleAfter(helloInterval, msg);
}

void RsvpTe::processPSB_TIMER(PsbTimerMsg *msg)
{
    PathStateBlock *psb = findPsbById(msg->getId());
    ASSERT(psb);

    // C8 (RFC 2961 refresh reduction): once this PSB already has an assigned
    // outbound MESSAGE_ID (from an earlier full send -- the very first call here
    // after PSB creation always takes the refreshPath() branch, since outMessageId
    // starts at 0), subsequent periodic refreshes are compressed into a Srefresh.
    // When refreshReduction is off, outMessageId is never assigned in the first
    // place, so this is behaviorally identical to the unconditional refreshPath()
    // call it replaces.
    if (refreshReduction && psb->outMessageId != 0)
        sendPathSrefresh(psb);
    else
        refreshPath(psb);
    scheduleRefreshTimer(psb, uniform(0.5, 1.5) * refreshInterval);
}

void RsvpTe::processPSB_TIMEOUT(PsbTimeoutMsg *msg)
{
    PathStateBlock *psb = findPsbById(msg->getId());
    ASSERT(psb);

    if (tedmod->isLocalAddress(psb->OutInterface)) {
        ASSERT(psb->OutInterface == tedmod->getInterfaceAddrByPeerAddress(psb->ERO[0].node));

        sendPathTearMessage(psb->ERO[0].node, psb->sessionObject,
                psb->Sender_Template_Object, psb->OutInterface, routerId, false);
    }

    removePSB(psb);
}

void RsvpTe::processRSB_REFRESH_TIMER(RsbRefreshTimerMsg *msg)
{
    ResvStateBlock *rsb = findRsbById(msg->getId());
    if (rsb->commitTimerMsg->isScheduled()) {
        // reschedule after commit
        scheduleRefreshTimer(rsb, 0.0);
    }
    else {
        refreshResv(rsb);

        scheduleRefreshTimer(rsb, uniform(0.5, 1.5) * refreshInterval);
    }
}

void RsvpTe::processRSB_COMMIT_TIMER(RsbCommitTimerMsg *msg)
{
    ResvStateBlock *rsb = findRsbById(msg->getId());
    commitResv(rsb);
}

void RsvpTe::processRSB_TIMEOUT(RsbTimeoutMsg *msg)
{
    EV_INFO << "RSB TIMEOUT RSB " << msg->getId() << endl;

    ResvStateBlock *rsb = findRsbById(msg->getId());

    ASSERT(rsb);
    ASSERT(tedmod->isLocalAddress(rsb->OI));

    // RFC 2205 Section 3.5: notify upstream neighbors before removing the state they refreshed
    sendResvTearMessage(rsb);

    for (unsigned int i = 0; i < rsb->FlowDescriptor.size(); i++) {
        removeRsbFilter(rsb, 0);
    }
    removeRSB(rsb);
}

bool RsvpTe::doCACCheck(const SessionObj& session, const SenderTspecObj& tspec, Ipv4Address OI)
{
    ASSERT(tedmod->isLocalAddress(OI));

    int k = tedmod->linkIndex(OI);

    double sharedBW = 0.0;

    for (auto& elem : RSBList) {
        // SE-style sharing is per outgoing link: only reservations of the same
        // session on the SAME link (OI) may share bandwidth with this request
        if ((elem.OI == OI) && (elem.sessionObject == session) && (elem.Flowspec_Object.req_bandwidth > sharedBW))
            sharedBW = elem.Flowspec_Object.req_bandwidth;
    }

    EV_DETAIL << "CACCheck: link=" << OI
              << " requested=" << tspec.req_bandwidth
              << " shared=" << sharedBW
              << " available (immediately)=" << tedmod->getLink(k).UnResvBandwidth[7]
              << " available (preemptible)=" << tedmod->getLink(k).UnResvBandwidth[session.setupPri] << endl;

    return tedmod->getLink(k).UnResvBandwidth[session.setupPri] + sharedBW >= tspec.req_bandwidth;
}

void RsvpTe::refreshPath(PathStateBlock *psbEle)
{
    EV_INFO << "refresh path (PSB " << psbEle->id << ")" << endl;

    Ipv4Address& OI = psbEle->OutInterface;
    EroVector& ERO = psbEle->ERO;

    ASSERT(!OI.isUnspecified());
    ASSERT(tedmod->isLocalAddress(OI));

    Packet *pk = new Packet("Path");
    const auto& pm = makeShared<RsvpPathMsg>();

    pm->setSession(psbEle->sessionObject);
    pm->setSenderTemplate(psbEle->Sender_Template_Object);
    pm->setSenderTspec(psbEle->Sender_Tspec_Object);

    RsvpHopObj hop;
    hop.Logical_Interface_Handle = OI;
    hop.Next_Hop_Address = routerId;
    pm->setHop(hop);

    pm->setERO(ERO);

    // C8 (RFC 2961 refresh reduction): attach this PSB's own MESSAGE_ID (assigning
    // one on first use) so that later periodic refreshes can be compressed into a
    // Srefresh instead (see processPSB_TIMER()); also piggyback any ACK/NACK owed to
    // this PSB's downstream peer.
    if (refreshReduction) {
        if (psbEle->outMessageId == 0)
            psbEle->outMessageId = ++maxMessageId;
        pm->setHasMessageId(true);
        pm->setMessageIdEpoch(messageIdEpoch);
        pm->setMessageId(psbEle->outMessageId);
    }
    if (psbEle->hasPendingAckOut) {
        pm->setHasMessageIdAck(true);
        pm->setMessageIdNack(psbEle->pendingAckOut.isNack);
        pm->setAckedMessageIdEpoch(psbEle->pendingAckOut.epoch);
        pm->setAckedMessageId(psbEle->pendingAckOut.messageId);
        psbEle->hasPendingAckOut = false;
    }

    pm->setChunkLength(computePathMessageLength(ERO, pm->getHasMessageId(), pm->getHasMessageIdAck()));
    pk->insertAtBack(pm);

    Ipv4Address nextHop = tedmod->getPeerByLocalAddress(OI);

    ASSERT(ERO.size() == 0 || ERO[0].node.equals(nextHop) || ERO[0].L);

    sendToIP(pk, nextHop);
}

// C8 (RFC 2961 Section 5.3): compressed counterpart of refreshPath(), sent once
// this PSB already has an assigned outMessageId. Carries no session/ERO/Tspec at
// all -- the receiver resolves it purely from (this PSB's downstream peer address,
// epoch, id); see processSrefreshMsg().
void RsvpTe::sendPathSrefresh(PathStateBlock *psbEle)
{
    ASSERT(refreshReduction && psbEle->outMessageId != 0);

    EV_INFO << "sending SREFRESH (PSB " << psbEle->id << ") messageId=" << psbEle->outMessageId << endl;

    Packet *pk = new Packet("Srefresh");
    const auto& msg = makeShared<RsvpSrefreshMsg>();

    msg->setMessageIdEpoch(messageIdEpoch);
    msg->setMessageId(psbEle->outMessageId);

    if (psbEle->hasPendingAckOut) {
        msg->setHasMessageIdAck(true);
        msg->setMessageIdNack(psbEle->pendingAckOut.isNack);
        msg->setAckedMessageIdEpoch(psbEle->pendingAckOut.epoch);
        msg->setAckedMessageId(psbEle->pendingAckOut.messageId);
        psbEle->hasPendingAckOut = false;
    }

    msg->setChunkLength(computeSrefreshMessageLength(msg->getHasMessageIdAck()));
    pk->insertAtBack(msg);

    Ipv4Address nextHop = tedmod->getPeerByLocalAddress(psbEle->OutInterface);
    sendToIP(pk, nextHop);
}

void RsvpTe::refreshResv(ResvStateBlock *rsbEle)
{
    EV_INFO << "refresh reservation (RSB " << rsbEle->id << ")" << endl;

    Ipv4AddressVector phops;

    for (auto& elem : PSBList) {
        if (elem.OutInterface != rsbEle->OI)
            continue;

        for (auto& _i : rsbEle->FlowDescriptor) {
            if ((FilterSpecObj&)elem.Sender_Template_Object != _i.Filter_Spec_Object)
                continue;

            if (tedmod->isLocalAddress(elem.previousHopAddress))
                continue; // IR nothing to refresh

            if (!contains(phops, elem.previousHopAddress))
                phops.push_back(elem.previousHopAddress);
        }

        // C8 (RFC 2961 refresh reduction): once a phop already has an assigned
        // outbound MESSAGE_ID (from a previous full send), subsequent periodic
        // refreshes to it are compressed into a Srefresh instead. When
        // refreshReduction is off, "refreshReduction && ..." is always false, so
        // this is behaviorally identical to the unconditional refreshResv(rsbEle,
        // phop) call it replaces.
        for (auto& phop : phops) {
            if (refreshReduction && rsbEle->outMessageIdByPhop.count(phop))
                sendResvSrefresh(rsbEle, phop);
            else
                refreshResv(rsbEle, phop);
        }
    }
}

void RsvpTe::refreshResv(ResvStateBlock *rsbEle, Ipv4Address PHOP)
{
    EV_INFO << "refresh reservation (RSB " << rsbEle->id << ") PHOP " << PHOP << endl;

    Packet *pk = new Packet("    Resv");
    const auto& msg = makeShared<RsvpResvMsg>();

    FlowDescriptorVector flows;

    msg->setSession(rsbEle->sessionObject);

    RsvpHopObj hop;
    hop.Logical_Interface_Handle = tedmod->peerRemoteInterface(PHOP);
    hop.Next_Hop_Address = PHOP;
    msg->setHop(hop);

    for (auto& elem : PSBList) {
        if (elem.previousHopAddress != PHOP)
            continue;

        if (elem.sessionObject != rsbEle->sessionObject)
            continue;

        for (unsigned int c = 0; c < rsbEle->FlowDescriptor.size(); c++) {
            if ((FilterSpecObj&)elem.Sender_Template_Object != rsbEle->FlowDescriptor[c].Filter_Spec_Object)
                continue;

            ASSERT(rsbEle->inLabelVector.size() == rsbEle->FlowDescriptor.size());

            FlowDescriptor_t flow;
            flow.Filter_Spec_Object = (FilterSpecObj&)elem.Sender_Template_Object;
            flow.Flowspec_Object = (FlowSpecObj&)elem.Sender_Tspec_Object;
            flow.RRO = rsbEle->FlowDescriptor[c].RRO;
            flow.RRO.push_back(routerId);
            flow.label = rsbEle->inLabelVector[c];
            flows.push_back(flow);

            break;
        }
    }

    msg->setFlowDescriptor(flows);

    // C8: attach this (RSB, phop)'s own MESSAGE_ID (assigning one on first use) and
    // any ACK/NACK owed to "phop" for a MESSAGE_ID it attached to a Path it sent us
    // (see the matching PSB's hasInMessageId/processPathMsg()).
    if (refreshReduction) {
        auto it = rsbEle->outMessageIdByPhop.find(PHOP);
        if (it == rsbEle->outMessageIdByPhop.end())
            it = rsbEle->outMessageIdByPhop.emplace(PHOP, ++maxMessageId).first;
        msg->setHasMessageId(true);
        msg->setMessageIdEpoch(messageIdEpoch);
        msg->setMessageId(it->second);
    }
    auto ackIt = rsbEle->pendingAcksByPhop.find(PHOP);
    if (ackIt != rsbEle->pendingAcksByPhop.end()) {
        msg->setHasMessageIdAck(true);
        msg->setMessageIdNack(ackIt->second.isNack);
        msg->setAckedMessageIdEpoch(ackIt->second.epoch);
        msg->setAckedMessageId(ackIt->second.messageId);
        rsbEle->pendingAcksByPhop.erase(ackIt);
    }

    msg->setChunkLength(computeResvMessageLength(flows, msg->getHasMessageId(), msg->getHasMessageIdAck()));
    pk->insertAtBack(msg);

    sendToIP(pk, PHOP);
}

// C8 (RFC 2961 Section 5.3): compressed counterpart of refreshResv(rsb, PHOP), sent
// once this (RSB, phop) pair already has an assigned outbound MESSAGE_ID. Carries no
// session/flow descriptors at all -- the receiver resolves it purely from (this
// router's address as seen by "PHOP", epoch, id); see processSrefreshMsg().
void RsvpTe::sendResvSrefresh(ResvStateBlock *rsbEle, Ipv4Address PHOP)
{
    auto it = rsbEle->outMessageIdByPhop.find(PHOP);
    ASSERT(refreshReduction && it != rsbEle->outMessageIdByPhop.end());

    EV_INFO << "sending SREFRESH (RSB " << rsbEle->id << ") PHOP " << PHOP << " messageId=" << it->second << endl;

    Packet *pk = new Packet("Srefresh");
    const auto& msg = makeShared<RsvpSrefreshMsg>();

    msg->setMessageIdEpoch(messageIdEpoch);
    msg->setMessageId(it->second);

    auto ackIt = rsbEle->pendingAcksByPhop.find(PHOP);
    if (ackIt != rsbEle->pendingAcksByPhop.end()) {
        msg->setHasMessageIdAck(true);
        msg->setMessageIdNack(ackIt->second.isNack);
        msg->setAckedMessageIdEpoch(ackIt->second.epoch);
        msg->setAckedMessageId(ackIt->second.messageId);
        rsbEle->pendingAcksByPhop.erase(ackIt);
    }

    msg->setChunkLength(computeSrefreshMessageLength(msg->getHasMessageIdAck()));
    pk->insertAtBack(msg);

    sendToIP(pk, PHOP);
}

// RFC 2205 Section 3.5: when soft state expires (or is torn down), send ResvTear
// upstream to every previous hop for this RSB's flows, mirroring refreshResv()'s
// PHOP discovery exactly (matching PSB entries whose OutInterface is this RSB's OI).
void RsvpTe::sendResvTearMessage(ResvStateBlock *rsbEle)
{
    Ipv4AddressVector phops;

    for (auto& elem : PSBList) {
        if (elem.OutInterface != rsbEle->OI)
            continue;

        for (auto& _i : rsbEle->FlowDescriptor) {
            if ((FilterSpecObj&)elem.Sender_Template_Object != _i.Filter_Spec_Object)
                continue;

            if (tedmod->isLocalAddress(elem.previousHopAddress))
                continue; // IR: nothing further upstream to tear

            if (!contains(phops, elem.previousHopAddress))
                phops.push_back(elem.previousHopAddress);
        }

        for (auto& phop : phops)
            sendResvTearMessage(rsbEle, phop);
    }
}

void RsvpTe::sendResvTearMessage(ResvStateBlock *rsbEle, Ipv4Address PHOP)
{
    EV_INFO << "sending ResvTear (RSB " << rsbEle->id << ") PHOP " << PHOP << endl;

    Packet *pk = new Packet("ResvTear");
    const auto& msg = makeShared<RsvpResvTear>();

    FlowDescriptorVector flows;

    msg->setSession(rsbEle->sessionObject);

    RsvpHopObj hop;
    hop.Logical_Interface_Handle = tedmod->peerRemoteInterface(PHOP);
    hop.Next_Hop_Address = PHOP;
    msg->setHop(hop);

    for (auto& elem : PSBList) {
        if (elem.previousHopAddress != PHOP)
            continue;

        if (elem.sessionObject != rsbEle->sessionObject)
            continue;

        for (auto& flow : rsbEle->FlowDescriptor) {
            if ((FilterSpecObj&)elem.Sender_Template_Object != flow.Filter_Spec_Object)
                continue;

            flows.push_back(flow);
            break;
        }
    }

    msg->setFlowDescriptor(flows);
    msg->setChunkLength(computeResvTearMessageLength(flows));
    pk->insertAtBack(msg);

    sendToIP(pk, PHOP);
}

// RFC 2205 Section 3.5: notify the receiver (downstream, via this RSB's OI -- the
// same direction Path/Resv-refresh travel) that its reservation failed at this hop.
void RsvpTe::sendResvErrorMessage(ResvStateBlock *rsbEle, int errCode)
{
    ASSERT(!rsbEle->OI.isUnspecified());

    Ipv4Address nextHop = tedmod->getPeerByLocalAddress(rsbEle->OI);

    EV_INFO << "sending ResvErr (RSB " << rsbEle->id << ") to " << nextHop << endl;

    Packet *pk = new Packet("ResvErr");
    const auto& msg = makeShared<RsvpResvError>();

    msg->setSession(rsbEle->sessionObject);

    RsvpHopObj hop;
    hop.Logical_Interface_Handle = rsbEle->OI;
    hop.Next_Hop_Address = routerId;
    msg->setHop(hop);

    msg->setErrorNode(routerId);
    msg->setErrorCode(errCode);

    msg->setFlowDescriptor(rsbEle->FlowDescriptor);
    msg->setChunkLength(computeResvErrorMessageLength(rsbEle->FlowDescriptor));
    pk->insertAtBack(msg);

    sendToIP(pk, nextHop);
}

void RsvpTe::preempt(Ipv4Address OI, int priority, double bandwidth)
{
    ASSERT(tedmod->isLocalAddress(OI));

    unsigned int index = tedmod->linkIndex(OI);

    for (auto& elem : RSBList) {
        if (elem.OI != OI)
            continue;

        if (elem.sessionObject.holdingPri != priority)
            continue;

        if (elem.Flowspec_Object.req_bandwidth == 0.0)
            continue;

        // preempt RSB

        EV_INFO << "preempting RSB " << elem.id << " (holding priority " << priority
                << ", releasing " << elem.Flowspec_Object.req_bandwidth << ")" << endl;

        for (int i = priority; i < 8; i++)
            tedmod->adjustUnresvBandwidth(index, i, elem.Flowspec_Object.req_bandwidth);

        bandwidth -= elem.Flowspec_Object.req_bandwidth;
        elem.Flowspec_Object.req_bandwidth = 0.0;

        // RFC 2205 Section 3.5: tell the receiver its reservation just died
        sendResvErrorMessage(&(elem), RESV_ERR_PREEMPTED);

        scheduleCommitTimer(&(elem));

        //

        if (bandwidth <= 0.0)
            break;
    }
}

bool RsvpTe::allocateResource(Ipv4Address OI, const SessionObj& session, double bandwidth)
{
    if (OI.isUnspecified())
        return true;

    if (!tedmod->isLocalAddress(OI))
        return true;

    if (bandwidth == 0.0)
        return true;

    int setupPri = session.setupPri;
    int holdingPri = session.holdingPri;

    unsigned int index = tedmod->linkIndex(OI);

    // Note: UnRB[7] <= UnRW[setupPri] <= UnRW[holdingPri] <= BW[0]
    // UnRW[7] is the actual BW left on the link

    if (tedmod->getLink(index).UnResvBandwidth[setupPri] < bandwidth)
        return false;

    for (int p = holdingPri; p < 8; p++) {
        tedmod->adjustUnresvBandwidth(index, p, -bandwidth);

        if (tedmod->getLink(index).UnResvBandwidth[p] < 0.0)
            preempt(OI, p, -tedmod->getLink(index).UnResvBandwidth[p]);
    }

    // announce changes (bandwidth-only; not a liveness flip, so this doesn't
    // go through Ted::setLinkState())

    tedmod->announceLinkChange(index);

    return true;
}

void RsvpTe::commitResv(ResvStateBlock *rsb)
{
    EV_INFO << "commit reservation (RSB " << rsb->id << ")" << endl;

    // allocate bandwidth as needed

    EV_INFO << "currently allocated: " << rsb->Flowspec_Object << endl;

    while (true) {
        // remove RSB if empty

        if (rsb->FlowDescriptor.size() == 0) {
            removeRSB(rsb);
            return;
        }

        FlowSpecObj req;
        unsigned int maxFlowIndex = 0;
        req.req_bandwidth = rsb->FlowDescriptor[0].Flowspec_Object.req_bandwidth;

        for (unsigned int i = 1; i < rsb->FlowDescriptor.size(); i++) {
            if (rsb->FlowDescriptor[i].Flowspec_Object.req_bandwidth > req.req_bandwidth) {
                req.req_bandwidth = rsb->FlowDescriptor[i].Flowspec_Object.req_bandwidth;
                maxFlowIndex = i;
            }
        }

        EV_INFO << "currently required: " << req << endl;

        double needed = req.req_bandwidth - rsb->Flowspec_Object.req_bandwidth;

        if (needed != 0.0) {
            if (allocateResource(rsb->OI, rsb->sessionObject, needed)) {
                // allocated (deallocated) successfully

                EV_DETAIL << "additional bandwidth of " << needed << " allocated sucessfully" << endl;

                rsb->Flowspec_Object.req_bandwidth += needed;
            }
            else {
                // bandwidth not available

                ASSERT(rsb->inLabelVector.size() == rsb->FlowDescriptor.size());

                EV_DETAIL << "not enough bandwidth to accommodate this RSB" << endl;

                int lspid = rsb->FlowDescriptor[maxFlowIndex].Filter_Spec_Object.Lsp_Id;
                int oldInLabel = rsb->inLabelVector[maxFlowIndex];
                PathStateBlock *psb = findPSB(rsb->sessionObject, (SenderTemplateObj&)rsb->FlowDescriptor[maxFlowIndex].Filter_Spec_Object);

                EV_DETAIL << "removing filter lspid=" << lspid << " (max. flow)" << endl;

                rsb->FlowDescriptor.erase(rsb->FlowDescriptor.begin() + maxFlowIndex);
                rsb->inLabelVector.erase(rsb->inLabelVector.begin() + maxFlowIndex);

                if (oldInLabel != -1) {
                    // path already existed, this must be preemption

                    sendPathErrorMessage(psb, PATH_ERR_PREEMPTED);

                    // an implicit-null advertisement never allocated a LIB entry
                    if (oldInLabel != IMPLICIT_NULL_LABEL)
                        lt->removeLibEntry(oldInLabel);
                }
                else {
                    // path not established yet, report as unfeasible

                    sendPathErrorMessage(psb, PATH_ERR_UNFEASIBLE);
                }

                continue;
            }
        }

        break;
    }

    // install labels into lib

    // C7 (make-before-break): if this commitResv() call is the one that installs a pending
    // MBB replacement's ingress label for the first time, remember the cutover to perform --
    // but only ACT on it after the loop below has fully finished. Doing it mid-loop (e.g.
    // calling removePSB() on the OLD psb right away) could erase a PSB and shift PSBList,
    // invalidating the `psb` pointer this same loop still uses for OTHER flow indices.
    int cutoverOldLspId = -1;
    SessionObj cutoverSession;
    SenderTemplateObj cutoverNewSender;
    int cutoverNewInLabel = -1;

    for (unsigned int i = 0; i < rsb->FlowDescriptor.size(); i++) {
        int lspid = rsb->FlowDescriptor[i].Filter_Spec_Object.Lsp_Id;

        EV_DETAIL << "processing lspid=" << lspid << endl;

        PathStateBlock *psb = findPSB(rsb->sessionObject, rsb->FlowDescriptor[i].Filter_Spec_Object);

        LabelOpVector outLabel;
        int inInterfaceId, outInterfaceId;

        bool IR = (psb->previousHopAddress == routerId);
        if (!IR) {
            Ipv4Address localInf = tedmod->getInterfaceAddrByPeerAddress(psb->previousHopAddress);
            inInterfaceId = rt->getInterfaceByAddress(localInf)->getInterfaceId();
        }
        else
            inInterfaceId = LibTable::ANY_INTERFACE; // ingress LSR: the label is pushed, not matched against an incoming interface

        // outlabel and outgoing interface

        LabelOp lop;
        int inLabel;

        if (tedmod->isLocalAddress(psb->OutInterface)) {
            // regular next hop

            if (rsb->FlowDescriptor[i].label == IMPLICIT_NULL_LABEL) {
                // penultimate hop popping: our downstream peer for this FEC
                // advertised the implicit null label, so we must pop rather
                // than push or swap to label 3
                ASSERT(!IR); // the ingress must never push the implicit null label
                lop.optcode = POP_OPER;
            }
            else {
                lop.optcode = IR ? PUSH_OPER : SWAP_OPER;
                lop.label = rsb->FlowDescriptor[i].label;
            }
            outLabel.push_back(lop);

            outInterfaceId = rt->getInterfaceByAddress(psb->OutInterface)->getInterfaceId();

            EV_DETAIL << "installing label for " << lspid << " outLabel=" << outLabel
                      << " outInterface=" << outInterfaceId << endl;

            ASSERT(rsb->inLabelVector.size() == rsb->FlowDescriptor.size());

            inLabel = lt->installLibEntry(rsb->inLabelVector[i], inInterfaceId,
                        outLabel, outInterfaceId);

            ASSERT(inLabel >= 0);
        }
        else {
            // egress router

            outInterfaceId = CHK(ift->findInterfaceByName("lo0"))->getInterfaceId();

            if (!tedmod->isLocalAddress(psb->sessionObject.DestAddress)) {
                NetworkInterface *ie = rt->getInterfaceForDestAddr(psb->sessionObject.DestAddress);
                if (ie)
                    outInterfaceId = ie->getInterfaceId();
            }

            if (advertiseImplicitNull) {
                // penultimate hop popping: advertise the implicit null label
                // instead of allocating a local pop entry -- our upstream
                // peer must pop the label itself, so no labeled traffic for
                // this tunnel should ever reach us
                EV_INFO << "advertising implicit null label (penultimate hop popping) for lspid=" << lspid << endl;
                inLabel = IMPLICIT_NULL_LABEL;
            }
            else {
                lop.label = 0;
                lop.optcode = POP_OPER;
                outLabel.push_back(lop);

                EV_DETAIL << "installing label for " << lspid << " outLabel=" << outLabel
                          << " outInterface=" << outInterfaceId << endl;

                ASSERT(rsb->inLabelVector.size() == rsb->FlowDescriptor.size());

                inLabel = lt->installLibEntry(rsb->inLabelVector[i], inInterfaceId,
                            outLabel, outInterfaceId);

                ASSERT(inLabel >= 0);
            }
        }

        if (IR && rsb->inLabelVector[i] == -1) {
            // path established
            sendPathNotify(psb->handler, psb->sessionObject, psb->Sender_Template_Object, PATH_CREATED, 0.0);
            emit(lspEstablishedSignal, simTime() - psb->pathCreationTime);

            // C7 (make-before-break, RFC 3209 Section 4.6.4): if this newly-established
            // ingress PSB is a pending MBB replacement, this is the moment to cut traffic
            // over -- record it now (inLabel is already known at this point), act on it
            // once this loop finishes.
            auto sit = findSession(rsb->sessionObject);
            if (sit != traffic.end()) {
                auto pit = findPath(&(*sit), psb->Sender_Template_Object);
                if (pit != sit->paths.end() && pit->replacesLspId >= 0) {
                    cutoverOldLspId = pit->replacesLspId;
                    cutoverSession = rsb->sessionObject;
                    cutoverNewSender = psb->Sender_Template_Object;
                    cutoverNewInLabel = inLabel;
                    pit->replacesLspId = -1; // no longer a pending replacement -- it's the session's normal path now
                }
            }
        }

        if (rsb->inLabelVector[i] != inLabel) {
            // remember our current label
            rsb->inLabelVector[i] = inLabel;

            // bind fec
            rpct->bind(psb->sessionObject, psb->Sender_Template_Object, inLabel);
        }
    }

    if (cutoverOldLspId >= 0)
        completeMakeBeforeBreakCutover(cutoverSession, cutoverNewSender, cutoverOldLspId, cutoverNewInLabel);
}

RsvpTe::ResvStateBlock *RsvpTe::createRSB(const Ptr<const RsvpResvMsg>& msg)
{
    ResvStateBlock rsbEle;

    rsbEle.id = ++maxRsbId;

    rsbEle.timeoutMsg = new RsbTimeoutMsg("rsb timeout");
    rsbEle.timeoutMsg->setId(rsbEle.id);

    rsbEle.refreshTimerMsg = new RsbRefreshTimerMsg("rsb timer");
    rsbEle.refreshTimerMsg->setId(rsbEle.id);

    rsbEle.commitTimerMsg = new RsbCommitTimerMsg("rsb commit");
    rsbEle.commitTimerMsg->setId(rsbEle.id);

    rsbEle.sessionObject = msg->getSession();
    rsbEle.Next_Hop_Address = msg->getNHOP();
    rsbEle.OI = msg->getLIH();

    ASSERT(rsbEle.inLabelVector.size() == rsbEle.FlowDescriptor.size());

    for (auto& elem : msg->getFlowDescriptor()) {
        FlowDescriptor_t flow = elem;
        rsbEle.FlowDescriptor.push_back(flow);
        rsbEle.inLabelVector.push_back(-1);
    }

    RSBList.push_back(rsbEle);
    ResvStateBlock *rsb = &(*(RSBList.end() - 1));
    emitRsbCount();

    EV_INFO << "created new RSB " << rsb->id << endl;

    // C8 (RFC 2961 refresh reduction): seed an ACK owed to any upstream sender whose
    // Path already arrived (with a MESSAGE_ID) before this RSB existed to queue it
    // against -- the common case for a transit node, since this function only runs
    // once the first downstream Resv arrives, which is typically after the matching
    // upstream Path(s) already created their PSB(s). Without this, such a PSB's
    // pending ack would have had nowhere to go at the time (processPathMsg() only
    // gets one chance to queue it, before refreshReduction compresses further
    // refreshes into Srefresh).
    for (auto& psb : PSBList) {
        if (psb.OutInterface == rsb->OI && psb.sessionObject == rsb->sessionObject && psb.hasInMessageId)
            rsb->pendingAcksByPhop[psb.previousHopAddress] = {false, psb.inMessageIdEpoch, psb.inMessageId};
    }

    return rsb;
}

void RsvpTe::updateRSB(ResvStateBlock *rsb, const RsvpResvMsg *msg)
{
    ASSERT(rsb);

    for (auto& elem : msg->getFlowDescriptor()) {
        FlowDescriptor_t flow = elem;

        unsigned int m;
        for (m = 0; m < rsb->FlowDescriptor.size(); m++) {
            if (rsb->FlowDescriptor[m].Filter_Spec_Object == flow.Filter_Spec_Object) {
                // sender found
                EV_DETAIL << "sender (lspid=" << flow.Filter_Spec_Object.Lsp_Id << ") found in RSB" << endl;

                if (rsb->FlowDescriptor[m].label != flow.label) {
                    EV_DETAIL << "label modified (new label=" << flow.label << ")" << endl;

                    rsb->FlowDescriptor[m].label = flow.label;

                    // label must be updated in lib table

                    scheduleCommitTimer(rsb);

                    // C8 (RFC 2961 refresh reduction): this RSB's content just
                    // changed, so any already-assigned outbound MESSAGE_IDs are
                    // stale (RFC 2961 requires a fresh id whenever the content
                    // changes) -- drop them all so the next Resv send to each phop
                    // is a full one (refreshResv()'s per-phop check treats a
                    // missing map entry as "not yet assigned") and gets a fresh id.
                    rsb->outMessageIdByPhop.clear();
                }

                break;
            }
        }
        if (m == rsb->FlowDescriptor.size()) {
            // sender not found
            EV_INFO << "sender (lspid=" << flow.Filter_Spec_Object.Lsp_Id << ") not found in RSB, adding..." << endl;

            rsb->FlowDescriptor.push_back(flow);
            rsb->inLabelVector.push_back(-1);

            // resv is new and must be forwarded

            scheduleCommitTimer(rsb);
            scheduleRefreshTimer(rsb, 0.0);

            // C8: same reasoning as the label-modified branch above.
            rsb->outMessageIdByPhop.clear();
        }
    }
}

void RsvpTe::removeRsbFilter(ResvStateBlock *rsb, unsigned int index)
{
    ASSERT(rsb);
    ASSERT(index < rsb->FlowDescriptor.size());
    ASSERT(rsb->inLabelVector.size() == rsb->FlowDescriptor.size());

    int lspid = rsb->FlowDescriptor[index].Filter_Spec_Object.Lsp_Id;
    int inLabel = rsb->inLabelVector[index];

    EV_INFO << "removing filter (lspid=" << lspid << ")" << endl;

    // an implicit-null advertisement never allocated a LIB entry
    if (inLabel != -1 && inLabel != IMPLICIT_NULL_LABEL)
        lt->removeLibEntry(inLabel);

    rsb->FlowDescriptor.erase(rsb->FlowDescriptor.begin() + index);
    rsb->inLabelVector.erase(rsb->inLabelVector.begin() + index);

    scheduleCommitTimer(rsb);
}

void RsvpTe::removeRSB(ResvStateBlock *rsb)
{
    ASSERT(rsb);
    ASSERT(rsb->FlowDescriptor.size() == 0);

    EV_INFO << "removing empty RSB " << rsb->id << endl;

    cancelAndDelete(rsb->refreshTimerMsg);
    cancelAndDelete(rsb->commitTimerMsg);
    cancelAndDelete(rsb->timeoutMsg);

    if (rsb->Flowspec_Object.req_bandwidth > 0) {
        // deallocate resources
        allocateResource(rsb->OI, rsb->sessionObject, -rsb->Flowspec_Object.req_bandwidth);
    }

    for (auto it = RSBList.begin(); it != RSBList.end(); it++) {
        if (it->id == rsb->id) {
            RSBList.erase(it);
            emitRsbCount();
            return;
        }
    }
    ASSERT(false);
}

void RsvpTe::removePSB(PathStateBlock *psb)
{
    ASSERT(psb);

    int lspid = psb->Sender_Template_Object.Lsp_Id;

    EV_INFO << "removing PSB " << psb->id << " (lspid " << lspid << ")" << endl;

    // remove reservation state if exists

    unsigned int filterIndex;
    ResvStateBlock *rsb = findRSB(psb->sessionObject, psb->Sender_Template_Object, filterIndex);
    if (rsb) {
        EV_INFO << "reservation state present, will be removed too" << endl;

        removeRsbFilter(rsb, filterIndex);
    }

    // proceed with actual removal

    cancelAndDelete(psb->timerMsg);
    cancelAndDelete(psb->timeoutMsg);

    for (auto it = PSBList.begin(); it != PSBList.end(); it++) {
        if (it->id == psb->id) {
            PSBList.erase(it);
            emitPsbCount();
            return;
        }
    }
    ASSERT(false);
}

bool RsvpTe::evalNextHopInterface(Ipv4Address destAddr, const EroVector& ERO, Ipv4Address& OI)
{
    if (ERO.size() > 0) {
        // explicit routing

        if (ERO[0].L) {
            NetworkInterface *ie = rt->getInterfaceForDestAddr(ERO[0].node);

            if (!ie) {
                EV_INFO << "next (loose) hop address " << ERO[0].node << " is currently unroutable" << endl;
                return false;
            }

            OI = ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
        }
        else {
            OI = tedmod->getInterfaceAddrByPeerAddress(ERO[0].node);
        }

        Ipv4Address peer = tedmod->getPeerByLocalAddress(OI);
        HelloState *h = findHello(peer);
        if (!h) {
            // the strict ERO points at a next hop that is not an RSVP Hello peer;
            // treat the path as infeasible so the caller can answer with a PathErr
            EV_WARN << "next (strict) hop " << peer << " on interface " << OI << " is not an RSVP peer" << endl;
            return false;
        }

        // ok, only if next hop is up and running

        return h->ok;
    }
    else {
        // hop-by-hop routing

        if (!tedmod->isLocalAddress(destAddr)) {
            NetworkInterface *ie = rt->getInterfaceForDestAddr(destAddr);

            if (!ie) {
                EV_INFO << "destination address " << destAddr << " is currently unroutable" << endl;
                return false;
            }

            OI = ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress();

            HelloState *h = findHello(tedmod->getPeerByLocalAddress(OI));
            if (!h) {
                // outgoing interface is not LSR, we are egress router

                OI = Ipv4Address();

                return true;
            }
            else {
                // outgoing interface is LSR

                ASSERT(h->ok); // rt->getInterfaceForDestAddr() wouldn't choose this entry

                return h->ok;
            }
        }
        else {
            // destAddress is ours, we're egress

            return true;
        }
    }
}

RsvpTe::PathStateBlock *RsvpTe::createPSB(const Ptr<RsvpPathMsg>& msg)
{
    const EroVector& ERO = msg->getERO();
    Ipv4Address destAddr = msg->getDestAddress();

    //

    Ipv4Address OI;

    if (!evalNextHopInterface(destAddr, ERO, OI))
        return nullptr;

    if (tedmod->isLocalAddress(OI) && !doCACCheck(msg->getSession(), msg->getSenderTspec(), OI))
        return nullptr; // not enough resources

    PathStateBlock psbEle;

    psbEle.id = ++maxPsbId;

    psbEle.timeoutMsg = new PsbTimeoutMsg("psb timeout");
    psbEle.timeoutMsg->setId(psbEle.id);

    psbEle.timerMsg = new PsbTimerMsg("psb timer");
    psbEle.timerMsg->setId(psbEle.id);

    psbEle.sessionObject = msg->getSession();
    psbEle.Sender_Template_Object = msg->getSenderTemplate();
    psbEle.Sender_Tspec_Object = msg->getSenderTspec();

    psbEle.previousHopAddress = msg->getNHOP();

    psbEle.OutInterface = OI;
    psbEle.ERO = ERO;

    psbEle.handler = -1;
    psbEle.pathCreationTime = simTime();

    PSBList.push_back(psbEle);
    PathStateBlock *cPSB = &(*(PSBList.end() - 1));
    emitPsbCount();

    EV_INFO << "created new PSB " << cPSB->id << endl;

    return cPSB;
}

RsvpTe::PathStateBlock *RsvpTe::createIngressPSB(const TrafficSession& session, const TrafficPath& path)
{
    EroVector ERO = path.ERO;

    while (ERO.size() > 0 && ERO[0].node == routerId) {
        // remove ourselves from the beginning of the hop list
        ERO.erase(ERO.begin());
    }

    // C6: if the configured ERO gives no real source-routing constraint -- it's empty, or
    // every remaining hop is loose (<lnode>, i.e. just a waypoint hint, never a strict hop) --
    // and CSPF is enabled, compute a full strict ERO to the destination ourselves instead of
    // falling back to plain hop-by-hop routing. A partially-strict ERO (at least one <node>)
    // is left untouched: the operator asked for a specific route, CSPF doesn't override it.
    bool allLoose = true;
    for (auto& hop : ERO)
        if (!hop.L)
            allLoose = false;

    if (computeEro && (ERO.empty() || allLoose)) {
        if (computationMode == "pce") {
            // Workstream F4 Phase 2: delegate to a co-located Pcc instead of running
            // Ted::calculateShortestPath() ourselves. requestPathComputation() bridges
            // its inherently asynchronous PCReq/PCRep TCP exchange onto this synchronous-
            // looking call (see Pcc.h's doc comment): PENDING/NO_PATH both mean "no ERO
            // available on THIS attempt", handled below by falling through to the exact
            // same nullptr-return, retry-via-createPath() contract the local-CSPF branch
            // above already relies on for "no path found yet" -- there is no separate
            // code path to build here.
            EroVector computedEro;
            Pcc::PceComputationResult result = pccmod->requestPathComputation(routerId, session.sobj.DestAddress,
                    path.tspec.req_bandwidth, session.sobj.setupPri, path.includeAny, path.excludeAny, computedEro);

            switch (result) {
                case Pcc::PceComputationResult::COMPUTED:
                    ERO = computedEro;
                    EV_DETAIL << "PCE computed ERO for session (dest=" << session.sobj.DestAddress
                              << ", lspid=" << path.sender.Lsp_Id << "): " << vectorToString(ERO) << endl;
                    break;

                case Pcc::PceComputationResult::PENDING:
                    EV_INFO << "PCReq outstanding for session (dest=" << session.sobj.DestAddress
                            << ", lspid=" << path.sender.Lsp_Id << "), awaiting the PCE's PCRep" << endl;
                    return nullptr;

                case Pcc::PceComputationResult::NO_PATH:
                default:
                    EV_INFO << "PCE reported no feasible path to " << session.sobj.DestAddress
                            << " (or the PCEP session is not yet OPERATIONAL) for session (lspid="
                            << path.sender.Lsp_Id << ")" << endl;
                    return nullptr;
            }
        }
        else {
            Ipv4AddressVector dest;
            dest.push_back(session.sobj.DestAddress);

            Ipv4AddressVector cspfPath = tedmod->calculateShortestPath(dest, tedmod->getLinks(),
                    path.tspec.req_bandwidth, session.sobj.setupPri, path.includeAny, path.excludeAny);

            if (cspfPath.empty()) {
                EV_INFO << "CSPF found no feasible path to " << session.sobj.DestAddress
                        << " (bandwidth=" << path.tspec.req_bandwidth
                        << ", priority=" << session.sobj.setupPri << ")" << endl;

                // No path found: reuse the existing pathProblem()/retry contract. There is no PSB
                // yet to hand to pathProblem(), so just return nullptr -- createPath(), our only
                // caller, already does exactly the right thing with that: a permanent path is
                // retried after retryInterval (PATH_RETRY notify), a non-permanent one is dropped
                // from the traffic database.
                return nullptr;
            }

            // cspfPath[0] is this router itself (the CSPF root); the ERO -- like a hand-written
            // one at this point -- carries only the hops AFTER us. CSPF always yields a full
            // STRICT route (every hop L=false).
            EroVector computedEro;
            for (unsigned int i = 1; i < cspfPath.size(); i++) {
                EroObj hop;
                hop.L = false;
                hop.node = cspfPath[i];
                computedEro.push_back(hop);
            }
            ERO = computedEro;

            EV_DETAIL << "CSPF computed ERO for session (dest=" << session.sobj.DestAddress
                      << ", lspid=" << path.sender.Lsp_Id << "): " << vectorToString(ERO) << endl;
        }
    }

    Ipv4Address OI;

    if (!evalNextHopInterface(session.sobj.DestAddress, ERO, OI))
        return nullptr;

    if (!doCACCheck(session.sobj, path.tspec, OI))
        return nullptr;

    EV_INFO << "CACCheck passed, creating PSB" << endl;

    PathStateBlock psbEle;
    psbEle.id = ++maxPsbId;

    psbEle.timeoutMsg = new PsbTimeoutMsg("psb timeout");
    psbEle.timeoutMsg->setId(psbEle.id);

    psbEle.timerMsg = new PsbTimerMsg("psb timer");
    psbEle.timerMsg->setId(psbEle.id);

    psbEle.sessionObject = session.sobj;
    psbEle.Sender_Template_Object = path.sender;
    psbEle.Sender_Tspec_Object = path.tspec;

    psbEle.previousHopAddress = routerId;

    psbEle.OutInterface = OI;
    psbEle.ERO = ERO;

    psbEle.handler = path.owner;
    psbEle.pathCreationTime = simTime();

    PSBList.push_back(psbEle);
    PathStateBlock *cPSB = &(*(PSBList.end() - 1));
    emitPsbCount();

    return cPSB;
}

RsvpTe::ResvStateBlock *RsvpTe::createEgressRSB(PathStateBlock *psb)
{
    ResvStateBlock rsbEle;

    rsbEle.id = ++maxRsbId;

    rsbEle.timeoutMsg = new RsbTimeoutMsg("rsb timeout");
    rsbEle.timeoutMsg->setId(rsbEle.id);

    rsbEle.refreshTimerMsg = new RsbRefreshTimerMsg("rsb timer");
    rsbEle.refreshTimerMsg->setId(rsbEle.id);

    rsbEle.commitTimerMsg = new RsbCommitTimerMsg("rsb commit");
    rsbEle.commitTimerMsg->setId(rsbEle.id);

    rsbEle.sessionObject = psb->sessionObject;
    rsbEle.Next_Hop_Address = psb->previousHopAddress;

    rsbEle.OI = psb->OutInterface;

    FlowDescriptor_t flow;
    flow.Flowspec_Object = (FlowSpecObj&)psb->Sender_Tspec_Object;
    flow.Filter_Spec_Object = (FilterSpecObj&)psb->Sender_Template_Object;
    flow.label = -1;

    rsbEle.FlowDescriptor.push_back(flow);
    rsbEle.inLabelVector.push_back(-1);

    RSBList.push_back(rsbEle);
    ResvStateBlock *rsb = &(*(RSBList.end() - 1));
    emitRsbCount();

    EV_INFO << "created new (egress) RSB " << rsb->id << endl;

    return rsb;
}

void RsvpTe::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        SignallingMsg *sMsg = check_and_cast<SignallingMsg *>(msg);
        processSignallingMessage(sMsg);
    }
    else {
        Packet *pk = check_and_cast<Packet *>(msg);
        processRSVPMessage(pk);
        return;
    }
}

void RsvpTe::processRSVPMessage(Packet *pk)
{
    const auto& msg = pk->peekAtFront<RsvpMessage>();
    int kind = msg->getRsvpKind();
    switch (kind) {
        case PATH_MESSAGE:
            processPathMsg(pk);
            break;

        case RESV_MESSAGE:
            processResvMsg(pk);
            break;

        case PTEAR_MESSAGE:
            processPathTearMsg(pk);
            break;

        case HELLO_MESSAGE:
            processHelloMsg(pk);
            break;

        case PERROR_MESSAGE:
            processPathErrMsg(pk);
            break;

        case RTEAR_MESSAGE:
            processResvTearMsg(pk);
            break;

        case RERROR_MESSAGE:
            processResvErrMsg(pk);
            break;

        case SREFRESH_MESSAGE:
            processSrefreshMsg(pk);
            break;

        default:
            throw cRuntimeError("Invalid RSVP kind of message '%s': %d", pk->getName(), kind);
    }
}

void RsvpTe::processHelloMsg(Packet *pk)
{
    EV_INFO << "Received RSVP_HELLO" << endl;
    const auto& msg = pk->peekAtFront<RsvpHelloMsg>();
    Ipv4Address sender = pk->getTag<L3AddressInd>()->getSrcAddress().toIpv4();
    Ipv4Address peer = tedmod->primaryAddress(sender);

    bool request = msg->getRequest();
    bool ack = msg->getAck();

    EV_INFO << "hello sender " << peer;
    if (request)
        EV_INFO << " REQ";
    if (ack)
        EV_INFO << " ACK";
    EV_INFO << endl;

    int rcvSrcInstance = msg->getSrcInstance();
    int rcvDstInstance = msg->getDstInstance();

    delete pk;

    HelloState *h = findHello(peer);
    if (!h) {
        // a Hello from a peer we are not maintaining a Hello session with
        // (e.g. a stale or misdirected message); ignore it
        EV_WARN << "received a Hello from unknown peer " << peer << ", ignoring" << endl;
        return;
    }

    ASSERT(h->srcInstance);
    ASSERT(rcvSrcInstance);

    bool failure = false;

    if (h->srcInstance != rcvDstInstance) {
        if (rcvDstInstance != 0) {
            failure = true;
        }
        else {
            ASSERT(request);
        }
    }

    if (h->dstInstance != rcvSrcInstance) {
        if (h->dstInstance != 0) {
            failure = true;
        }
        h->dstInstance = rcvSrcInstance;
    }

    if (failure) {
        // mismatch encountered
        h->srcInstance = ++maxSrcInstance;
    }

    if (failure || !h->ok) {
        h->ok = true;

        EV_INFO << "local peer " << peer << " is now considered up and running" << endl;

        recoveryEvent(peer);

        // if peer was considered down, we have stopped sending hellos: resume now
        if (!h->timer->isScheduled())
            scheduleAfter(SIMTIME_ZERO, h->timer);
    }

    if (request) {
        // immediately respond to a request with an ack
        h->ack = true;
        h->request = false;

        rescheduleAfter(SIMTIME_ZERO, h->timer);
    }
    else {
        // next message will be regular

        h->ack = false;
        h->request = false;

        ASSERT(h->timer->isScheduled());
    }

    rescheduleAfter(helloTimeout, h->timeout);
}

void RsvpTe::processPathErrMsg(Packet *pk)
{
    EV_INFO << "Received PATH_ERROR" << endl;

    const auto& msg = pk->peekAtFront<RsvpPathError>();
    int errCode = msg->getErrorCode();

    PathStateBlock *psb = findPSB(msg->getSession(), msg->getSenderTemplate());
    if (!psb) {
        EV_INFO << "matching PSB not found, ignoring error message" << endl;
        delete pk;
        return;
    }

    if (psb->previousHopAddress != routerId) {
        EV_INFO << "forwarding error message to PHOP (" << psb->previousHopAddress << ")" << endl;

        delete pk->removeControlInfo(); // FIXME
        pk->trim();
        sendToIP(pk, psb->previousHopAddress);
    }
    else {
        EV_INFO << "error reached ingress router" << endl;

        switch (errCode) {
            case PATH_ERR_PREEMPTED:
                sendPathNotify(psb->handler, psb->sessionObject, psb->Sender_Template_Object, PATH_PREEMPTED, 0.0);
                break;

            case PATH_ERR_UNFEASIBLE:
                sendPathNotify(psb->handler, psb->sessionObject, psb->Sender_Template_Object, PATH_UNFEASIBLE, 0.0);
                break;

            case PATH_ERR_NEXTHOP_FAILED:
                sendPathNotify(psb->handler, psb->sessionObject, psb->Sender_Template_Object, PATH_FAILED, 0.0);
                break;

            default:
                // an error code this model does not act on; log and ignore
                EV_WARN << "ignoring PathErr with unhandled error code " << errCode
                        << " in message '" << msg->getName() << "'" << endl;
                break;
        }

        delete pk;
    }
}

void RsvpTe::processPathTearMsg(Packet *pk)
{
    EV_INFO << "Received PATH_TEAR" << endl;

    const auto& msg = pk->peekAtFront<RsvpPathTear>();
    int lspid = msg->getLspId();

    PathStateBlock *psb = findPSB(msg->getSession(), msg->getSenderTemplate());
    if (!psb) {
        EV_DETAIL << "received PATH_TEAR for nonexisting lspid=" << lspid << endl;
        delete pk;
        return;
    }

    // forward path teardown downstream

    if (psb->ERO.size() > 0) {
        EV_INFO << "forward teardown downstream" << endl;

        sendPathTearMessage(psb->ERO[0].node, psb->sessionObject, psb->Sender_Template_Object,
                tedmod->getInterfaceAddrByPeerAddress(psb->ERO[0].node), routerId, msg->getForce());
    }

    // remove path state block

    removePSB(psb);

    delete pk;
}

void RsvpTe::processResvTearMsg(Packet *pk)
{
    EV_INFO << "Received RESV_TEAR" << endl;

    const auto& msg = pk->peekAtFront<RsvpResvTear>();

    // Simplification: each flow entry is processed (and, if it matches, propagated
    // upstream) independently. All flows named in one ResvTear currently resolve to
    // the same local RSB in every example/test this model exercises (one sender per
    // RSB); a true SE-style multi-sender RSB would need to dedupe the upstream send.
    for (auto& flow : msg->getFlowDescriptor()) {
        unsigned int index;
        ResvStateBlock *rsb = findRSB(msg->getSession(), flow.Filter_Spec_Object, index);
        if (!rsb) {
            EV_DETAIL << "received RESV_TEAR for lspid=" << flow.Filter_Spec_Object.Lsp_Id << " with no matching RSB, ignoring" << endl;
            continue;
        }

        EV_INFO << "tearing down reservation (RSB " << rsb->id << ")" << endl;

        // propagate the teardown further upstream before removing our own state
        // (mirrors the RSB-timeout sender path, minus the timer cancellation)
        sendResvTearMessage(rsb);

        removeRsbFilter(rsb, index);

        if (rsb->FlowDescriptor.empty())
            removeRSB(rsb);
    }

    delete pk;
}

void RsvpTe::processResvErrMsg(Packet *pk)
{
    EV_INFO << "Received RESV_ERROR" << endl;

    const auto& msg = pk->peekAtFront<RsvpResvError>();

    for (auto& flow : msg->getFlowDescriptor()) {
        unsigned int index;
        ResvStateBlock *rsb = findRSB(msg->getSession(), flow.Filter_Spec_Object, index);
        if (!rsb) {
            EV_DETAIL << "received RESV_ERROR for lspid=" << flow.Filter_Spec_Object.Lsp_Id << " with no matching RSB, ignoring" << endl;
            continue;
        }

        if (rsb->OI.isUnspecified()) {
            // egress: the reservation this receiver asked for just failed upstream
            EV_WARN << "reservation failed (errorCode=" << msg->getErrorCode() << ", errorNode=" << msg->getErrorNode()
                    << ") for lspid=" << flow.Filter_Spec_Object.Lsp_Id << ", removing local reservation state" << endl;

            removeRsbFilter(rsb, index);

            if (rsb->FlowDescriptor.empty())
                removeRSB(rsb);
        }
        else {
            // transit: forward the same error further downstream toward the egress
            EV_INFO << "forwarding ResvErr toward egress via RSB " << rsb->id << endl;

            sendResvErrorMessage(rsb, msg->getErrorCode());
        }
    }

    delete pk;
}

void RsvpTe::processPathMsg(Packet *pk)
{
    EV_INFO << "Received PATH_MESSAGE" << endl;
    auto msg = dynamicPtrCast<RsvpPathMsg>(pk->peekAtFront<RsvpPathMsg>()->dupShared());
    print(msg.get());

    // process ERO

    EroVector ERO = msg->getERO();

    while (ERO.size() > 0 && ERO[0].node == routerId) {
        ERO.erase(ERO.begin());
    }

    msg->setERO(ERO);

    // create PSB if doesn't exist yet

    PathStateBlock *psb = findPSB(msg->getSession(), msg->getSenderTemplate());

    if (!psb) {
        psb = createPSB(msg);
        if (!psb) {
            sendPathErrorMessage(msg->getSession(), msg->getSenderTemplate(),
                    msg->getSenderTspec(), msg->getNHOP(), PATH_ERR_UNFEASIBLE);
            delete pk;
            return;
        }
        scheduleRefreshTimer(psb, 0.0);

        if (tedmod->isLocalAddress(psb->OutInterface)) {
            unsigned int index = tedmod->linkIndex(psb->OutInterface);
            if (!tedmod->getLink(index).state) {
                sendPathErrorMessage(psb, PATH_ERR_NEXTHOP_FAILED);
            }
        }
    }

    // schedule timer&timeout

    scheduleTimeout(psb);

    // create RSB if we're egress and doesn't exist yet

    unsigned int index;
    ResvStateBlock *rsb = findRSB(msg->getSession(), msg->getSenderTemplate(), index);

    if (!rsb && psb->OutInterface.isUnspecified()) {
        ASSERT(ERO.size() == 0);
        rsb = createEgressRSB(psb);
        ASSERT(rsb);
        scheduleCommitTimer(rsb);
    }

    if (rsb)
        scheduleRefreshTimer(rsb, 0.0);

    // C8 (RFC 2961 refresh reduction): record the peer's MESSAGE_ID for this PSB (so
    // a later Srefresh from them naming it resolves back here -- see
    // processSrefreshMsg()) and process any piggybacked ACK/NACK for our own
    // outbound Resv-id previously sent to this same previousHopAddress. Queuing the
    // ack itself needs a matching RSB to piggyback it on: if egress, "rsb" was just
    // created above and is available immediately; for a transit node with no RSB
    // yet, createRSB() seeds it later instead (see there) -- this function only
    // ever runs with a full (uncompressed) Path, so there is no later call here to
    // retry the queuing once refreshReduction has compressed refreshes into
    // Srefresh.
    if (msg->getHasMessageId()) {
        psb->hasInMessageId = true;
        psb->inMessageIdEpoch = msg->getMessageIdEpoch();
        psb->inMessageId = msg->getMessageId();
        if (rsb)
            rsb->pendingAcksByPhop[psb->previousHopAddress] = {false, psb->inMessageIdEpoch, psb->inMessageId};
    }
    if (msg->getHasMessageIdAck())
        handleMessageIdAck(psb->previousHopAddress, msg->getMessageIdNack(), msg->getAckedMessageIdEpoch(), msg->getAckedMessageId());

    delete pk;
}

void RsvpTe::processResvMsg(Packet *pk)
{
    EV_INFO << "Received RESV_MESSAGE" << endl;
    pk->trimFront();
    auto msg = pk->removeAtFront<RsvpResvMsg>();
    print(msg.get());

    Ipv4Address OI = msg->getLIH();

    // find matching PSB for every flow

    for (unsigned int m = 0; m < msg->getFlowDescriptor().size(); m++) {
        PathStateBlock *psb = findPSB(msg->getSession(), (SenderTemplateObj&)msg->getFlowDescriptor()[m].Filter_Spec_Object);
        if (!psb) {
            EV_DETAIL << "matching PSB not found for lspid=" << msg->getFlowDescriptor()[m].Filter_Spec_Object.Lsp_Id << endl;
            // remove descriptor from message
            msg->getFlowDescriptorForUpdate().erase(msg->getFlowDescriptorForUpdate().begin() + m);
            --m;
        }
    }

    if (msg->getFlowDescriptor().size() == 0) {
        EV_INFO << "no matching PSB found" << endl;
        delete pk;
        return;
    }

    // find matching RSB

    ResvStateBlock *rsb = nullptr;
    for (auto& elem : RSBList) {
        if (!(msg->isInSession(&elem.sessionObject)))
            continue;

        if (elem.Next_Hop_Address != msg->getNHOP())
            continue;

        if (elem.OI != msg->getLIH())
            continue;

        rsb = &(elem);
        break;
    }

    if (!rsb) {
        rsb = createRSB(msg);

        scheduleCommitTimer(rsb);

        // reservation is new, propagate upstream immediately
        scheduleRefreshTimer(rsb, 0.0);
    }
    else
        updateRSB(rsb, msg.get());

    scheduleTimeout(rsb);

    // C8: record the peer's MESSAGE_ID for this RSB (so a later Srefresh from them
    // naming it resolves back here) and process any piggybacked ACK/NACK for our
    // own outbound Path-id previously sent to this same downstream peer. The
    // matching PSB is the one sharing this RSB's (session, OI) -- the same identity
    // refreshResv() itself uses to find it (rsb->Next_Hop_Address is NOT a reliable
    // peer address here: refreshResv() writes the SEND TARGET into that hop field,
    // so on receipt it reflects OUR OWN address, not the sender's -- getPeerByLocal
    // Address(rsb->OI) is this file's established way to recover the actual
    // downstream peer, e.g. sendResvErrorMessage()). The matching PSB is guaranteed
    // to already exist here -- a Resv flow with no matching PSB was already dropped
    // above.
    if (msg->getHasMessageId()) {
        rsb->hasInMessageId = true;
        rsb->inMessageIdEpoch = msg->getMessageIdEpoch();
        rsb->inMessageId = msg->getMessageId();
        for (auto& elem : PSBList) {
            if (elem.sessionObject == rsb->sessionObject && elem.OutInterface == rsb->OI) {
                elem.hasPendingAckOut = true;
                elem.pendingAckOut = {false, rsb->inMessageIdEpoch, rsb->inMessageId};
                break;
            }
        }
    }
    if (msg->getHasMessageIdAck())
        handleMessageIdAck(tedmod->getPeerByLocalAddress(rsb->OI), msg->getMessageIdNack(), msg->getAckedMessageIdEpoch(), msg->getAckedMessageId());

    delete pk;
}

// C8 (RFC 2961 Section 5.3): a compressed refresh naming (epoch, id). Resolved
// purely from (the packet's actual sender address, epoch, id) -- Srefresh carries
// no session/sender identity of its own (see RsvpSrefreshMsg.msg).
void RsvpTe::processSrefreshMsg(Packet *pk)
{
    const auto& msg = pk->peekAtFront<RsvpSrefreshMsg>();
    // Normalize the packet's actual (possibly per-interface) source address to the
    // peer's canonical router id -- the same mapping processHelloMsg() applies for
    // the same reason: previousHopAddress/routerId-based bookkeeping is keyed on
    // the peer's primary address, not necessarily the address it happened to send
    // this particular packet from.
    Ipv4Address peer = tedmod->primaryAddress(pk->getTag<L3AddressInd>()->getSrcAddress().toIpv4());
    uint32_t epoch = msg->getMessageIdEpoch();
    uint32_t id = msg->getMessageId();

    EV_INFO << "Received SREFRESH from " << peer << " (epoch=" << epoch << ", id=" << id << ")" << endl;

    if (msg->getHasMessageIdAck())
        handleMessageIdAck(peer, msg->getMessageIdNack(), msg->getAckedMessageIdEpoch(), msg->getAckedMessageId());

    bool resolved = false;
    for (auto& psb : PSBList) {
        if (psb.previousHopAddress == peer && psb.hasInMessageId && psb.inMessageIdEpoch == epoch && psb.inMessageId == id) {
            EV_DETAIL << "SREFRESH recognized for PSB " << psb.id << ", refreshing soft state" << endl;
            scheduleTimeout(&psb);
            resolved = true;
            break;
        }
    }
    if (!resolved) {
        for (auto& rsb : RSBList) {
            if (tedmod->isLocalAddress(rsb.OI) && tedmod->getPeerByLocalAddress(rsb.OI) == peer
                    && rsb.hasInMessageId && rsb.inMessageIdEpoch == epoch && rsb.inMessageId == id) {
                EV_DETAIL << "SREFRESH recognized for RSB " << rsb.id << ", refreshing soft state" << endl;
                scheduleTimeout(&rsb);
                resolved = true;
                break;
            }
        }
    }

    if (!resolved) {
        EV_WARN << "SREFRESH from " << peer << " names an unrecognized messageId (epoch=" << epoch
                << ", id=" << id << "), sending a NACK" << endl;
        sendMessageIdNack(peer, epoch, id);
    }

    delete pk;
}

// C8: a positive ack is purely informational in this model (there is nothing this
// model does differently once a peer confirms receipt -- no summary-refresh-interval
// extension is modeled); a nack means "peer" does not recognize the (epoch, id) we
// last told them about, so fall back to an immediate full resend of whichever state
// (PSB or RSB-per-phop) we still have that id assigned to.
void RsvpTe::handleMessageIdAck(Ipv4Address peer, bool isNack, uint32_t epoch, uint32_t id)
{
    if (!isNack) {
        EV_DETAIL << "received MESSAGE_ID_ACK from " << peer << " (epoch=" << epoch << ", id=" << id << ")" << endl;
        return;
    }

    EV_WARN << "received MESSAGE_ID_NACK from " << peer << " (epoch=" << epoch << ", id=" << id
            << "), falling back to a full refresh" << endl;

    if (epoch != messageIdEpoch)
        return; // stale epoch (e.g. from before this router last restarted); nothing of ours to fall back on

    for (auto& psb : PSBList) {
        if (psb.outMessageId == id && tedmod->isLocalAddress(psb.OutInterface)
                && tedmod->getPeerByLocalAddress(psb.OutInterface) == peer) {
            refreshPath(&psb);
            return;
        }
    }
    for (auto& rsb : RSBList) {
        auto it = rsb.outMessageIdByPhop.find(peer);
        if (it != rsb.outMessageIdByPhop.end() && it->second == id) {
            refreshResv(&rsb, peer);
            return;
        }
    }

    EV_WARN << "MESSAGE_ID_NACK from " << peer << " names an id (" << id
            << ") this router no longer owns, ignoring" << endl;
}

// An Srefresh from "peer" named an (epoch, id) this router never assigned: queue and
// promptly send a NACK back toward them (piggybacked on whatever this router would
// send them next -- forced out now rather than waiting for the next periodic cycle,
// since RFC 2961 wants a negative acknowledgment delivered promptly). "peer" is
// resolved as either an upstream Path-sender (nacked via the Resv this router sends
// back to them) or a downstream Resv-sender (nacked via the Path this router sends
// to them); if this router has no state at all toward "peer" yet, there is no
// vehicle to carry the nack and it is dropped (a documented corner case of the
// piggyback-only scope).
void RsvpTe::sendMessageIdNack(Ipv4Address peer, uint32_t epoch, uint32_t id)
{
    for (auto& psb : PSBList) {
        if (psb.previousHopAddress != peer)
            continue;
        for (auto& rsb : RSBList) {
            if (rsb.OI == psb.OutInterface && rsb.sessionObject == psb.sessionObject) {
                rsb.pendingAcksByPhop[peer] = {true, epoch, id};
                if (refreshReduction && rsb.outMessageIdByPhop.count(peer))
                    sendResvSrefresh(&rsb, peer);
                else
                    refreshResv(&rsb, peer);
                return;
            }
        }
    }

    for (auto& rsb : RSBList) {
        if (!tedmod->isLocalAddress(rsb.OI) || tedmod->getPeerByLocalAddress(rsb.OI) != peer)
            continue;
        for (auto& psb : PSBList) {
            if (psb.sessionObject == rsb.sessionObject && psb.OutInterface == rsb.OI) {
                psb.hasPendingAckOut = true;
                psb.pendingAckOut = {true, epoch, id};
                if (refreshReduction && psb.outMessageId != 0)
                    sendPathSrefresh(&psb);
                else
                    refreshPath(&psb);
                return;
            }
        }
    }

    EV_WARN << "unrecognized SREFRESH from " << peer << " (epoch=" << epoch << ", id=" << id
            << ") but this router has no state at all toward them yet -- dropping the NACK "
               "(no vehicle to carry it; a documented corner case of the piggyback-only scope)" << endl;
}

void RsvpTe::recoveryEvent(Ipv4Address peer)
{
    // called when peer's operation is restored

    unsigned int index = tedmod->linkIndex(routerId, peer);
    tedmod->setLinkState(index, true);

    // refresh all paths towards this neighbour
    for (auto& elem : PSBList) {
        if (elem.OutInterface != tedmod->getLink(index).local)
            continue;

        scheduleRefreshTimer(&(elem), 0.0);
    }
}

void RsvpTe::processSignallingMessage(SignallingMsg *msg)
{
    int command = msg->getCommand();
    switch (command) {
        case MSG_PSB_TIMER:
            processPSB_TIMER(check_and_cast<PsbTimerMsg *>(msg));
            break;

        case MSG_PSB_TIMEOUT:
            processPSB_TIMEOUT(check_and_cast<PsbTimeoutMsg *>(msg));
            break;

        case MSG_RSB_REFRESH_TIMER:
            processRSB_REFRESH_TIMER(check_and_cast<RsbRefreshTimerMsg *>(msg));
            break;

        case MSG_RSB_COMMIT_TIMER:
            processRSB_COMMIT_TIMER(check_and_cast<RsbCommitTimerMsg *>(msg));
            break;

        case MSG_RSB_TIMEOUT:
            processRSB_TIMEOUT(check_and_cast<RsbTimeoutMsg *>(msg));
            break;

        case MSG_HELLO_TIMER:
            processHELLO_TIMER(check_and_cast<HelloTimerMsg *>(msg));
            break;

        case MSG_HELLO_TIMEOUT:
            processHELLO_TIMEOUT(check_and_cast<HelloTimeoutMsg *>(msg));
            break;

        case MSG_PATH_NOTIFY:
            processPATH_NOTIFY(check_and_cast<PathNotifyMsg *>(msg));
            break;

        case MSG_REOPTIMIZE_TIMER:
            processREOPTIMIZE_TIMER(check_and_cast<ReoptimizeTimerMsg *>(msg));
            break;

        default:
            throw cRuntimeError("Invalid command %d in message '%s'", command, msg->getName());
    }
}

void RsvpTe::pathProblem(PathStateBlock *psb)
{
    ASSERT(psb);
    ASSERT(!psb->OutInterface.isUnspecified());

    // C7 (make-before-break, RFC 3209 Section 4.6.4): copy what's needed by value up front.
    // triggerMakeBeforeBreak() below (permanent-path case) calls createPath(), which can
    // grow PSBList/traffic[].paths (both std::vectors) and reallocate them -- invalidating
    // `psb` and any iterator taken before the call. Everything after this point uses only
    // the saved copies (or freshly re-taken iterators), never `psb` past its last use below.
    SessionObj session = psb->sessionObject;
    SenderTemplateObj oldSender = psb->Sender_Template_Object;

    auto sit = findSession(session);
    ASSERT(sit != traffic.end());
    TrafficSession *s = &(*sit);

    auto pit = findPath(s, oldSender);
    ASSERT(pit != s->paths.end());

    bool permanent = pit->permanent;
    bool isReplacementInProgress = (pit->replacesLspId >= 0);
    bool hasReplacementPending = (pit->pendingReplacementLspId >= 0);

    if (permanent && !isReplacementInProgress) {
        if (hasReplacementPending) {
            // A single path problem can legitimately generate more than one PathErr
            // reaching this ingress in quick succession (e.g. a multi-hop preemption
            // re-notifies independently from each hop it preempted at) -- a replacement is
            // already being signaled/retried for this path, so there is nothing new to do.
            EV_INFO << "make-before-break replacement lspid=" << pit->pendingReplacementLspId
                    << " for lspid=" << oldSender.Lsp_Id << " is already pending, ignoring" << endl;
            return;
        }

        // Start signaling a replacement Lsp_Id NOW, and leave this (old, possibly still
        // working) PSB and its traffic-database entry completely alone -- no PathTear, no
        // retry -- until the replacement's ingress label is actually installed (see
        // commitResv()'s cutover, completeMakeBeforeBreakCutover()). If the replacement
        // itself later fails before that happens, it retries on its own (the
        // isReplacementInProgress branch below); this original path is never touched by
        // any of that.
        EV_INFO << "path problem on lspid=" << oldSender.Lsp_Id
                << ", starting make-before-break re-route (RFC 3209 Section 4.6.4)" << endl;

        triggerMakeBeforeBreak(session, oldSender);
        return;
    }

    // Either a non-permanent (best-effort) path -- never retried, MBB or not -- or this IS
    // itself a still-pending make-before-break replacement that failed before ever cutting
    // traffic over. Both fall back to the original tear-and-maybe-retry behavior. In the
    // replacement case, the (separate) path it was meant to replace is untouched.
    if (isReplacementInProgress) {
        EV_INFO << "make-before-break replacement lspid=" << oldSender.Lsp_Id
                << " failed before cutover, retrying it (the path it was meant to replace is unaffected)" << endl;
    }

    Ipv4Address nextHop = tedmod->getPeerByLocalAddress(psb->OutInterface);

    EV_INFO << "sending PathTear to " << nextHop << endl;

    sendPathTearMessage(nextHop, session, oldSender,
            tedmod->getInterfaceAddrByPeerAddress(nextHop), routerId, true);

    if (permanent) {
        EV_INFO << "this path is permanent, we will try to re-create it later" << endl;

        sendPathNotify(getId(), session, oldSender, PATH_RETRY, retryInterval);
    }
    else {
        EV_INFO << "removing path from traffic database" << endl;

        s->paths.erase(pit);
    }

    // remove path

    EV_INFO << "removing PSB" << endl;

    removePSB(psb);
}

void RsvpTe::triggerMakeBeforeBreak(const SessionObj& session, const SenderTemplateObj& oldSender)
{
    auto sit = findSession(session);
    if (sit == traffic.end())
        return; // e.g. raced with a del-session removing the whole session in between

    TrafficSession *s = &(*sit);

    auto pit = findPath(s, oldSender);
    if (pit == s->paths.end())
        return; // e.g. raced with a del-session removing just this path

    if (pit->replacesLspId >= 0 || pit->pendingReplacementLspId >= 0)
        return; // already being replaced, or already has a replacement in flight; callers
                 // already guard this, but stay defensive

    TrafficPath newPath = *pit;
    newPath.sender.Lsp_Id = ++maxLspId;
    newPath.replacesLspId = oldSender.Lsp_Id;
    newPath.pendingReplacementLspId = -1; // the new path has no replacement of its own (yet)
    // newPath.ERO is a copy of the old path's ERO (typically empty/loose when the old path
    // itself relied on computeEro): createIngressPSB() recomputes it fresh via CSPF exactly
    // as it would for a brand-new path. If the operator hand-wrote a strict ERO instead,
    // MBB deliberately does not second-guess it, same as any other retry of that path would.

    pit->pendingReplacementLspId = newPath.sender.Lsp_Id; // mark the OLD path: don't spawn another

    s->paths.push_back(newPath); // may reallocate `s->paths` -- `pit` is no longer used after this
    SenderTemplateObj newSender = newPath.sender;

    EV_INFO << "make-before-break: signaling replacement lspid=" << newSender.Lsp_Id
            << " for lspid=" << oldSender.Lsp_Id << endl;

    createPath(session, newSender);
}

void RsvpTe::completeMakeBeforeBreakCutover(const SessionObj& session, const SenderTemplateObj& newSender, int oldLspId, int newInLabel)
{
    SenderTemplateObj oldSender;
    oldSender.SrcAddress = newSender.SrcAddress;
    oldSender.Lsp_Id = oldLspId;

    EV_INFO << "make-before-break cutover: lspid=" << newSender.Lsp_Id
            << " is up, switching traffic away from lspid=" << oldLspId << endl;

    // Traffic cutover: rebind the classifier's FEC entries from the old lspid to the new
    // one. This IS the data-plane switchover; nothing else needs to change.
    rpct->rebind(session, oldSender, newSender, newInLabel);

    PathStateBlock *oldPsb = findPSB(session, oldSender);
    if (oldPsb) {
        Ipv4Address nextHop = tedmod->getPeerByLocalAddress(oldPsb->OutInterface);

        EV_INFO << "sending PathTear to " << nextHop << endl;

        sendPathTearMessage(nextHop, oldPsb->sessionObject, oldPsb->Sender_Template_Object,
                tedmod->getInterfaceAddrByPeerAddress(nextHop), routerId, true);

        removePSB(oldPsb);
    }

    auto sit = findSession(session);
    if (sit != traffic.end()) {
        auto pit = findPath(&(*sit), oldSender);
        if (pit != sit->paths.end())
            sit->paths.erase(pit);
    }
}

void RsvpTe::considerReoptimization(const SessionObj& session, const SenderTemplateObj& sender)
{
    auto sit = findSession(session);
    if (sit == traffic.end())
        return;

    auto pit = findPath(&(*sit), sender);
    if (pit == sit->paths.end() || pit->replacesLspId >= 0 || pit->pendingReplacementLspId >= 0)
        return; // gone, already mid-reroute itself, or already has a replacement in flight

    PathStateBlock *psb = findPSB(session, sender);
    if (!psb || psb->previousHopAddress != routerId || psb->OutInterface.isUnspecified())
        return; // no established ingress PSB to compare against (egress-terminated or not up yet)

    bool allStrict = !psb->ERO.empty();
    for (auto& hop : psb->ERO)
        if (hop.L)
            allStrict = false;

    if (!allStrict)
        return; // can't cheaply score a loose/empty route hop by hop; scope limitation

    double currentMetric = 0.0;
    Ipv4Address from = routerId;
    for (auto& hop : psb->ERO) {
        currentMetric += Ted::getTeMetric(tedmod->getLink(tedmod->linkIndex(from, hop.node)));
        from = hop.node;
    }

    Ipv4AddressVector dest;
    dest.push_back(session.DestAddress);

    Ipv4AddressVector cspfPath = tedmod->calculateShortestPath(dest, tedmod->getLinks(),
            psb->Sender_Tspec_Object.req_bandwidth, session.setupPri, pit->includeAny, pit->excludeAny);

    if (cspfPath.size() < 2)
        return; // no feasible alternative right now

    double candidateMetric = 0.0;
    for (unsigned int i = 1; i < cspfPath.size(); i++)
        candidateMetric += Ted::getTeMetric(tedmod->getLink(tedmod->linkIndex(cspfPath[i - 1], cspfPath[i])));

    if (candidateMetric < currentMetric) {
        EV_INFO << "reoptimization: found a strictly better path for lspid=" << sender.Lsp_Id
                << " (current metric=" << currentMetric << ", candidate metric=" << candidateMetric
                << "), starting make-before-break re-route" << endl;

        triggerMakeBeforeBreak(session, sender);
    }
}

void RsvpTe::processREOPTIMIZE_TIMER(ReoptimizeTimerMsg *msg)
{
    // Collect (session, sender) pairs first: considerReoptimization() (via
    // triggerMakeBeforeBreak()) may push a new TrafficPath into some session's `paths`
    // vector, which would invalidate an in-flight iterator over that same vector.
    std::vector<std::pair<SessionObj, SenderTemplateObj>> candidates;
    for (auto& session : traffic)
        for (auto& path : session.paths)
            if (path.replacesLspId < 0 && path.pendingReplacementLspId < 0)
                candidates.push_back({session.sobj, path.sender});

    for (auto& c : candidates)
        considerReoptimization(c.first, c.second);

    scheduleAfter(reoptimizeInterval, msg);
}

void RsvpTe::processPATH_NOTIFY(PathNotifyMsg *msg)
{
    PathStateBlock *psb;

    switch (msg->getStatus()) {
        case PATH_RETRY:
            createPath(msg->getSession(), msg->getSender());
            break;

        case PATH_UNFEASIBLE:
        case PATH_PREEMPTED:
        case PATH_FAILED:
            psb = findPSB(msg->getSession(), msg->getSender());
            if (psb)
                pathProblem(psb);
            break;

        case PATH_CREATED:
            EV_INFO << "Path successfully established" << endl;
            break;

        default:
            ASSERT(false);
            break;
    }

    delete msg;
}

std::vector<RsvpTe::TrafficSession>::iterator RsvpTe::findSession(const SessionObj& session)
{
    auto it = traffic.begin();
    for (; it != traffic.end(); it++) {
        if (it->sobj == session)
            break;
    }
    return it;
}

void RsvpTe::addSession(const cXMLElement& node)
{
    Enter_Method("addSession");

    readTrafficSessionFromXML(&node);
}

void RsvpTe::delSession(const cXMLElement& node)
{
    Enter_Method("delSession");

    checkTags(&node, "tunnel_id extended_tunnel_id endpoint paths");

    SessionObj sobj;

    sobj.Tunnel_Id = getParameterIntValue(&node, "tunnel_id");
    sobj.Extended_Tunnel_Id = getParameterIPAddressValue(&node, "extended_tunnel_id", routerId).getInt();
    sobj.DestAddress = getParameterIPAddressValue(&node, "endpoint");

    auto sit = findSession(sobj);
    ASSERT(sit != traffic.end());
    TrafficSession *session = &(*sit);

    const cXMLElement *paths = getUniqueChildIfExists(&node, "paths");
    cXMLElementList pathList;
    if (paths) {
        // only specified paths will be removed, session remains

        checkTags(paths, "path");
        pathList = paths->getChildrenByTagName("path");
    }

    for (auto it = session->paths.begin(); it != session->paths.end(); ) {
        bool remove;

        if (paths) {
            remove = false;

            for (auto& elem : pathList) {
                if (it->sender.Lsp_Id == getParameterIntValue(elem, "lspid")) {
                    // remove path from session
                    remove = true;
                    break;
                }
            }
        }
        else {
            // remove all paths

            remove = true;
        }

        if (remove) {
            PathStateBlock *psb = findPSB(session->sobj, it->sender);
            if (psb) {
                ASSERT(psb->ERO.size() > 0);

                sendPathTearMessage(psb->ERO[0].node, psb->sessionObject, psb->Sender_Template_Object,
                        tedmod->getInterfaceAddrByPeerAddress(psb->ERO[0].node), routerId, true);

                removePSB(psb);
            }

            it = session->paths.erase(it);
        }
        else
            ++it;
    }

    if (!paths) {
        traffic.erase(sit);
    }
}

void RsvpTe::processCommand(const cXMLElement& node)
{
    if (!strcmp(node.getTagName(), "add-session")) {
        addSession(node);
    }
    else if (!strcmp(node.getTagName(), "del-session")) {
        delSession(node);
    }
    else
        throw cRuntimeError("Unknown scenario command '%s'", node.getTagName());
}

void RsvpTe::sendPathTearMessage(Ipv4Address peerIP, const SessionObj& session, const SenderTemplateObj& sender, Ipv4Address LIH, Ipv4Address NHOP, bool force)
{
    Packet *pk = new Packet("PathTear");
    const auto& msg = makeShared<RsvpPathTear>();
    msg->setSenderTemplate(sender);
    msg->setSession(session);
    RsvpHopObj hop;
    hop.Logical_Interface_Handle = LIH;
    hop.Next_Hop_Address = NHOP;
    msg->setHop(hop);
    msg->setForce(force);
    msg->setChunkLength(computePathTearMessageLength());
    pk->insertAtBack(msg);

    sendToIP(pk, peerIP);
}

void RsvpTe::sendPathErrorMessage(PathStateBlock *psb, int errCode)
{
    sendPathErrorMessage(psb->sessionObject, psb->Sender_Template_Object, psb->Sender_Tspec_Object, psb->previousHopAddress, errCode);
}

void RsvpTe::sendPathErrorMessage(SessionObj session, SenderTemplateObj sender, SenderTspecObj tspec, Ipv4Address nextHop, int errCode)
{
    Packet *pk = new Packet("PathErr");
    const auto& msg = makeShared<RsvpPathError>();
    msg->setErrorCode(errCode);
    msg->setErrorNode(routerId);
    msg->setSession(session);
    msg->setSenderTemplate(sender);
    msg->setSenderTspec(tspec);

    msg->setChunkLength(computePathErrorMessageLength());
    pk->insertAtBack(msg);

    sendToIP(pk, nextHop);
}

void RsvpTe::sendToIP(Packet *msg, Ipv4Address destAddr)
{
    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::rsvpTe);
    msg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::rsvpTe);
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    msg->addTagIfAbsent<L3AddressReq>()->setDestAddress(destAddr);
    send(msg, "ipOut");
}

void RsvpTe::scheduleTimeout(PathStateBlock *psbEle)
{
    ASSERT(psbEle);
    // RFC 2205 Section 3.7: state expires after (K + 0.5) * 1.5 * R if not refreshed
    rescheduleAfter((stateLifetimeFactor + 0.5) * 1.5 * refreshInterval, psbEle->timeoutMsg);
}

void RsvpTe::scheduleRefreshTimer(PathStateBlock *psbEle, simtime_t delay)
{
    ASSERT(psbEle);

    if (psbEle->OutInterface.isUnspecified())
        return;

    if (!tedmod->isLocalAddress(psbEle->OutInterface))
        return;

    EV_DETAIL << "scheduling PSB " << psbEle->id << " refresh " << (simTime() + delay) << endl;

    rescheduleAfter(delay, psbEle->timerMsg);
}

void RsvpTe::scheduleTimeout(ResvStateBlock *rsbEle)
{
    ASSERT(rsbEle);
    // RFC 2205 Section 3.7: state expires after (K + 0.5) * 1.5 * R if not refreshed
    rescheduleAfter((stateLifetimeFactor + 0.5) * 1.5 * refreshInterval, rsbEle->timeoutMsg);
}

void RsvpTe::scheduleRefreshTimer(ResvStateBlock *rsbEle, simtime_t delay)
{
    ASSERT(rsbEle);
    rescheduleAfter(delay, rsbEle->refreshTimerMsg);
}

void RsvpTe::scheduleCommitTimer(ResvStateBlock *rsbEle)
{
    ASSERT(rsbEle);
    rescheduleAfter(SIMTIME_ZERO, rsbEle->commitTimerMsg);
}

RsvpTe::ResvStateBlock *RsvpTe::findRSB(const SessionObj& session, const SenderTemplateObj& sender, unsigned int& index)
{
    for (auto& elem : RSBList) {
        if (elem.sessionObject != session)
            continue;

        index = 0;
        for (auto fit = elem.FlowDescriptor.begin(); fit != elem.FlowDescriptor.end(); fit++) {
            if ((SenderTemplateObj&)fit->Filter_Spec_Object == sender) {
                return &(elem);
            }
            ++index;
        }

        // don't break here, may be in different (if outInterface is different)
    }
    return nullptr;
}

RsvpTe::PathStateBlock *RsvpTe::findPSB(const SessionObj& session, const SenderTemplateObj& sender)
{
    for (auto& elem : PSBList) {
        if ((elem.sessionObject == session) && (elem.Sender_Template_Object == sender))
            return &(elem);
    }
    return nullptr;
}

RsvpTe::PathStateBlock *RsvpTe::findPsbById(int id)
{
    for (auto& elem : PSBList) {
        if (elem.id == id)
            return &elem;
    }
    ASSERT(false);
    return nullptr; // prevent warning
}

RsvpTe::ResvStateBlock *RsvpTe::findRsbById(int id)
{
    for (auto& elem : RSBList) {
        if (elem.id == id)
            return &elem;
    }
    ASSERT(false);
    return nullptr; // prevent warning
}

RsvpTe::HelloState *RsvpTe::findHello(Ipv4Address peer)
{
    for (auto& elem : HelloList) {
        if (elem.peer == peer)
            return &(elem);
    }
    return nullptr;
}

bool operator==(const SessionObj& a, const SessionObj& b)
{
    return a.DestAddress == b.DestAddress &&
           a.Tunnel_Id == b.Tunnel_Id &&
           a.Extended_Tunnel_Id == b.Extended_Tunnel_Id;
    // NOTE: don't compare holdingPri and setupPri; their placement
    // into sessionObject is only for our convenience
}

bool operator!=(const SessionObj& a, const SessionObj& b)
{
    return !operator==(a, b);
}

bool operator==(const FilterSpecObj& a, const FilterSpecObj& b)
{
    return a.SrcAddress == b.SrcAddress && a.Lsp_Id == b.Lsp_Id;
}

bool operator!=(const FilterSpecObj& a, const FilterSpecObj& b)
{
    return !operator==(a, b);
}

bool operator==(const SenderTemplateObj& a, const SenderTemplateObj& b)
{
    return a.SrcAddress == b.SrcAddress && a.Lsp_Id == b.Lsp_Id;
}

bool operator!=(const SenderTemplateObj& a, const SenderTemplateObj& b)
{
    return !operator==(a, b);
}

std::ostream& operator<<(std::ostream& os, const FlowSpecObj& a)
{
    os << "{bandwidth:" << a.req_bandwidth << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SessionObj& a)
{
    os << "{tunnelId:" << a.Tunnel_Id << "  exTunnelId:" << a.Extended_Tunnel_Id
       << "  destAddr:" << a.DestAddress << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SenderTemplateObj& a)
{
    os << "{lspid:" << a.Lsp_Id << "  sender:" << a.SrcAddress << "}";
    return os;
}

void RsvpTe::print(const RsvpPathMsg *p)
{
    EV_INFO << "PATH_MESSAGE: lspid " << p->getLspId() << " ERO " << vectorToString(p->getERO()) << endl;
}

void RsvpTe::print(const RsvpResvMsg *r)
{
    EV_INFO << "RESV_MESSAGE: " << endl;
    for (auto& elem : r->getFlowDescriptor()) {
        EV_INFO << " lspid " << elem.Filter_Spec_Object.Lsp_Id
                << " label " << elem.label << endl;
    }
}

void RsvpTe::clear()
{
    while (!PSBList.empty())
        removePSB(&PSBList.front());
    while (!RSBList.empty())
        removeRSB(&RSBList.front());
    while (!HelloList.empty())
        removeHello(&HelloList.front());
    if (reoptimizeTimerMsg)
        cancelEvent(reoptimizeTimerMsg);
}

void RsvpTe::handleStartOperation(LifecycleOperation *operation)
{
    setupHello();
    if (reoptimizeTimerMsg && !reoptimizeTimerMsg->isScheduled())
        scheduleAfter(reoptimizeInterval, reoptimizeTimerMsg);
}

void RsvpTe::handleMessageWhenDown(cMessage *msg)
{
    // D8: a graceful shutdown that sent a forced PathTear puts this module into
    // NOT_OPERATING immediately (see handleStopOperation/startActiveOperationExtraTime),
    // while the node's later shutdown stages are held up briefly so the PathTear can
    // actually leave. Unlike ~Ldp/~Rip (which close their sockets on stop, so no further
    // protocol message can physically reach handleMessageWhenUp/handleMessageWhenDown
    // afterwards), ~RsvpTe is wired directly into the Ipv4 protocol dispatch by protocol
    // number -- there is no socket to close -- so a peer who hasn't yet heard about the
    // shutdown can still deliver a Hello/Path/Resv/... well after this module went down
    // (found empirically: rsvpte_graceful_shutdown.test's downstream neighbor's still-due
    // Hello arrives ~11ms after LSR1's shutdown). The base class's default
    // handleMessageWhenDown() assumes that scenario cannot happen (it only tolerates a
    // message landing in the exact same instant as the down transition, throwing
    // otherwise) -- reasonable for socket-based protocols, wrong here. There is nothing
    // useful this module can do with such a message (all local state already cleared),
    // so just drop it, the same way the data plane drops traffic for a downed protocol.
    // A self-message arriving while down would still indicate a real bug (e.g. a timer
    // clear() failed to cancel), so that diagnostic is preserved via the base class.
    if (msg->isSelfMessage()) {
        RoutingProtocolBase::handleMessageWhenDown(msg);
        return;
    }
    EV_INFO << "RsvpTe is down, dropping '" << msg->getName() << "'" << endl;
    delete msg;
}

void RsvpTe::handleStopOperation(LifecycleOperation *operation)
{
    // D8: a graceful shutdown notifies downstream peers so they can tear down their
    // reservation state immediately, instead of having to wait out the RFC 2205
    // refresh/timeout interval for state this node will never refresh again. Mirrors
    // delSession's teardown pattern: for every path in a session THIS node originated
    // (i.e. every entry in `traffic`, the locally-configured/signaled demand), force a
    // PathTear downstream for its currently-installed PSB, then clear all local state.
    // Sessions merely transiting this node (no corresponding `traffic` entry) are not
    // torn down here -- this node is not their owner.
    //
    // Deliberately does NOT consult `tedmod` here (unlike delSession/processPSB_TIMEOUT):
    // this handler runs during ModuleStopOperation's STAGE_ROUTING_PROTOCOLS, the very
    // same stage Ted's own handleStopOperation() runs in, and submodules within a stage
    // are visited in NED declaration order -- ~Ted is declared before ~RsvpTe in
    // ~MplsRouterBase/RsvpMplsRouter, so by the time this code runs Ted may have already
    // cleared its link database (ted.clear()/interfaceAddrs.clear()). The PSB already
    // caches everything needed -- OutInterface is the LIH toward ERO[0], set once at PSB
    // creation (createIngressPSB/createPSB) and never dependent on Ted afterwards -- so
    // this uses that cached value directly, exactly as processPSB_TIMEOUT does for the
    // same reason (a PSB timeout can likewise coincide with Ted no longer being queryable).
    bool sentAnyPathTear = false;
    for (auto& session : traffic) {
        for (auto& path : session.paths) {
            PathStateBlock *psb = findPSB(session.sobj, path.sender);
            if (psb && !psb->OutInterface.isUnspecified()) {
                ASSERT(psb->ERO.size() > 0);

                sendPathTearMessage(psb->ERO[0].node, psb->sessionObject, psb->Sender_Template_Object,
                        psb->OutInterface, routerId, true);
                sentAnyPathTear = true;
            }
        }
    }

    clear();

    if (sentAnyPathTear) {
        // ModuleStopOperation advances stage by stage (STAGE_ROUTING_PROTOCOLS, then
        // STAGE_TRANSPORT_LAYER, STAGE_NETWORK_LAYER, ... STAGE_LINK_LAYER), synchronously,
        // within the same call chain as this scenario command, UNLESS this stage is held
        // pending -- so without delaying, Ipv4/the interface below would stop in the very
        // same event before the PathTear(s) just send()'d above even get to traverse the
        // local dispatcher chain, and get silently dropped ("... is down, dropping").
        //
        // Use startActiveOperationExtraTime(), NOT delayActiveOperationFinish(): the two
        // look similar but differ in exactly the way that matters here.
        // delayActiveOperationFinish() keeps operationalState at STOPPING_OPERATION until
        // the timeout elapses, and OperationalMixin::handleMessage() routes STOPPING_OPERATION
        // through handleMessageWhenUp() same as OPERATING -- i.e. this module would keep
        // fully processing arriving RSVP packets (Hello/Path/Resv/...) for the whole window,
        // against the state clear() just wiped, AND via a `tedmod` that -- per the note above
        // -- may already be stopped too; a Hello from a peer who hasn't heard about the
        // shutdown yet arriving in that window previously crashed in Ted::primaryAddress()
        // (found empirically: rsvpte_graceful_shutdown.test's own scenario reproduces this).
        // startActiveOperationExtraTime(), by contrast, flips operationalState to the
        // operation's end state (NOT_OPERATING for a stop) IMMEDIATELY, so any further
        // inbound packet is safely dropped by the framework's own handleMessageWhenDown()
        // ("... is down, dropping ..."), while still deferring the doneCallback (and hence
        // the node's later shutdown stages) for the extra time -- exactly the mechanism
        // ~Ppp uses in its own handleStopOperation() for the analogous "let queued work
        // drain before the interface truly goes dark" case.
        startActiveOperationExtraTime(par("stopOperationExtraTime"));
    }
}

void RsvpTe::handleCrashOperation(LifecycleOperation *operation)
{
    clear();
}

} // namespace inet

