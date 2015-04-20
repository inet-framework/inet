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

#include "inet/networklayer/rsvp_te/RSVP.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/rsvp_te/Utils.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ted/TED.h"

namespace inet {

#define PSB_REFRESH_INTERVAL       5.0
#define RSB_REFRESH_INTERVAL       6.0

#define PSB_TIMEOUT_INTERVAL       16.0
#define RSB_TIMEOUT_INTERVAL       19.0

#define PATH_ERR_UNFEASIBLE        1
#define PATH_ERR_PREEMPTED         2
#define PATH_ERR_NEXTHOP_FAILED    3

Define_Module(RSVP);

using namespace xmlutils;

RSVP::RSVP()
{
}

RSVP::~RSVP()
{
    // TODO cancelAndDelete timers in all data structures
}

void RSVP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        tedmod = getModuleFromPar<TED>(par("tedModule"), this);
        rt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        routerId = rt->getRouterId();
        lt = getModuleFromPar<LIBTable>(par("libTableModule"), this);
        rpct = getModuleFromPar<IRSVPClassifier>(par("classifierModule"), this);

        maxPsbId = 0;
        maxRsbId = 0;
        maxSrcInstance = 0;

        retryInterval = 1.0;

        // setup hello
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (isOperational)
            setupHello();

        // process traffic configuration
        readTrafficFromXML(par("traffic").xmlValue());

        IPSocket ipSocket(gate("ipOut"));
        ipSocket.registerProtocol(IP_PROT_RSVP);
    }
}

int RSVP::getInLabel(const SessionObj_t& session, const SenderTemplateObj_t& sender)
{
    unsigned int index;
    ResvStateBlock_t *rsb = findRSB(session, sender, index);
    if (!rsb)
        return -1;

    return rsb->inLabelVector[index];
}

void RSVP::createPath(const SessionObj_t& session, const SenderTemplateObj_t& sender)
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

    PathStateBlock_t *psb = createIngressPSB(*sit, *pit);
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

void RSVP::readTrafficFromXML(const cXMLElement *traffic)
{
    ASSERT(traffic);
    ASSERT(!strcmp(traffic->getTagName(), "sessions"));
    checkTags(traffic, "session");
    cXMLElementList list = traffic->getChildrenByTagName("session");
    for (auto & elem : list)
        readTrafficSessionFromXML(elem);
}

EroVector RSVP::readTrafficRouteFromXML(const cXMLElement *route)
{
    checkTags(route, "node lnode");

    EroVector ERO;

    for (cXMLElement *hop = route->getFirstChild(); hop; hop = hop->getNextSibling()) {
        EroObj_t h;
        if (!strcmp(hop->getTagName(), "node")) {
            h.L = false;
            h.node = L3AddressResolver().resolve(hop->getNodeValue()).toIPv4();
        }
        else if (!strcmp(hop->getTagName(), "lnode")) {
            h.L = true;
            h.node = L3AddressResolver().resolve(hop->getNodeValue()).toIPv4();
        }
        else {
            ASSERT(false);
        }
        ERO.push_back(h);
    }

    return ERO;
}

void RSVP::readTrafficSessionFromXML(const cXMLElement *session)
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

std::vector<RSVP::traffic_path_t>::iterator RSVP::findPath(traffic_session_t *session, const SenderTemplateObj_t& sender)
{
    auto it = session->paths.begin();
    for ( ; it != session->paths.end(); it++) {
        if (it->sender == sender)
            break;
    }
    return it;
}

void RSVP::setupHello()
{
    helloInterval = par("helloInterval").doubleValue();
    helloTimeout = par("helloTimeout").doubleValue();

    cStringTokenizer tokenizer(par("peers"));
    const char *token;
    while ((token = tokenizer.nextToken()) != nullptr) {
        ASSERT(ift->getInterfaceByName(token));

        IPv4Address peer = tedmod->getPeerByLocalAddress(ift->getInterfaceByName(token)->ipv4Data()->getIPAddress());

        HelloState_t h;

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

void RSVP::startHello(IPv4Address peer, simtime_t delay)
{
    EV_INFO << "scheduling hello start in " << delay << " seconds" << endl;

    HelloState_t *h = findHello(peer);
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

void RSVP::removeHello(HelloState_t *h)
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

void RSVP::sendPathNotify(int handler, const SessionObj_t& session, const SenderTemplateObj_t& sender, int status, simtime_t delay)
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

void RSVP::processHELLO_TIMEOUT(HelloTimeoutMsg *msg)
{
    IPv4Address peer = msg->getPeer();

    EV_INFO << "hello timeout, considering " << peer << " failed" << endl;

    // update hello state (set to failed and turn hello off)

    HelloState_t *hello = findHello(peer);
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

void RSVP::processHELLO_TIMER(HelloTimerMsg *msg)
{
    IPv4Address peer = msg->getPeer();

    HelloState_t *h = findHello(peer);
    ASSERT(h);

    RSVPHelloMsg *hMsg = new RSVPHelloMsg("hello message");

    hMsg->setSrcInstance(h->srcInstance);
    hMsg->setDstInstance(h->dstInstance);

    hMsg->setRequest(h->request);
    hMsg->setAck(h->ack);

    int length = 10;

    // see comment elsewhere (in TED.cc)
    length /= 10;

    hMsg->setByteLength(length);

    sendToIP(hMsg, peer);

    h->ack = false;

    scheduleAt(simTime() + helloInterval, msg);
}

void RSVP::processPSB_TIMER(PsbTimerMsg *msg)
{
    PathStateBlock_t *psb = findPsbById(msg->getId());
    ASSERT(psb);

    refreshPath(psb);
    scheduleRefreshTimer(psb, PSB_REFRESH_INTERVAL);
}

void RSVP::processPSB_TIMEOUT(PsbTimeoutMsg *msg)
{
    PathStateBlock_t *psb = findPsbById(msg->getId());
    ASSERT(psb);

    if (tedmod->isLocalAddress(psb->OutInterface)) {
        ASSERT(psb->OutInterface == tedmod->getInterfaceAddrByPeerAddress(psb->ERO[0].node));

        sendPathTearMessage(psb->ERO[0].node, psb->Session_Object,
                psb->Sender_Template_Object, psb->OutInterface, routerId, false);
    }

    removePSB(psb);
}

void RSVP::processRSB_REFRESH_TIMER(RsbRefreshTimerMsg *msg)
{
    ResvStateBlock_t *rsb = findRsbById(msg->getId());
    if (rsb->commitTimerMsg->isScheduled()) {
        // reschedule after commit
        scheduleRefreshTimer(rsb, 0.0);
    }
    else {
        refreshResv(rsb);

        scheduleRefreshTimer(rsb, RSB_REFRESH_INTERVAL);
    }
}

void RSVP::processRSB_COMMIT_TIMER(RsbCommitTimerMsg *msg)
{
    ResvStateBlock_t *rsb = findRsbById(msg->getId());
    commitResv(rsb);
}

void RSVP::processRSB_TIMEOUT(RsbTimeoutMsg *msg)
{
    EV_INFO << "RSB TIMEOUT RSB " << msg->getId() << endl;

    ResvStateBlock_t *rsb = findRsbById(msg->getId());

    ASSERT(rsb);
    ASSERT(tedmod->isLocalAddress(rsb->OI));

    for (unsigned int i = 0; i < rsb->FlowDescriptor.size(); i++) {
        removeRsbFilter(rsb, 0);
    }
    removeRSB(rsb);
}

bool RSVP::doCACCheck(const SessionObj_t& session, const SenderTspecObj_t& tspec, IPv4Address OI)
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

void RSVP::refreshPath(PathStateBlock_t *psbEle)
{
    EV_INFO << "refresh path (PSB " << psbEle->id << ")" << endl;

    IPv4Address& OI = psbEle->OutInterface;
    EroVector& ERO = psbEle->ERO;

    ASSERT(!OI.isUnspecified());
    ASSERT(tedmod->isLocalAddress(OI));

    RSVPPathMsg *pm = new RSVPPathMsg("Path");

    pm->setSession(psbEle->Session_Object);
    pm->setSenderTemplate(psbEle->Sender_Template_Object);
    pm->setSenderTspec(psbEle->Sender_Tspec_Object);

    RsvpHopObj_t hop;
    hop.Logical_Interface_Handle = OI;
    hop.Next_Hop_Address = routerId;
    pm->setHop(hop);

    pm->setERO(ERO);
    pm->setColor(psbEle->color);

    int length = 85 + (ERO.size() * 5);

    pm->setByteLength(length);

    IPv4Address nextHop = tedmod->getPeerByLocalAddress(OI);

    ASSERT(ERO.size() == 0 || ERO[0].node.equals(nextHop) || ERO[0].L);

    sendToIP(pm, nextHop);
}

void RSVP::refreshResv(ResvStateBlock_t *rsbEle)
{
    EV_INFO << "refresh reservation (RSB " << rsbEle->id << ")" << endl;

    IPAddressVector phops;

    for (auto & elem : PSBList) {
        if (elem.OutInterface != rsbEle->OI)
            continue;

        for (auto & _i : rsbEle->FlowDescriptor) {
            if ((FilterSpecObj_t&)elem.Sender_Template_Object != _i.Filter_Spec_Object)
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

void RSVP::refreshResv(ResvStateBlock_t *rsbEle, IPv4Address PHOP)
{
    EV_INFO << "refresh reservation (RSB " << rsbEle->id << ") PHOP " << PHOP << endl;

    RSVPResvMsg *msg = new RSVPResvMsg("    Resv");

    FlowDescriptorVector flows;

    msg->setSession(rsbEle->Session_Object);

    RsvpHopObj_t hop;
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
            if ((FilterSpecObj_t&)elem.Sender_Template_Object != rsbEle->FlowDescriptor[c].Filter_Spec_Object)
                continue;

            ASSERT(rsbEle->inLabelVector.size() == rsbEle->FlowDescriptor.size());

            FlowDescriptor_t flow;
            flow.Filter_Spec_Object = (FilterSpecObj_t&)elem.Sender_Template_Object;
            flow.Flowspec_Object = (FlowSpecObj_t&)elem.Sender_Tspec_Object;
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

    // see comment elsewhere (in TED.cc)
    length /= 10;

    msg->setByteLength(length);

    sendToIP(msg, PHOP);
}

void RSVP::preempt(IPv4Address OI, int priority, double bandwidth)
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

bool RSVP::allocateResource(IPv4Address OI, const SessionObj_t& session, double bandwidth)
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

void RSVP::announceLinkChange(int tedlinkindex)
{
    TEDChangeInfo d;
    d.setTedLinkIndicesArraySize(1);
    d.setTedLinkIndices(0, tedlinkindex);
    emit(NF_TED_CHANGED, &d);
}

void RSVP::commitResv(ResvStateBlock_t *rsb)
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

        FlowSpecObj_t req;
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
                PathStateBlock_t *psb = findPSB(rsb->Session_Object, (SenderTemplateObj_t&)rsb->FlowDescriptor[maxFlowIndex].Filter_Spec_Object);

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

        PathStateBlock_t *psb = findPSB(rsb->Session_Object, rsb->FlowDescriptor[i].Filter_Spec_Object);

        LabelOpVector outLabel;
        std::string inInterface, outInterface;

        bool IR = (psb->Previous_Hop_Address == routerId);
        //bool ER = psb->OutInterface.isUnspecified();
        if (!IR) {
            IPv4Address localInf = tedmod->getInterfaceAddrByPeerAddress(psb->Previous_Hop_Address);
            inInterface = rt->getInterfaceByAddress(localInf)->getName();
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

            outInterface = rt->getInterfaceByAddress(psb->OutInterface)->getName();
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
                    outInterface = ie->getName(); // FIXME why use name to identify an interface?
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
            if (elem.OI == IPv4Address(lspid))
                scheduleCommitTimer(&elem);
        }
    }
}

RSVP::ResvStateBlock_t *RSVP::createRSB(RSVPResvMsg *msg)
{
    ResvStateBlock_t rsbEle;

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
    ResvStateBlock_t *rsb = &(*(RSBList.end() - 1));

    EV_INFO << "created new RSB " << rsb->id << endl;

    return rsb;
}

void RSVP::updateRSB(ResvStateBlock_t *rsb, RSVPResvMsg *msg)
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

void RSVP::removeRsbFilter(ResvStateBlock_t *rsb, unsigned int index)
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

void RSVP::removeRSB(ResvStateBlock_t *rsb)
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

void RSVP::removePSB(PathStateBlock_t *psb)
{
    ASSERT(psb);

    int lspid = psb->Sender_Template_Object.Lsp_Id;

    EV_INFO << "removing PSB " << psb->id << " (lspid " << lspid << ")" << endl;

    // remove reservation state if exists **************************************

    unsigned int filterIndex;
    ResvStateBlock_t *rsb = findRSB(psb->Session_Object, psb->Sender_Template_Object, filterIndex);
    if (rsb) {
        EV_INFO << "reservation state present, will be removed too" << endl;

        removeRsbFilter(rsb, filterIndex);
    }

    // proceed with actual removal *********************************************

    cancelEvent(psb->timerMsg);
    cancelEvent(psb->timeoutMsg);

    delete psb->timerMsg;
    delete psb->timeoutMsg;

    for (auto it = PSBList.begin(); it != PSBList.end(); it++) {
        if (it->id == psb->id) {
            PSBList.erase(it);
            return;
        }
    }
    ASSERT(false);
}

bool RSVP::evalNextHopInterface(IPv4Address destAddr, const EroVector& ERO, IPv4Address& OI)
{
    if (ERO.size() > 0) {
        // explicit routing

        if (ERO[0].L) {
            InterfaceEntry *ie = rt->getInterfaceForDestAddr(ERO[0].node);

            if (!ie) {
                EV_INFO << "next (loose) hop address " << ERO[0].node << " is currently unroutable" << endl;
                return false;
            }

            OI = ie->ipv4Data()->getIPAddress();
        }
        else {
            OI = tedmod->getInterfaceAddrByPeerAddress(ERO[0].node);
        }

        IPv4Address peer = tedmod->getPeerByLocalAddress(OI);
        HelloState_t *h = findHello(peer);
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

            OI = ie->ipv4Data()->getIPAddress();

            HelloState_t *h = findHello(tedmod->getPeerByLocalAddress(OI));
            if (!h) {
                // outgoing interface is not LSR, we are egress router

                OI = IPv4Address();

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

RSVP::PathStateBlock_t *RSVP::createPSB(RSVPPathMsg *msg)
{
    const EroVector& ERO = msg->getERO();
    IPv4Address destAddr = msg->getDestAddress();

    //

    IPv4Address OI;

    if (!evalNextHopInterface(destAddr, ERO, OI))
        return nullptr;

    if (tedmod->isLocalAddress(OI) && !doCACCheck(msg->getSession(), msg->getSenderTspec(), OI))
        return nullptr; // not enough resources

    PathStateBlock_t psbEle;

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
    PathStateBlock_t *cPSB = &(*(PSBList.end() - 1));

    EV_INFO << "created new PSB " << cPSB->id << endl;

    return cPSB;
}

RSVP::PathStateBlock_t *RSVP::createIngressPSB(const traffic_session_t& session, const traffic_path_t& path)
{
    EroVector ERO = path.ERO;

    while (ERO.size() > 0 && ERO[0].node == routerId) {
        // remove ourselves from the beginning of the hop list
        ERO.erase(ERO.begin());
    }

    IPv4Address OI;

    if (!evalNextHopInterface(session.sobj.DestAddress, ERO, OI))
        return nullptr;

    if (!doCACCheck(session.sobj, path.tspec, OI))
        return nullptr;

    EV_INFO << "CACCheck passed, creating PSB" << endl;

    PathStateBlock_t psbEle;
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
    PathStateBlock_t *cPSB = &(*(PSBList.end() - 1));

    return cPSB;
}

RSVP::ResvStateBlock_t *RSVP::createEgressRSB(PathStateBlock_t *psb)
{
    ResvStateBlock_t rsbEle;

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
    flow.Flowspec_Object = (FlowSpecObj_t&)psb->Sender_Tspec_Object;
    flow.Filter_Spec_Object = (FilterSpecObj_t&)psb->Sender_Template_Object;
    flow.label = -1;

    rsbEle.FlowDescriptor.push_back(flow);
    rsbEle.inLabelVector.push_back(-1);

    RSBList.push_back(rsbEle);
    ResvStateBlock_t *rsb = &(*(RSBList.end() - 1));

    EV_INFO << "created new (egress) RSB " << rsb->id << endl;

    return rsb;
}

void RSVP::handleMessage(cMessage *msg)
{
    SignallingMsg *sMsg = dynamic_cast<SignallingMsg *>(msg);
    RSVPMessage *rMsg = dynamic_cast<RSVPMessage *>(msg);

    if (sMsg) {
        processSignallingMessage(sMsg);
        return;
    }
    else if (rMsg) {
        processRSVPMessage(rMsg);
        return;
    }
    else
        ASSERT(false);
}

void RSVP::processRSVPMessage(RSVPMessage *msg)
{
    int kind = msg->getRsvpKind();
    switch (kind) {
        case PATH_MESSAGE:
            processPathMsg(check_and_cast<RSVPPathMsg *>(msg));
            break;

        case RESV_MESSAGE:
            processResvMsg(check_and_cast<RSVPResvMsg *>(msg));
            break;

        case PTEAR_MESSAGE:
            processPathTearMsg(check_and_cast<RSVPPathTear *>(msg));
            break;

        case HELLO_MESSAGE:
            processHelloMsg(check_and_cast<RSVPHelloMsg *>(msg));
            break;

        case PERROR_MESSAGE:
            processPathErrMsg(check_and_cast<RSVPPathError *>(msg));
            break;

        default:
            throw cRuntimeError("Invalid RSVP kind of message '%s': %d", msg->getName(), kind);
    }
}

void RSVP::processHelloMsg(RSVPHelloMsg *msg)
{
    EV_INFO << "Received RSVP_HELLO" << endl;
    //print(msg);

    IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo *>(msg->getControlInfo());
    IPv4Address sender = controlInfo->getSrcAddr();
    IPv4Address peer = tedmod->primaryAddress(sender);

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

    delete msg;

    HelloState_t *h = findHello(peer);
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

void RSVP::processPathErrMsg(RSVPPathError *msg)
{
    EV_INFO << "Received PATH_ERROR" << endl;
    //print(msg);

    //int lspid = msg->getLspId();
    int errCode = msg->getErrorCode();

    PathStateBlock_t *psb = findPSB(msg->getSession(), msg->getSenderTemplate());
    if (!psb) {
        EV_INFO << "matching PSB not found, ignoring error message" << endl;
        delete msg;
        return;
    }

    if (psb->Previous_Hop_Address != routerId) {
        EV_INFO << "forwarding error message to PHOP (" << psb->Previous_Hop_Address << ")" << endl;

        delete msg->removeControlInfo();
        sendToIP(msg, psb->Previous_Hop_Address);
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

        delete msg;
    }
}

void RSVP::processPathTearMsg(RSVPPathTear *msg)
{
    EV_INFO << "Received PATH_TEAR" << endl;
    //print(msg);

    int lspid = msg->getLspId();

    PathStateBlock_t *psb = findPSB(msg->getSession(), msg->getSenderTemplate());
    if (!psb) {
        EV_DETAIL << "received PATH_TEAR for nonexisting lspid=" << lspid << endl;
        delete msg;
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
            delete msg;
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

    delete msg;
}

void RSVP::processPathMsg(RSVPPathMsg *msg)
{
    EV_INFO << "Received PATH_MESSAGE" << endl;
    print(msg);

    // process ERO *************************************************************

    EroVector ERO = msg->getERO();

    while (ERO.size() > 0 && ERO[0].node == routerId) {
        ERO.erase(ERO.begin());
    }

    msg->setERO(ERO);

    // create PSB if doesn't exist yet *****************************************

    PathStateBlock_t *psb = findPSB(msg->getSession(), msg->getSenderTemplate());

    if (!psb) {
        psb = createPSB(msg);
        if (!psb) {
            sendPathErrorMessage(msg->getSession(), msg->getSenderTemplate(),
                    msg->getSenderTspec(), msg->getNHOP(), PATH_ERR_UNFEASIBLE);
            delete msg;
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
    ResvStateBlock_t *rsb = findRSB(msg->getSession(), msg->getSenderTemplate(), index);

    if (!rsb && psb->OutInterface.isUnspecified()) {
        ASSERT(ERO.size() == 0);
        rsb = createEgressRSB(psb);
        ASSERT(rsb);
        scheduleCommitTimer(rsb);
    }

    if (rsb)
        scheduleRefreshTimer(rsb, 0.0);

    delete msg;
}

void RSVP::processResvMsg(RSVPResvMsg *msg)
{
    EV_INFO << "Received RESV_MESSAGE" << endl;
    print(msg);

    IPv4Address OI = msg->getLIH();

    // find matching PSB for every flow ****************************************

    for (unsigned int m = 0; m < msg->getFlowDescriptor().size(); m++) {
        PathStateBlock_t *psb = findPSB(msg->getSession(), (SenderTemplateObj_t&)msg->getFlowDescriptor()[m].Filter_Spec_Object);
        if (!psb) {
            EV_DETAIL << "matching PSB not found for lspid=" << msg->getFlowDescriptor()[m].Filter_Spec_Object.Lsp_Id << endl;

            // remove descriptor from message
            msg->getFlowDescriptor().erase(msg->getFlowDescriptor().begin() + m);
            --m;
        }
    }

    if (msg->getFlowDescriptor().size() == 0) {
        EV_INFO << "no matching PSB found" << endl;
        delete msg;
        return;
    }

    // find matching RSB *******************************************************

    ResvStateBlock_t *rsb = nullptr;
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
        updateRSB(rsb, msg);

    scheduleTimeout(rsb);

    delete msg;
}

void RSVP::recoveryEvent(IPv4Address peer)
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

void RSVP::processSignallingMessage(SignallingMsg *msg)
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

void RSVP::pathProblem(PathStateBlock_t *psb)
{
    ASSERT(psb);
    ASSERT(!psb->OutInterface.isUnspecified());

    IPv4Address nextHop = tedmod->getPeerByLocalAddress(psb->OutInterface);

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

void RSVP::processPATH_NOTIFY(PathNotifyMsg *msg)
{
    PathStateBlock_t *psb;

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

std::vector<RSVP::traffic_session_t>::iterator RSVP::findSession(const SessionObj_t& session)
{
    auto it = traffic.begin();
    for ( ; it != traffic.end(); it++) {
        if (it->sobj == session)
            break;
    }
    return it;
}

void RSVP::addSession(const cXMLElement& node)
{
    Enter_Method_Silent();

    readTrafficSessionFromXML(&node);
}

void RSVP::delSession(const cXMLElement& node)
{
    Enter_Method_Silent();

    checkTags(&node, "tunnel_id extended_tunnel_id endpoint paths");

    SessionObj_t sobj;

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
            PathStateBlock_t *psb = findPSB(session->sobj, it->sender);
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

void RSVP::processCommand(const cXMLElement& node)
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

void RSVP::sendPathTearMessage(IPv4Address peerIP, const SessionObj_t& session, const SenderTemplateObj_t& sender, IPv4Address LIH, IPv4Address NHOP, bool force)
{
    RSVPPathTear *msg = new RSVPPathTear("PathTear");
    msg->setSenderTemplate(sender);
    msg->setSession(session);
    RsvpHopObj_t hop;
    hop.Logical_Interface_Handle = LIH;
    hop.Next_Hop_Address = NHOP;
    msg->setHop(hop);
    msg->setForce(force);

    int length = 44;

    msg->setByteLength(length);

    sendToIP(msg, peerIP);
}

void RSVP::sendPathErrorMessage(PathStateBlock_t *psb, int errCode)
{
    sendPathErrorMessage(psb->Session_Object, psb->Sender_Template_Object, psb->Sender_Tspec_Object, psb->Previous_Hop_Address, errCode);
}

void RSVP::sendPathErrorMessage(SessionObj_t session, SenderTemplateObj_t sender, SenderTspecObj_t tspec, IPv4Address nextHop, int errCode)
{
    RSVPPathError *msg = new RSVPPathError("PathErr");
    msg->setErrorCode(errCode);
    msg->setErrorNode(routerId);
    msg->setSession(session);
    msg->setSenderTemplate(sender);
    msg->setSenderTspec(tspec);

    int length = 52;

    // see comment elsewhere (in TED.cc)
    length /= 10;

    msg->setByteLength(length);

    sendToIP(msg, nextHop);
}

void RSVP::sendToIP(cMessage *msg, IPv4Address destAddr)
{
    IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
    controlInfo->setDestAddr(destAddr);
    controlInfo->setProtocol(IP_PROT_RSVP);
    msg->setControlInfo(controlInfo);

    msg->addPar("color") = RSVP_TRAFFIC;

    send(msg, "ipOut");
}

void RSVP::scheduleTimeout(PathStateBlock_t *psbEle)
{
    ASSERT(psbEle);

    if (psbEle->timeoutMsg->isScheduled())
        cancelEvent(psbEle->timeoutMsg);

    scheduleAt(simTime() + PSB_TIMEOUT_INTERVAL, psbEle->timeoutMsg);
}

void RSVP::scheduleRefreshTimer(PathStateBlock_t *psbEle, simtime_t delay)
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

void RSVP::scheduleTimeout(ResvStateBlock_t *rsbEle)
{
    ASSERT(rsbEle);

    if (rsbEle->timeoutMsg->isScheduled())
        cancelEvent(rsbEle->timeoutMsg);

    scheduleAt(simTime() + RSB_TIMEOUT_INTERVAL, rsbEle->timeoutMsg);
}

void RSVP::scheduleRefreshTimer(ResvStateBlock_t *rsbEle, simtime_t delay)
{
    ASSERT(rsbEle);

    if (rsbEle->refreshTimerMsg->isScheduled())
        cancelEvent(rsbEle->refreshTimerMsg);

    scheduleAt(simTime() + delay, rsbEle->refreshTimerMsg);
}

void RSVP::scheduleCommitTimer(ResvStateBlock_t *rsbEle)
{
    ASSERT(rsbEle);

    if (rsbEle->commitTimerMsg->isScheduled())
        cancelEvent(rsbEle->commitTimerMsg);

    scheduleAt(simTime(), rsbEle->commitTimerMsg);
}

RSVP::ResvStateBlock_t *RSVP::findRSB(const SessionObj_t& session, const SenderTemplateObj_t& sender, unsigned int& index)
{
    for (auto & elem : RSBList) {
        if (elem.Session_Object != session)
            continue;

        index = 0;
        for (auto fit = elem.FlowDescriptor.begin(); fit != elem.FlowDescriptor.end(); fit++) {
            if ((SenderTemplateObj_t&)fit->Filter_Spec_Object == sender) {
                return &(elem);
            }
            ++index;
        }

        // don't break here, may be in different (if outInterface is different)
    }
    return nullptr;
}

RSVP::PathStateBlock_t *RSVP::findPSB(const SessionObj_t& session, const SenderTemplateObj_t& sender)
{
    for (auto & elem : PSBList) {
        if ((elem.Session_Object == session) && (elem.Sender_Template_Object == sender))
            return &(elem);
    }
    return nullptr;
}

RSVP::PathStateBlock_t *RSVP::findPsbById(int id)
{
    for (auto & elem : PSBList) {
        if (elem.id == id)
            return &elem;
    }
    ASSERT(false);
    return nullptr;    // prevent warning
}

RSVP::ResvStateBlock_t *RSVP::findRsbById(int id)
{
    for (auto & elem : RSBList) {
        if (elem.id == id)
            return &elem;
    }
    ASSERT(false);
    return nullptr;    // prevent warning
}

RSVP::HelloState_t *RSVP::findHello(IPv4Address peer)
{
    for (auto & elem : HelloList) {
        if (elem.peer == peer)
            return &(elem);
    }
    return nullptr;
}

bool operator==(const SessionObj_t& a, const SessionObj_t& b)
{
    return a.DestAddress == b.DestAddress &&
           a.Tunnel_Id == b.Tunnel_Id &&
           a.Extended_Tunnel_Id == b.Extended_Tunnel_Id;
    // NOTE: don't compare holdingPri and setupPri; their placement
    // into Session_Object is only for our convenience
}

bool operator!=(const SessionObj_t& a, const SessionObj_t& b)
{
    return !operator==(a, b);
}

bool operator==(const FilterSpecObj_t& a, const FilterSpecObj_t& b)
{
    return a.SrcAddress == b.SrcAddress && a.Lsp_Id == b.Lsp_Id;
}

bool operator!=(const FilterSpecObj_t& a, const FilterSpecObj_t& b)
{
    return !operator==(a, b);
}

bool operator==(const SenderTemplateObj_t& a, const SenderTemplateObj_t& b)
{
    return a.SrcAddress == b.SrcAddress && a.Lsp_Id == b.Lsp_Id;
}

bool operator!=(const SenderTemplateObj_t& a, const SenderTemplateObj_t& b)
{
    return !operator==(a, b);
}

std::ostream& operator<<(std::ostream& os, const FlowSpecObj_t& a)
{
    os << "{bandwidth:" << a.req_bandwidth << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SessionObj_t& a)
{
    os << "{tunnelId:" << a.Tunnel_Id << "  exTunnelId:" << a.Extended_Tunnel_Id
       << "  destAddr:" << a.DestAddress << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SenderTemplateObj_t& a)
{
    os << "{lspid:" << a.Lsp_Id << "  sender:" << a.SrcAddress << "}";
    return os;
}

void RSVP::print(RSVPPathMsg *p)
{
    EV_INFO << "PATH_MESSAGE: lspid " << p->getLspId() << " ERO " << vectorToString(p->getERO()) << endl;
}

void RSVP::print(RSVPResvMsg *r)
{
    EV_INFO << "RESV_MESSAGE: " << endl;
    for (auto & elem : r->getFlowDescriptor()) {
        EV_INFO << " lspid " << elem.Filter_Spec_Object.Lsp_Id
                << " label " << elem.label << endl;
    }
}

void RSVP::clear()
{
    while (!PSBList.empty())
        removePSB(&PSBList.front());
    while (!RSBList.empty())
        removeRSB(&RSBList.front());
    while (!HelloList.empty())
        removeHello(&HelloList.front());
}

bool RSVP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
            setupHello();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            clear();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH)
            clear();
    }
    return true;
}

} // namespace inet

