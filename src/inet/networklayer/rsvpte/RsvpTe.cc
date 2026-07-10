//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/rsvpte/RsvpTe.h"

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
constexpr int SENDER_TSPEC_OBJECT_BYTES = 40;
constexpr int FLOWSPEC_OBJECT_BYTES = 40;
constexpr int LABEL_OBJECT_BYTES = 8;
constexpr int STYLE_OBJECT_BYTES = 8;
constexpr int ERROR_SPEC_OBJECT_BYTES = 12;
constexpr int HELLO_OBJECT_BYTES = 12;
constexpr int ERO_RRO_OBJECT_HEADER_BYTES = 4;
constexpr int ERO_RRO_SUBOBJECT_BYTES = 8; // IPv4 prefix subobject

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

B computePathMessageLength(const EroVector& ero)
{
    return B(RSVP_COMMON_HEADER_BYTES + SESSION_OBJECT_BYTES + RSVP_HOP_OBJECT_BYTES + TIME_VALUES_OBJECT_BYTES
             + LABEL_REQUEST_OBJECT_BYTES + SENDER_TEMPLATE_OBJECT_BYTES + SENDER_TSPEC_OBJECT_BYTES)
           + computeEroLength(ero);
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

B computeResvMessageLength(const FlowDescriptorVector& flows)
{
    return B(RSVP_COMMON_HEADER_BYTES + SESSION_OBJECT_BYTES + RSVP_HOP_OBJECT_BYTES + TIME_VALUES_OBJECT_BYTES + STYLE_OBJECT_BYTES)
           + computeFlowDescriptorListLength(flows);
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
        refreshInterval = par("refreshInterval");
        stateLifetimeFactor = par("stateLifetimeFactor");
        retryInterval = par("retryInterval");
        advertiseImplicitNull = par("advertiseImplicitNull");
        WATCH(maxPsbId);
        WATCH(maxRsbId);
        WATCH(maxSrcInstance);
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

    traffic_session_t newSession;

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
        checkTags(path, "sender lspid bandwidth route permanent owner");

        int lspid = getParameterIntValue(path, "lspid");

        std::vector<traffic_path_t>::iterator pit;

        traffic_path_t newPath;

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

        const cXMLElement *route = getUniqueChildIfExists(path, "route");
        if (route)
            newPath.ERO = readTrafficRouteFromXML(route);

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

std::vector<RsvpTe::traffic_path_t>::iterator RsvpTe::findPath(traffic_session_t *session, const SenderTemplateObj& sender)
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
        for (auto& link : tedmod->ted) {
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

    // update TED and routing table

    unsigned int index = tedmod->linkIndex(routerId, peer);
    tedmod->ted[index].state = false;
    announceLinkChange(index);
    tedmod->rebuildRoutingTable();

    // send PATH_ERROR for existing paths

    for (auto& elem : PSBList) {
        if (elem.OutInterface == tedmod->ted[index].local)
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

    refreshPath(psb);
    scheduleRefreshTimer(psb, uniform(0.5, 1.5) * refreshInterval);
}

void RsvpTe::processPSB_TIMEOUT(PsbTimeoutMsg *msg)
{
    PathStateBlock *psb = findPsbById(msg->getId());
    ASSERT(psb);

    if (tedmod->isLocalAddress(psb->OutInterface)) {
        ASSERT(psb->OutInterface == tedmod->getInterfaceAddrByPeerAddress(psb->ERO[0].node));

        sendPathTearMessage(psb->ERO[0].node, psb->Session_Object,
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
        if ((elem.OI == OI) && (elem.Session_Object == session) && (elem.Flowspec_Object.req_bandwidth > sharedBW))
            sharedBW = elem.Flowspec_Object.req_bandwidth;
    }

    EV_DETAIL << "CACCheck: link=" << OI
              << " requested=" << tspec.req_bandwidth
              << " shared=" << sharedBW
              << " available (immediately)=" << tedmod->ted[k].UnResvBandwidth[7]
              << " available (preemptible)=" << tedmod->ted[k].UnResvBandwidth[session.setupPri] << endl;

    return tedmod->ted[k].UnResvBandwidth[session.setupPri] + sharedBW >= tspec.req_bandwidth;
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

    pm->setSession(psbEle->Session_Object);
    pm->setSenderTemplate(psbEle->Sender_Template_Object);
    pm->setSenderTspec(psbEle->Sender_Tspec_Object);

    RsvpHopObj hop;
    hop.Logical_Interface_Handle = OI;
    hop.Next_Hop_Address = routerId;
    pm->setHop(hop);

    pm->setERO(ERO);

    pm->setChunkLength(computePathMessageLength(ERO));
    pk->insertAtBack(pm);

    Ipv4Address nextHop = tedmod->getPeerByLocalAddress(OI);

    ASSERT(ERO.size() == 0 || ERO[0].node.equals(nextHop) || ERO[0].L);

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

            if (tedmod->isLocalAddress(elem.Previous_Hop_Address))
                continue; // IR nothing to refresh

            if (!contains(phops, elem.Previous_Hop_Address))
                phops.push_back(elem.Previous_Hop_Address);
        }

        for (auto& phop : phops)
            refreshResv(rsbEle, phop);
    }
}

void RsvpTe::refreshResv(ResvStateBlock *rsbEle, Ipv4Address PHOP)
{
    EV_INFO << "refresh reservation (RSB " << rsbEle->id << ") PHOP " << PHOP << endl;

    Packet *pk = new Packet("    Resv");
    const auto& msg = makeShared<RsvpResvMsg>();

    FlowDescriptorVector flows;

    msg->setSession(rsbEle->Session_Object);

    RsvpHopObj hop;
    hop.Logical_Interface_Handle = tedmod->peerRemoteInterface(PHOP);
    hop.Next_Hop_Address = PHOP;
    msg->setHop(hop);

    for (auto& elem : PSBList) {
        if (elem.Previous_Hop_Address != PHOP)
            continue;

        if (elem.Session_Object != rsbEle->Session_Object)
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

    msg->setChunkLength(computeResvMessageLength(flows));
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

            if (tedmod->isLocalAddress(elem.Previous_Hop_Address))
                continue; // IR: nothing further upstream to tear

            if (!contains(phops, elem.Previous_Hop_Address))
                phops.push_back(elem.Previous_Hop_Address);
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

    msg->setSession(rsbEle->Session_Object);

    RsvpHopObj hop;
    hop.Logical_Interface_Handle = tedmod->peerRemoteInterface(PHOP);
    hop.Next_Hop_Address = PHOP;
    msg->setHop(hop);

    for (auto& elem : PSBList) {
        if (elem.Previous_Hop_Address != PHOP)
            continue;

        if (elem.Session_Object != rsbEle->Session_Object)
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

    msg->setSession(rsbEle->Session_Object);

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

        if (elem.Session_Object.holdingPri != priority)
            continue;

        if (elem.Flowspec_Object.req_bandwidth == 0.0)
            continue;

        // preempt RSB

        EV_INFO << "preempting RSB " << elem.id << " (holding priority " << priority
                << ", releasing " << elem.Flowspec_Object.req_bandwidth << ")" << endl;

        for (int i = priority; i < 8; i++)
            tedmod->ted[index].UnResvBandwidth[i] += elem.Flowspec_Object.req_bandwidth;

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

    if (tedmod->ted[index].UnResvBandwidth[setupPri] < bandwidth)
        return false;

    for (int p = holdingPri; p < 8; p++) {
        tedmod->ted[index].UnResvBandwidth[p] -= bandwidth;

        if (tedmod->ted[index].UnResvBandwidth[p] < 0.0)
            preempt(OI, p, -tedmod->ted[index].UnResvBandwidth[p]);
    }

    // announce changes

    announceLinkChange(index);

    return true;
}

void RsvpTe::announceLinkChange(int tedlinkindex)
{
    TedChangeInfo d;
    d.setTedLinkIndicesArraySize(1);
    d.setTedLinkIndices(0, tedlinkindex);
    emit(tedChangedSignal, &d);
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
            if (allocateResource(rsb->OI, rsb->Session_Object, needed)) {
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
                PathStateBlock *psb = findPSB(rsb->Session_Object, (SenderTemplateObj&)rsb->FlowDescriptor[maxFlowIndex].Filter_Spec_Object);

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

    for (unsigned int i = 0; i < rsb->FlowDescriptor.size(); i++) {
        int lspid = rsb->FlowDescriptor[i].Filter_Spec_Object.Lsp_Id;

        EV_DETAIL << "processing lspid=" << lspid << endl;

        PathStateBlock *psb = findPSB(rsb->Session_Object, rsb->FlowDescriptor[i].Filter_Spec_Object);

        LabelOpVector outLabel;
        int inInterfaceId, outInterfaceId;

        bool IR = (psb->Previous_Hop_Address == routerId);
        if (!IR) {
            Ipv4Address localInf = tedmod->getInterfaceAddrByPeerAddress(psb->Previous_Hop_Address);
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

            if (!tedmod->isLocalAddress(psb->Session_Object.DestAddress)) {
                NetworkInterface *ie = rt->getInterfaceForDestAddr(psb->Session_Object.DestAddress);
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
            sendPathNotify(psb->handler, psb->Session_Object, psb->Sender_Template_Object, PATH_CREATED, 0.0);
            emit(lspEstablishedSignal, simTime() - psb->pathCreationTime);
        }

        if (rsb->inLabelVector[i] != inLabel) {
            // remember our current label
            rsb->inLabelVector[i] = inLabel;

            // bind fec
            rpct->bind(psb->Session_Object, psb->Sender_Template_Object, inLabel);
        }
    }
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

    rsbEle.Session_Object = msg->getSession();
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
        allocateResource(rsb->OI, rsb->Session_Object, -rsb->Flowspec_Object.req_bandwidth);
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
    ResvStateBlock *rsb = findRSB(psb->Session_Object, psb->Sender_Template_Object, filterIndex);
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

    psbEle.Session_Object = msg->getSession();
    psbEle.Sender_Template_Object = msg->getSenderTemplate();
    psbEle.Sender_Tspec_Object = msg->getSenderTspec();

    psbEle.Previous_Hop_Address = msg->getNHOP();

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

RsvpTe::PathStateBlock *RsvpTe::createIngressPSB(const traffic_session_t& session, const traffic_path_t& path)
{
    EroVector ERO = path.ERO;

    while (ERO.size() > 0 && ERO[0].node == routerId) {
        // remove ourselves from the beginning of the hop list
        ERO.erase(ERO.begin());
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

    psbEle.Session_Object = session.sobj;
    psbEle.Sender_Template_Object = path.sender;
    psbEle.Sender_Tspec_Object = path.tspec;

    psbEle.Previous_Hop_Address = routerId;

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

    rsbEle.Session_Object = psb->Session_Object;
    rsbEle.Next_Hop_Address = psb->Previous_Hop_Address;

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

    if (psb->Previous_Hop_Address != routerId) {
        EV_INFO << "forwarding error message to PHOP (" << psb->Previous_Hop_Address << ")" << endl;

        delete pk->removeControlInfo(); // FIXME
        pk->trim();
        sendToIP(pk, psb->Previous_Hop_Address);
    }
    else {
        EV_INFO << "error reached ingress router" << endl;

        switch (errCode) {
            case PATH_ERR_PREEMPTED:
                sendPathNotify(psb->handler, psb->Session_Object, psb->Sender_Template_Object, PATH_PREEMPTED, 0.0);
                break;

            case PATH_ERR_UNFEASIBLE:
                sendPathNotify(psb->handler, psb->Session_Object, psb->Sender_Template_Object, PATH_UNFEASIBLE, 0.0);
                break;

            case PATH_ERR_NEXTHOP_FAILED:
                sendPathNotify(psb->handler, psb->Session_Object, psb->Sender_Template_Object, PATH_FAILED, 0.0);
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

        sendPathTearMessage(psb->ERO[0].node, psb->Session_Object, psb->Sender_Template_Object,
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
            if (!tedmod->ted[index].state) {
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
        if (!(msg->isInSession(&elem.Session_Object)))
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

    delete pk;
}

void RsvpTe::recoveryEvent(Ipv4Address peer)
{
    // called when peer's operation is restored

    unsigned int index = tedmod->linkIndex(routerId, peer);
    bool rtmodified = !tedmod->ted[index].state;
    tedmod->ted[index].state = true;
    announceLinkChange(index);

    // rebuild routing table if link state changed
    if (rtmodified)
        tedmod->rebuildRoutingTable();

    // refresh all paths towards this neighbour
    for (auto& elem : PSBList) {
        if (elem.OutInterface != tedmod->ted[index].local)
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

        default:
            throw cRuntimeError("Invalid command %d in message '%s'", command, msg->getName());
    }
}

void RsvpTe::pathProblem(PathStateBlock *psb)
{
    ASSERT(psb);
    ASSERT(!psb->OutInterface.isUnspecified());

    Ipv4Address nextHop = tedmod->getPeerByLocalAddress(psb->OutInterface);

    EV_INFO << "sending PathTear to " << nextHop << endl;

    sendPathTearMessage(nextHop, psb->Session_Object, psb->Sender_Template_Object,
            tedmod->getInterfaceAddrByPeerAddress(nextHop), routerId, true);

    // schedule re-creation if path is permanent

    auto sit = findSession(psb->Session_Object);
    ASSERT(sit != traffic.end());
    traffic_session_t *s = &(*sit);

    auto pit = findPath(s, psb->Sender_Template_Object);
    ASSERT(pit != s->paths.end());
    traffic_path_t *p = &(*pit);

    if (p->permanent) {
        EV_INFO << "this path is permanent, we will try to re-create it later" << endl;

        sendPathNotify(getId(), psb->Session_Object, psb->Sender_Template_Object, PATH_RETRY, retryInterval);
    }
    else {
        EV_INFO << "removing path from traffic database" << endl;

        sit->paths.erase(pit);
    }

    // remove path

    EV_INFO << "removing PSB" << endl;

    removePSB(psb);
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

std::vector<RsvpTe::traffic_session_t>::iterator RsvpTe::findSession(const SessionObj& session)
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
    traffic_session_t *session = &(*sit);

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

                sendPathTearMessage(psb->ERO[0].node, psb->Session_Object, psb->Sender_Template_Object,
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
    sendPathErrorMessage(psb->Session_Object, psb->Sender_Template_Object, psb->Sender_Tspec_Object, psb->Previous_Hop_Address, errCode);
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
        if (elem.Session_Object != session)
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
        if ((elem.Session_Object == session) && (elem.Sender_Template_Object == sender))
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
    // into Session_Object is only for our convenience
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
}

void RsvpTe::handleStartOperation(LifecycleOperation *operation)
{
    setupHello();
}

void RsvpTe::handleStopOperation(LifecycleOperation *operation)
{
    clear();
}

void RsvpTe::handleCrashOperation(LifecycleOperation *operation)
{
    clear();
}

} // namespace inet

