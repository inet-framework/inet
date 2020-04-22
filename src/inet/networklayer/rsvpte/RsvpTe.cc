//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/XMLUtils.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/rsvpte/RsvpTe.h"
#include "inet/networklayer/rsvpte/Utils.h"
#include "inet/networklayer/ted/Ted.h"

namespace inet {

#define PSB_REFRESH_INTERVAL       5.0
#define RSB_REFRESH_INTERVAL       6.0

#define PSB_TIMEOUT_INTERVAL       16.0
#define RSB_TIMEOUT_INTERVAL       19.0

#define PATH_ERR_UNFEASIBLE        1
#define PATH_ERR_PREEMPTED         2
#define PATH_ERR_NEXTHOP_FAILED    3

Define_Module(RsvpTe);

using namespace xmlutils;

RsvpTe::RsvpTe()
{
}

RsvpTe::~RsvpTe()
{
    // TODO cancelAndDelete timers in all data structures
    for (auto& psb: PSBList) {
        cancelAndDelete(psb.timerMsg);
        cancelAndDelete(psb.timeoutMsg);
    }
    for (auto& rsb: RSBList) {
        cancelAndDelete(rsb.refreshTimerMsg);
        cancelAndDelete(rsb.commitTimerMsg);
        cancelAndDelete(rsb.timeoutMsg);
    }
    for (auto& hello: HelloList) {
        cancelAndDelete(hello.timer);
        cancelAndDelete(hello.timeout);
    }
}

void RsvpTe::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);
    // TODO: INITSTAGE
    if (stage == INITSTAGE_LOCAL) {
        tedmod = getModuleFromPar<Ted>(par("tedModule"), this);
        rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        lt = getModuleFromPar<LibTable>(par("libTableModule"), this);
        rpct = getModuleFromPar<IRsvpClassifier>(par("classifierModule"), this);

        maxPsbId = 0;
        maxRsbId = 0;
        maxSrcInstance = 0;

        retryInterval = 1.0;
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

            sit->paths.erase(pit--);
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
    for (auto & elem : list)
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
        checkTags(path, "sender lspid bandwidth max_delay route permanent owner color");

        int lspid = getParameterIntValue(path, "lspid");
        ;

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
        newPath.color = getParameterIntValue(path, "color", 0);

        newPath.tspec.req_bandwidth = getParameterDoubleValue(path, "bandwidth", 0.0);
        newPath.max_delay = getParameterDoubleValue(path, "max_delay", 0.0);

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
    for ( ; it != session->paths.end(); it++) {
        if (it->sender == sender)
            break;
    }
    return it;
}

void RsvpTe::setupHello()
{
    routerId = rt->getRouterId();

    helloInterval = par("helloInterval");
    helloTimeout = par("helloTimeout");

    cStringTokenizer tokenizer(par("peers"));
    const char *token;
    while ((token = tokenizer.nextToken()) != nullptr) {
        Ipv4Address peer = tedmod->getPeerByLocalAddress(CHK(ift->findInterfaceByName(token))->getProtocolData<Ipv4InterfaceData>()->getIPAddress());

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

    scheduleAt(simTime() + delay, h->timer);
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
        scheduleAt(simTime() + delay, msg);
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

    for (auto & elem : PSBList) {
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

    int length = 10;

    // see comment elsewhere (in Ted.cc)
    length /= 10;

    hMsg->setChunkLength(B(length));
    pk->insertAtBack(hMsg);

    sendToIP(pk, peer);

    h->ack = false;

    scheduleAt(simTime() + helloInterval, msg);
}

void RsvpTe::processPSB_TIMER(PsbTimerMsg *msg)
{
    PathStateBlock *psb = findPsbById(msg->getId());
    ASSERT(psb);

    refreshPath(psb);
    scheduleRefreshTimer(psb, PSB_REFRESH_INTERVAL);
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

        scheduleRefreshTimer(rsb, RSB_REFRESH_INTERVAL);
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

    for (auto & elem : RSBList) {
        if ((elem.Session_Object == session) && (elem.Flowspec_Object.req_bandwidth > sharedBW))
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
    pm->setColor(psbEle->color);

    int length = 85 + (ERO.size() * 5);

    pm->setChunkLength(B(length));
    pk->insertAtBack(pm);

    Ipv4Address nextHop = tedmod->getPeerByLocalAddress(OI);

    ASSERT(ERO.size() == 0 || ERO[0].node.equals(nextHop) || ERO[0].L);

    sendToIP(pk, nextHop);
}

void RsvpTe::refreshResv(ResvStateBlock *rsbEle)
{
    EV_INFO << "refresh reservation (RSB " << rsbEle->id << ")" << endl;

    Ipv4AddressVector phops;

    for (auto & elem : PSBList) {
        if (elem.OutInterface != rsbEle->OI)
            continue;

        for (auto & _i : rsbEle->FlowDescriptor) {
            if ((FilterSpecObj&)elem.Sender_Template_Object != _i.Filter_Spec_Object)
                continue;

            if (tedmod->isLocalAddress(elem.Previous_Hop_Address))
                continue; // IR nothing to refresh

            if (!find(phops, elem.Previous_Hop_Address))
                phops.push_back(elem.Previous_Hop_Address);
        }

        for (auto & phop : phops)
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

    for (auto & elem : PSBList) {
        if (elem.Previous_Hop_Address != PHOP)
            continue;

        //if (it->LIH != LIH)
        //  continue;

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

    int fd_length = 0;
    for (auto & flow : flows)
        fd_length += 28 + (flow.RRO.size() * 4);

    int length = 34 + fd_length;

    // see comment elsewhere (in Ted.cc)
    length /= 10;

    msg->setChunkLength(B(length));
    pk->insertAtBack(msg);

    sendToIP(pk, PHOP);
}

void RsvpTe::preempt(Ipv4Address OI, int priority, double bandwidth)
{
    ASSERT(tedmod->isLocalAddress(OI));

    unsigned int index = tedmod->linkIndex(OI);

    for (auto & elem : RSBList) {
        if (elem.OI != OI)
            continue;

        if (elem.Session_Object.holdingPri != priority)
            continue;

        if (elem.Flowspec_Object.req_bandwidth == 0.0)
            continue;

        // preempt RSB

        for (int i = priority; i < 8; i++)
            tedmod->ted[index].UnResvBandwidth[i] += elem.Flowspec_Object.req_bandwidth;

        bandwidth -= elem.Flowspec_Object.req_bandwidth;
        elem.Flowspec_Object.req_bandwidth = 0.0;

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
        std::string inInterface, outInterface;

        bool IR = (psb->Previous_Hop_Address == routerId);
        //bool ER = psb->OutInterface.isUnspecified();
        if (!IR) {
            Ipv4Address localInf = tedmod->getInterfaceAddrByPeerAddress(psb->Previous_Hop_Address);
            inInterface = rt->getInterfaceByAddress(localInf)->getInterfaceName();
        }
        else
            inInterface = "any";

        // outlabel and outgoing interface

        LabelOp lop;

        if (tedmod->isLocalAddress(psb->OutInterface)) {
            // regular next hop

            lop.optcode = IR ? PUSH_OPER : SWAP_OPER;
            lop.label = rsb->FlowDescriptor[i].label;
            outLabel.push_back(lop);

            outInterface = rt->getInterfaceByAddress(psb->OutInterface)->getInterfaceName();
        }
        else {
            // egress router

            lop.label = 0;
            lop.optcode = POP_OPER;
            outLabel.push_back(lop);

            outInterface = "lo0";

            if (!tedmod->isLocalAddress(psb->Session_Object.DestAddress)) {
                InterfaceEntry *ie = rt->getInterfaceForDestAddr(psb->Session_Object.DestAddress);
                if (ie)
                    outInterface = ie->getInterfaceName(); // FIXME why use name to identify an interface?
            }
        }

        EV_DETAIL << "installing label for " << lspid << " outLabel=" << outLabel
                  << " outInterface=" << outInterface << endl;

        ASSERT(rsb->inLabelVector.size() == rsb->FlowDescriptor.size());

        int inLabel = lt->installLibEntry(rsb->inLabelVector[i], inInterface,
                    outLabel, outInterface, psb->color);

        ASSERT(inLabel >= 0);

        if (IR && rsb->inLabelVector[i] == -1) {
            // path established
            sendPathNotify(psb->handler, psb->Session_Object, psb->Sender_Template_Object, PATH_CREATED, 0.0);
        }

        if (rsb->inLabelVector[i] != inLabel) {
            // remember our current label
            rsb->inLabelVector[i] = inLabel;

            // bind fec
            rpct->bind(psb->Session_Object, psb->Sender_Template_Object, inLabel);
        }

        // schedule commit of merging backups too...
        for (auto & elem : RSBList) {
            if (elem.OI == Ipv4Address(lspid))
                scheduleCommitTimer(&elem);
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

    for (auto & elem : msg->getFlowDescriptor()) {
        FlowDescriptor_t flow = elem;
        rsbEle.FlowDescriptor.push_back(flow);
        rsbEle.inLabelVector.push_back(-1);
    }

    RSBList.push_back(rsbEle);
    ResvStateBlock *rsb = &(*(RSBList.end() - 1));

    EV_INFO << "created new RSB " << rsb->id << endl;

    return rsb;
}

void RsvpTe::updateRSB(ResvStateBlock *rsb, const RsvpResvMsg *msg)
{
    ASSERT(rsb);

    for (auto & elem : msg->getFlowDescriptor()) {
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

    if (inLabel != -1)
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

    cancelEvent(rsb->refreshTimerMsg);
    cancelEvent(rsb->commitTimerMsg);
    cancelEvent(rsb->timeoutMsg);

    delete rsb->refreshTimerMsg;
    delete rsb->commitTimerMsg;
    delete rsb->timeoutMsg;

    if (rsb->Flowspec_Object.req_bandwidth > 0) {
        // deallocate resources
        allocateResource(rsb->OI, rsb->Session_Object, -rsb->Flowspec_Object.req_bandwidth);
    }

    for (auto it = RSBList.begin(); it != RSBList.end(); it++) {
        if (it->id == rsb->id) {
            RSBList.erase(it);
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

    // remove reservation state if exists **************************************

    unsigned int filterIndex;
    ResvStateBlock *rsb = findRSB(psb->Session_Object, psb->Sender_Template_Object, filterIndex);
    if (rsb) {
        EV_INFO << "reservation state present, will be removed too" << endl;

        removeRsbFilter(rsb, filterIndex);
    }

    // proceed with actual removal *********************************************

    cancelAndDelete(psb->timerMsg);
    cancelAndDelete(psb->timeoutMsg);

    for (auto it = PSBList.begin(); it != PSBList.end(); it++) {
        if (it->id == psb->id) {
            PSBList.erase(it);
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
            InterfaceEntry *ie = rt->getInterfaceForDestAddr(ERO[0].node);

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
        if (!h)
            throw cRuntimeError("Peer %s on interface %s is not an RSVP peer", peer.str().c_str(), OI.str().c_str());

        // ok, only if next hop is up and running

        return h->ok;
    }
    else {
        // hop-by-hop routing

        if (!tedmod->isLocalAddress(destAddr)) {
            InterfaceEntry *ie = rt->getInterfaceForDestAddr(destAddr);

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

                ASSERT(h->ok);    // rt->getInterfaceForDestAddr() wouldn't choose this entry

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
    //psbEle.LIH = msg->getLIH();

    psbEle.OutInterface = OI;
    psbEle.ERO = ERO;

    psbEle.color = msg->getColor();
    psbEle.handler = -1;

    PSBList.push_back(psbEle);
    PathStateBlock *cPSB = &(*(PSBList.end() - 1));

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
    psbEle.color = path.color;

    psbEle.handler = path.owner;

    PSBList.push_back(psbEle);
    PathStateBlock *cPSB = &(*(PSBList.end() - 1));

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

        default:
            throw cRuntimeError("Invalid RSVP kind of message '%s': %d", pk->getName(), kind);
    }
}

void RsvpTe::processHelloMsg(Packet *pk)
{
    EV_INFO << "Received RSVP_HELLO" << endl;
    //print(msg);
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
    ASSERT(h);

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
            scheduleAt(simTime(), h->timer);
    }

    if (request) {
        // immediately respond to a request with an ack
        h->ack = true;
        h->request = false;

        cancelEvent(h->timer);
        scheduleAt(simTime(), h->timer);
    }
    else {
        // next message will be regular

        h->ack = false;
        h->request = false;

        ASSERT(h->timer->isScheduled());
    }

    cancelEvent(h->timeout);
    scheduleAt(simTime() + helloTimeout, h->timeout);
}

void RsvpTe::processPathErrMsg(Packet *pk)
{
    EV_INFO << "Received PATH_ERROR" << endl;
    //print(msg);

    const auto& msg = pk->peekAtFront<RsvpPathError>();
    //int lspid = msg->getLspId();
    int errCode = msg->getErrorCode();

    PathStateBlock *psb = findPSB(msg->getSession(), msg->getSenderTemplate());
    if (!psb) {
        EV_INFO << "matching PSB not found, ignoring error message" << endl;
        delete pk;
        return;
    }

    if (psb->Previous_Hop_Address != routerId) {
        EV_INFO << "forwarding error message to PHOP (" << psb->Previous_Hop_Address << ")" << endl;

        delete pk->removeControlInfo();         //FIXME
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
                throw cRuntimeError("Invalid errorcode %d in message '%s'", errCode, msg->getName());
        }

        delete pk;
    }
}

void RsvpTe::processPathTearMsg(Packet *pk)
{
    EV_INFO << "Received PATH_TEAR" << endl;
    //print(msg);

    const auto& msg = pk->peekAtFront<RsvpPathTear>();
    int lspid = msg->getLspId();

    PathStateBlock *psb = findPSB(msg->getSession(), msg->getSenderTemplate());
    if (!psb) {
        EV_DETAIL << "received PATH_TEAR for nonexisting lspid=" << lspid << endl;
        delete pk;
        return;
    }

    // ignore message if backup exists and force flag is not set

    bool modified = false;

    for (auto it = PSBList.begin(); it != PSBList.end(); it++) {
        if (it->OutInterface.getInt() != (uint32)lspid)
            continue;

        // merging backup exists

        if (!msg->getForce()) {
            EV_DETAIL << "merging backup tunnel exists and force flag is not set, ignoring teardown" << endl;
            delete pk;
            return;
        }

        EV_DETAIL << "merging backup must be removed too" << endl;

        removePSB(&(*it));
        --it;

        modified = true;
    }

    if (modified)
        psb = findPSB(msg->getSession(), msg->getSenderTemplate());

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

void RsvpTe::processPathMsg(Packet *pk)
{
    EV_INFO << "Received PATH_MESSAGE" << endl;
    auto msg = dynamicPtrCast<RsvpPathMsg>(pk->peekAtFront<RsvpPathMsg>()->dupShared());
    print(msg.get());

    // process ERO *************************************************************

    EroVector ERO = msg->getERO();

    while (ERO.size() > 0 && ERO[0].node == routerId) {
        ERO.erase(ERO.begin());
    }

    msg->setERO(ERO);

    // create PSB if doesn't exist yet *****************************************

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

    // schedule timer&timeout **************************************************

    scheduleTimeout(psb);

    // create RSB if we're egress and doesn't exist yet ************************

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

    // find matching PSB for every flow ****************************************

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

    // find matching RSB *******************************************************

    ResvStateBlock *rsb = nullptr;
    for (auto & elem : RSBList) {
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
    for (auto & elem : PSBList) {
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
    for ( ; it != traffic.end(); it++) {
        if (it->sobj == session)
            break;
    }
    return it;
}

void RsvpTe::addSession(const cXMLElement& node)
{
    Enter_Method_Silent();

    readTrafficSessionFromXML(&node);
}

void RsvpTe::delSession(const cXMLElement& node)
{
    Enter_Method_Silent();

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

    for (auto it = session->paths.begin(); it != session->paths.end(); it++) {
        bool remove;

        if (paths) {
            remove = false;

            for (auto & elem : pathList) {
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

            session->paths.erase(it--);
        }
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
        ASSERT(false);
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
    B length = B(44);
    msg->setChunkLength(length);
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

    int length = 52;

    // see comment elsewhere (in Ted.cc)
    length /= 10;

    msg->setChunkLength(B(length));
    pk->insertAtBack(msg);

    sendToIP(pk, nextHop);
}

void RsvpTe::sendToIP(Packet *msg, Ipv4Address destAddr)
{
    msg->addPar("color") = RSVP_TRAFFIC;
    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::rsvpTe);
    msg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::rsvpTe);
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    msg->addTagIfAbsent<L3AddressReq>()->setDestAddress(destAddr);
    send(msg, "ipOut");
}

void RsvpTe::scheduleTimeout(PathStateBlock *psbEle)
{
    ASSERT(psbEle);

    if (psbEle->timeoutMsg->isScheduled())
        cancelEvent(psbEle->timeoutMsg);

    scheduleAt(simTime() + PSB_TIMEOUT_INTERVAL, psbEle->timeoutMsg);
}

void RsvpTe::scheduleRefreshTimer(PathStateBlock *psbEle, simtime_t delay)
{
    ASSERT(psbEle);

    if (psbEle->OutInterface.isUnspecified())
        return;

    if (!tedmod->isLocalAddress(psbEle->OutInterface))
        return;

    if (psbEle->timerMsg->isScheduled())
        cancelEvent(psbEle->timerMsg);

    EV_DETAIL << "scheduling PSB " << psbEle->id << " refresh " << (simTime() + delay) << endl;

    scheduleAt(simTime() + delay, psbEle->timerMsg);
}

void RsvpTe::scheduleTimeout(ResvStateBlock *rsbEle)
{
    ASSERT(rsbEle);

    if (rsbEle->timeoutMsg->isScheduled())
        cancelEvent(rsbEle->timeoutMsg);

    scheduleAt(simTime() + RSB_TIMEOUT_INTERVAL, rsbEle->timeoutMsg);
}

void RsvpTe::scheduleRefreshTimer(ResvStateBlock *rsbEle, simtime_t delay)
{
    ASSERT(rsbEle);

    if (rsbEle->refreshTimerMsg->isScheduled())
        cancelEvent(rsbEle->refreshTimerMsg);

    scheduleAt(simTime() + delay, rsbEle->refreshTimerMsg);
}

void RsvpTe::scheduleCommitTimer(ResvStateBlock *rsbEle)
{
    ASSERT(rsbEle);

    if (rsbEle->commitTimerMsg->isScheduled())
        cancelEvent(rsbEle->commitTimerMsg);

    scheduleAt(simTime(), rsbEle->commitTimerMsg);
}

RsvpTe::ResvStateBlock *RsvpTe::findRSB(const SessionObj& session, const SenderTemplateObj& sender, unsigned int& index)
{
    for (auto & elem : RSBList) {
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
    for (auto & elem : PSBList) {
        if ((elem.Session_Object == session) && (elem.Sender_Template_Object == sender))
            return &(elem);
    }
    return nullptr;
}

RsvpTe::PathStateBlock *RsvpTe::findPsbById(int id)
{
    for (auto & elem : PSBList) {
        if (elem.id == id)
            return &elem;
    }
    ASSERT(false);
    return nullptr;    // prevent warning
}

RsvpTe::ResvStateBlock *RsvpTe::findRsbById(int id)
{
    for (auto & elem : RSBList) {
        if (elem.id == id)
            return &elem;
    }
    ASSERT(false);
    return nullptr;    // prevent warning
}

RsvpTe::HelloState *RsvpTe::findHello(Ipv4Address peer)
{
    for (auto & elem : HelloList) {
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
    for (auto & elem : r->getFlowDescriptor()) {
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

