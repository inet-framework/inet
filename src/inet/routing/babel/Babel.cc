//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/Babel.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"
#include "inet/routing/babel/BabelSerializer.h"

namespace inet {
namespace babel {

Define_Module(Babel);

Babel::~Babel()
{
    cancelAndDelete(triggeredUpdate);
    cancelAndDelete(bufferGc);
    deleteBuffers();
    deleteToAcks();
    // cancel and delete any timers still owned by the interfaces/neighbours
    for (auto iface : bit.getInterfaces()) {
        iface->deleteHTimer();
        iface->deleteUTimer();
    }
    for (auto neigh : bnt.getNeighbours()) {
        neigh->deleteNHTimer();
        neigh->deleteNITimer();
    }
}

void Babel::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        host = getContainingNode(this);
        ift.reference(this, "interfaceTableModule", true);
        if (par("ipv4RoutingTableModule").stringValue()[0])
            rt4.reference(this, "ipv4RoutingTableModule", false);
        if (par("ipv6RoutingTableModule").stringValue()[0])
            rt6.reference(this, "ipv6RoutingTableModule", false);
        udpPort = par("udpPort");
        seqno = intuniform(0, UINT16_MAX);
        socket.setOutputGate(gate("socketOut"));
        triggeredUpdate = new cMessage("BabelTriggeredUpdate");
        bufferGc = createTimer(timerT::BUFFERGC, nullptr);
        host->subscribe(interfaceStateChangedSignal, this);
        host->subscribe(ipv6AddressAssignedSignal, this);
        host->subscribe(routeDeletedSignal, this); // the table may drop our routes (e.g. on interface down)
        WATCH(routerId);
        WATCH(seqno);
        WATCH(udpPort);
        WATCH_PTRVECTOR(bit.getInterfaces());
        WATCH_PTRVECTOR(bnt.getNeighbours());
        WATCH_PTRVECTOR(btt.getRoutes());
        WATCH_PTRVECTOR(bst.getSources());
        WATCH_PTRVECTOR(bpsrt.getRequests());
        WATCH_PTRVECTOR(buffers);
        WATCH_PTRVECTOR(ackwait);
    }
}

void Babel::finish()
{
    EV_INFO << "Babel on " << host->getFullName() << " (router-id " << routerId << "):" << endl;
    for (auto neigh : bnt.getNeighbours())
        EV_INFO << "  neighbour " << neigh->str() << endl;
    for (auto route : btt.getRoutes())
        EV_INFO << "  route " << route->str() << endl;
}

void Babel::setMainInterface()
{
    if (ift->getNumInterfaces() <= 0)
        throw cRuntimeError("Babel: the host has no interfaces");
    // pick the first non-loopback interface as the source of the router-id
    mainInterface = nullptr;
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        NetworkInterface *ie = ift->getInterface(i);
        if (!ie->isLoopback()) {
            mainInterface = ie;
            break;
        }
    }
    if (mainInterface == nullptr)
        mainInterface = ift->getInterface(0);
}

void Babel::generateRouterId()
{
    if (mainInterface != nullptr) {
        const InterfaceToken& token = mainInterface->getInterfaceToken();
        routerId.setRid(token.normal(), token.low());
    }
    else
        routerId.setRid(intuniform(0, INT32_MAX), intuniform(0, INT32_MAX));
    EV_INFO << "Babel: router-id is " << routerId << "." << endl;
}

void Babel::configureInterfaces()
{
    // run Babel on every multicast, non-loopback interface, in the address
    // families the node actually supports (dual-stack when both are present)
    bool hasV4 = rt4.getNullable() != nullptr;
    bool hasV6 = rt6.getNullable() != nullptr;
    int af = (hasV4 && hasV6) ? AF::IPvX : (hasV4 ? AF::IPv4 : AF::IPv6);

    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        NetworkInterface *ie = ift->getInterface(i);
        if (ie->isMulticast() && !ie->isLoopback()) {
            BabelInterface *biface = new BabelInterface(ie, seqno);
            biface->setAfSend(af);
            biface->setAfDist(af);
            biface->setWired(!ie->isWireless());
            biface->setNominalRxcost(ie->isWireless() ? defval::NOM_RXCOST_WIRELESS : defval::NOM_RXCOST_WIRED);
            biface->setCostComputationModule(ie->isWireless()
                    ? static_cast<IBabelCostComputation *>(&wirelessCost)
                    : static_cast<IBabelCostComputation *>(&wiredCost));
            bit.addInterface(biface);
        }
    }
}

void Babel::handleStartOperation(LifecycleOperation *operation)
{
    // done here (rather than in initialize()) so the interfaces are registered
    // and operational before Babel is configured on them
    setMainInterface();
    generateRouterId();
    configureInterfaces();

    socket.setMulticastLoop(false);
    socket.setTimeToLive(1); // Babel TLVs are link-local (RFC 6126)
    socket.bind(udpPort);

    for (auto biface : bit.getInterfaces())
        activateInterface(biface);

    originateConnectedRoutes();
    selectRoutes();

    resetTimer(bufferGc, defval::BUFFER_GC_INTERVAL);

    EV_INFO << "Babel started on UDP port " << udpPort << "." << endl;
}

void Babel::handleStopOperation(LifecycleOperation *operation)
{
    for (auto biface : bit.getInterfaces())
        deactivateInterface(biface);
    if (bufferGc->isScheduled())
        cancelEvent(bufferGc);
    deleteBuffers();
    deleteToAcks();
    socket.close();
}

void Babel::handleCrashOperation(LifecycleOperation *operation)
{
    if (bufferGc->isScheduled())
        cancelEvent(bufferGc);
    deleteBuffers();
    deleteToAcks();
    socket.destroy();
}

void Babel::activateInterface(BabelInterface *iface)
{
    ASSERT(iface != nullptr);
    if (!iface->getInterface()->isUp())
        return;

    std::vector<L3Address> groups;
    multicastGroupsFor(iface, groups);
    for (const auto& group : groups)
        socket.joinMulticastGroup(group, iface->getInterfaceId());

    if (iface->getAfSend() != AF::NONE) {
        if (iface->getHTimer() == nullptr)
            iface->setHTimer(createTimer(timerT::HELLO, iface));
        iface->resetHTimer(uniform(SEND_URGENT, CStoS(iface->getHInterval()))); // randomized first Hello

        if (iface->getUTimer() == nullptr)
            iface->setUTimer(createTimer(timerT::UPDATE, iface));
        iface->resetUTimer(uniform(SEND_URGENT, CStoS(iface->getUInterval()))); // randomized first Update
    }

    iface->setEnabled(true);

    // say hello and request a full route dump in each address family
    if (iface->getAfSend() != AF::NONE) {
        uint16_t hseqno = iface->getIncHSeqno();
        for (const auto& group : groups) {
            std::vector<Ptr<BabelTlv>> tlvs;
            auto hello = makeShared<BabelHelloTlv>();
            hello->setSeqno(hseqno);
            hello->setInterval(iface->getHInterval());
            tlvs.push_back(hello);
            sendTLVs(group, iface, tlvs);

            sendRouteReq(iface, group, AE::WILDCARD, netPrefix<L3Address>());
        }
    }

    EV_INFO << "Babel: interface " << iface->getIfaceName() << " activated." << endl;
}

void Babel::deactivateInterface(BabelInterface *iface)
{
    ASSERT(iface != nullptr);
    iface->deleteHTimer();
    iface->deleteUTimer();

    // retract (do not delete) the routes learned via this interface, so that
    // selectRoutes() can request a fresh seqno and switch to an alternate path;
    // the neighbours themselves time out on their own Hello timers
    bool changed = btt.retractRoutesOnIface(iface);

    iface->setEnabled(false);
    if (changed) {
        selectRoutes();
        triggerUpdate();
    }
}

cMessage *Babel::createTimer(short kind, void *context)
{
    cMessage *timer = new cMessage(timerT::toStr(kind).c_str(), kind);
    timer->setContextPointer(context);
    return timer;
}

void Babel::handleMessageWhenUp(cMessage *msg)
{
    if (msg == triggeredUpdate)
        sendFullDump(nullptr); // propagate accumulated changes on all interfaces
    else if (msg->isSelfMessage())
        processTimer(msg);
    else if (msg->getKind() == UDP_I_DATA)
        processUdpPacket(check_and_cast<Packet *>(msg));
    else if (msg->getKind() == UDP_I_ERROR) {
        EV_DETAIL << "Babel: ignoring UDP error report." << endl;
        delete msg;
    }
    else if (msg->getKind() == UDP_I_SOCKET_CLOSED) {
        EV_DETAIL << "Babel: ignoring UDP socket closed indication." << endl;
        delete msg;
    }
    else
        throw cRuntimeError("Babel: unknown message kind (%d)", (int)msg->getKind());
}

void Babel::processTimer(cMessage *timer)
{
    switch (timer->getKind()) {
        case timerT::HELLO:
            sendHello(check_and_cast<BabelInterface *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        case timerT::UPDATE:
            processUpdateTimer(check_and_cast<BabelInterface *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        case timerT::NEIGHHELLO:
            processNeighHelloTimer(check_and_cast<BabelNeighbour *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        case timerT::NEIGHIHU:
            processNeighIhuTimer(check_and_cast<BabelNeighbour *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        case timerT::ROUTEEXPIRY:
            processRouteExpiryTimer(check_and_cast<BabelRoute *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        case timerT::ROUTEBEFEXPIRY:
            processBefRouteExpiryTimer(check_and_cast<BabelRoute *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        case timerT::SOURCEGC:
            processSourceGCTimer(check_and_cast<BabelSource *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        case timerT::SRRESEND:
            processRSResendTimer(check_and_cast<BabelPenSR *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        case timerT::BUFFER:
            flushBuffer(check_and_cast<BabelBuffer *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        case timerT::BUFFERGC:
            deleteUnusedBuffers();
            resetTimer(bufferGc, defval::BUFFER_GC_INTERVAL);
            break;
        case timerT::TOACKRESEND:
            checkAndResendToAck(check_and_cast<BabelToAck *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        default:
            throw cRuntimeError("Babel: received timer of unknown type: %d", timer->getKind());
    }
}

void Babel::processUpdateTimer(BabelInterface *iface)
{
    // re-scan connected subnets: IPv6 global addresses become valid only after
    // duplicate-address detection completes, i.e. some time after startup
    originateConnectedRoutes();
    selectRoutes();

    sendFullDump(iface);
    iface->resetUTimer();
}

void Babel::processRouteExpiryTimer(BabelRoute *route)
{
    if (route->metric() != COST_INF) {
        // first expiry: mark as retracted and re-advertise, keep the entry around briefly
        route->getRDistance().setMetric(COST_INF);
        route->resetETimer();
        sendUpdate(nullptr, route, true); // reliably retract on all interfaces
        selectRoutes();
    }
    else {
        // second expiry: remove for good
        if (route->getRTEntry())
            removeFromRT(route);
        btt.removeRoute(route);
        selectRoutes();
    }
}

void Babel::processBefRouteExpiryTimer(BabelRoute *route)
{
    // proactively refresh the route before it expires
    if (route->getNeighbour() != nullptr)
        sendRouteReq(route->getNeighbour()->getInterface(), route->getNeighbour()->getAddress(),
                getAeOfPrefix(route->getPrefix().getAddr()), route->getPrefix());
}

void Babel::processSourceGCTimer(BabelSource *source)
{
    bst.removeSource(source);
}

void Babel::processNeighHelloTimer(BabelNeighbour *neigh)
{
    neigh->noteLoss();
    neigh->setExpHSeqno(plusmod16(neigh->getExpHSeqno(), 1));

    bool changed = false;
    if (neigh->getHistory() == 0) {
        EV_INFO << "Babel: neighbour " << neigh->getAddress() << " on " << neigh->getInterface()->getIfaceName() << " lost." << endl;
        bpsrt.removePenSRsByNeigh(neigh);
        changed = removeRoutesByNeigh(neigh);
        bnt.removeNeighbour(neigh);
    }
    else {
        neigh->resetNHTimer();
        changed = neigh->recomputeCost();
    }

    if (changed) {
        selectRoutes();
        triggerUpdate();
    }
}

void Babel::processNeighIhuTimer(BabelNeighbour *neigh)
{
    neigh->setTxcost(COST_INF);
    if (neigh->recomputeCost()) {
        selectRoutes();
        triggerUpdate();
    }
}

void Babel::sendHello(BabelInterface *iface)
{
    uint16_t hseqno = iface->getIncHSeqno();
    bool periodicIhu = (hseqno % defval::IHU_INTERVAL_MULT == 0);

    std::vector<L3Address> groups;
    multicastGroupsFor(iface, groups);

    // send a Hello to each address family's group, with the IHUs of the
    // neighbours discovered in that family appended
    for (const auto& group : groups) {
        bool groupIsV6 = group.getType() == L3Address::IPv6;

        std::vector<Ptr<BabelTlv>> tlvs;
        auto hello = makeShared<BabelHelloTlv>();
        hello->setSeqno(hseqno);
        hello->setInterval(iface->getHInterval());
        tlvs.push_back(hello);

        for (auto neigh : bnt.getNeighbours()) {
            if (neigh->getInterface() != iface)
                continue;
            if ((neigh->getAddress().getType() == L3Address::IPv6) != groupIsV6)
                continue; // IHU goes to its neighbour's own address family
            bool lossy = ((neigh->getHistory() & 0xF000) != 0xF000) || neigh->getTxcost() >= 384;
            if (periodicIhu || lossy) {
                auto ihu = makeShared<BabelIhuTlv>();
                ihu->setAe(getAeOfAddr(neigh->getAddress()));
                ihu->setRxcost(neigh->computeRxcost());
                ihu->setInterval(defval::IHU_INTERVAL_MULT * iface->getHInterval());
                ihu->setAddress(neigh->getAddress());
                tlvs.push_back(ihu);
            }
        }

        sendTLVs(group, iface, tlvs);
    }

    iface->resetHTimer();
}

BabelBuffer *Babel::findOrCreateBuffer(const L3Address& dst, BabelInterface *iface)
{
    for (auto buff : buffers)
        if (buff->getDst() == dst && buff->getOutIface()->getInterfaceId() == iface->getInterfaceId())
            return buff;
    BabelBuffer *buff = new BabelBuffer(dst, iface, createTimer(timerT::BUFFER, nullptr));
    buff->getFlushTimer()->setContextPointer(buff);
    buffers.push_back(buff);
    return buff;
}

void Babel::sendTLVs(const L3Address& dst, BabelInterface *iface, const std::vector<Ptr<BabelTlv>>& tlvs, double maxtime, bool reliable)
{
    if (!iface->getEnabled() || iface->getAfSend() == AF::NONE || tlvs.empty())
        return;

    BabelBuffer *buff = findOrCreateBuffer(dst, iface);

    // a packet must not carry two Hellos -> flush what is buffered first
    bool addingHello = false;
    for (const auto& tlv : tlvs)
        if (tlv->getTlvType() == BABEL_HELLO) { addingHello = true; break; }
    if (addingHello && buff->containsHello())
        flushBuffer(buff);

    for (const auto& tlv : tlvs)
        buff->addTlv(tlv);
    if (reliable)
        buff->setNeedsAck(true);

    BabelTimer *ft = buff->getFlushTimer();
    if (maxtime == SEND_NOW)
        flushBuffer(buff);
    else {
        double delay = (maxtime == SEND_BUFFERED) ? CStoS(iface->getHInterval() / defval::BUFFER_MT_DIVISOR) : maxtime;
        // jitter the deadline so that distinct buffers (and other nodes) do not flush at
        // exactly the same instant -- important on a shared wireless medium (RFC 6126, 3.1)
        delay *= uniform(0.5, 1.0);
        if (!ft->isScheduled())
            scheduleAfter(delay, ft);
        else if ((ft->getArrivalTime() - simTime()).dbl() > delay)
            rescheduleAfter(delay, ft);
    }
}

void Babel::flushBuffer(BabelBuffer *buff)
{
    auto& tlvs = buff->getTlvs();
    const L3Address& dst = buff->getDst();
    BabelInterface *iface = buff->getOutIface();

    // a reliable flush leads with an Acknowledgment Request (RFC 6126, 3.1)
    uint16_t nonce = 0;
    if (buff->getNeedsAck()) {
        nonce = generateNonce();
        auto ackreq = makeShared<BabelAckReqTlv>();
        ackreq->setNonce(nonce);
        ackreq->setInterval(iface->getHInterval() / 2);
        tlvs.insert(tlvs.begin(), ackreq);
    }

    bool v6 = dst.getType() == L3Address::IPv6;
    int maxbody = iface->getInterface()->getMtu()
            - (v6 ? IPV6_HEADER_SIZE : IPV4_HEADER_SIZE) - UDP_HEADER_SIZE - BABEL_HEADER_SIZE;
    if (maxbody < 512 - BABEL_HEADER_SIZE)
        maxbody = 512 - BABEL_HEADER_SIZE;

    // pack the queued TLVs into as few messages as the MTU allows
    size_t i = 0;
    bool firstGroup = true;
    while (i < tlvs.size()) {
        std::vector<Ptr<BabelTlv>> group;
        int bodyLen = 0;
        while (i < tlvs.size()) {
            int len = babelTlvLength(tlvs[i].get()).get<B>();
            if (!group.empty() && bodyLen + len > maxbody)
                break;
            group.push_back(tlvs[i]);
            bodyLen += len;
            ++i;
        }
        sendBabelMessage(dst, iface, group);

        // the first message carries the Acknowledgment Request -> track it for retransmission
        if (firstGroup && buff->getNeedsAck()) {
            BabelToAck *toack = new BabelToAck(nonce, defval::RESEND_NUM, createTimer(timerT::TOACKRESEND, nullptr), dst, iface, group);
            if (dst.isMulticast()) {
                for (auto neigh : bnt.getNeighbours())
                    if (neigh->getInterface() == iface && (neigh->getAddress().getType() == L3Address::IPv6) == v6)
                        toack->addDstNode(neigh->getAddress());
            }
            else
                toack->addDstNode(dst);

            if (toack->dstNodesSize() > 0) {
                ackwait.push_back(toack);
                toack->resetResendTimer(CStoS(iface->getHInterval() / 2));
            }
            else
                delete toack; // nobody to acknowledge
        }
        firstGroup = false;
    }

    buff->clear();
    if (buff->getFlushTimer()->isScheduled())
        cancelEvent(buff->getFlushTimer());
}

void Babel::deleteUnusedBuffers()
{
    for (auto it = buffers.begin(); it != buffers.end();) {
        BabelTimer *ft = (*it)->getFlushTimer();
        if (ft == nullptr || !ft->isScheduled()) {
            delete *it;
            it = buffers.erase(it);
        }
        else
            ++it;
    }
}

void Babel::deleteBuffers()
{
    for (auto buff : buffers)
        delete buff;
    buffers.clear();
}

void Babel::sendBabelMessage(const L3Address& dst, BabelInterface *iface, const std::vector<Ptr<BabelTlv>>& tlvs)
{
    if (!iface->getEnabled() || !iface->getInterface()->isUp() || iface->getAfSend() == AF::NONE || tlvs.empty())
        return;

    B bodyLength = B(0);
    for (auto& tlv : tlvs) {
        if (tlv->isMutable()) // already immutable when this is a retransmission
            tlv->setChunkLength(babelTlvLength(tlv.get()));
        bodyLength += tlv->getChunkLength();
    }

    auto header = makeShared<BabelHeader>();
    header->setBodyLength(bodyLength.get<B>());

    auto packet = new Packet("babel");
    packet->insertAtBack(header);
    for (auto& tlv : tlvs)
        packet->insertAtBack(tlv);

    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(iface->getInterfaceId());
    if (dst.getType() == L3Address::IPv6) {
        auto ipv6Data = iface->getInterface()->findProtocolData<Ipv6InterfaceData>();
        if (ipv6Data != nullptr)
            packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(ipv6Data->getLinkLocalAddress());
    }

    EV_INFO << "Babel: sending " << tlvs.size() << " TLV(s) to " << dst << " via " << iface->getIfaceName() << "." << endl;
    socket.sendTo(packet, dst, udpPort);
}

void Babel::processUdpPacket(Packet *packet)
{
    auto interfaceInd = packet->findTag<InterfaceInd>();
    auto l3AddressInd = packet->findTag<L3AddressInd>();
    if (interfaceInd == nullptr || l3AddressInd == nullptr) {
        delete packet;
        return;
    }
    int interfaceId = interfaceInd->getInterfaceId();
    L3Address src = l3AddressInd->getSrcAddress();
    L3Address dst = l3AddressInd->getDestAddress();

    BabelInterface *iface = bit.findInterfaceById(interfaceId);
    if (iface == nullptr || !iface->getEnabled() || !iface->getInterface()->isUp()) {
        delete packet;
        return;
    }

    auto header = packet->popAtFront<BabelHeader>();
    if (header->getMagic() != defval::MAGIC || header->getVersion() != defval::VERSION) {
        EV_WARN << "Babel: dropping message with bad magic/version." << endl;
        delete packet;
        return;
    }

    // context carried across the TLVs of one message (RFC 6126, section 4.5)
    bool changed = false;
    rid prevRouterId;
    L3Address prevNextHop4 = (src.getType() == L3Address::IPv4) ? src : L3Address();
    L3Address prevNextHop6 = (src.getType() == L3Address::IPv6) ? src : L3Address();

    while (packet->getDataLength() > b(0)) {
        auto tlv = packet->popAtFront<BabelTlv>();
        switch (tlv->getTlvType()) {
            case BABEL_HELLO:
                processHello(staticPtrCast<const BabelHelloTlv>(tlv), iface, src);
                break;
            case BABEL_IHU:
                processIhu(staticPtrCast<const BabelIhuTlv>(tlv), iface, src, dst);
                break;
            case BABEL_ROUTERID: {
                auto t = staticPtrCast<const BabelRouterIdTlv>(tlv);
                prevRouterId.setRid(t->getRouterIdHi(), t->getRouterIdLo());
                break;
            }
            case BABEL_NEXTHOP: {
                auto t = staticPtrCast<const BabelNextHopTlv>(tlv);
                if (t->getAe() == AE::IPv4)
                    prevNextHop4 = t->getNextHop();
                else if (t->getAe() == AE::IPv6 || t->getAe() == AE::LLIPv6)
                    prevNextHop6 = t->getNextHop();
                break;
            }
            case BABEL_UPDATE: {
                auto t = staticPtrCast<const BabelUpdateTlv>(tlv);
                const L3Address& nh = (t->getAe() == AE::IPv4) ? prevNextHop4 : prevNextHop6;
                changed = processUpdate(t, iface, src, prevRouterId, nh) || changed;
                break;
            }
            case BABEL_ROUTEREQ:
                processRouteReq(staticPtrCast<const BabelRouteReqTlv>(tlv), iface, src, dst);
                break;
            case BABEL_SEQNOREQ:
                processSeqnoReq(staticPtrCast<const BabelSeqnoReqTlv>(tlv), iface, src);
                break;
            case BABEL_ACKREQ:
                processAckReq(staticPtrCast<const BabelAckReqTlv>(tlv), iface, src);
                break;
            case BABEL_ACK:
                processAck(staticPtrCast<const BabelAckTlv>(tlv), src);
                break;
            default:
                // Pad1/PadN: nothing to do
                break;
        }
    }

    if (changed) {
        selectRoutes();
        triggerUpdate();
    }

    delete packet;
}

void Babel::processHello(const Ptr<const BabelHelloTlv>& hello, BabelInterface *iface, const L3Address& src)
{
    uint16_t hseqno = hello->getSeqno();
    uint16_t interval = hello->getInterval();

    BabelNeighbour *neigh = bnt.findNeighbour(iface, src);
    if (neigh == nullptr) {
        neigh = bnt.addNeighbour(new BabelNeighbour(iface, src,
                createTimer(timerT::NEIGHHELLO, nullptr),
                createTimer(timerT::NEIGHIHU, nullptr)));
        EV_INFO << "Babel: new neighbour " << src << " on " << iface->getIfaceName() << "." << endl;
    }
    else if (hseqno != neigh->getExpHSeqno()) {
        if (abs(minusmod16(hseqno, neigh->getExpHSeqno())) > HISTORY_LEN)
            neigh->setHistory(0); // sender probably rebooted
        else if (comparemod16(hseqno, neigh->getExpHSeqno()) == -1)
            neigh->setHistory(neigh->getHistory() << minusmod16(neigh->getExpHSeqno(), hseqno));
        else if (comparemod16(hseqno, neigh->getExpHSeqno()) == 1)
            neigh->setHistory(neigh->getHistory() >> minusmod16(hseqno, neigh->getExpHSeqno()));
    }

    neigh->noteReceive();
    neigh->setNeighHelloInterval(interval);
    neigh->setExpHSeqno(plusmod16(hseqno, 1));
    neigh->resetNHTimer(CStoS(interval) * 1.5); // extra margin to absorb jitter

    // first message from a neighbour -> answer immediately (also sends an IHU due to poor history)
    if ((neigh->getHistory() & 0xBF00) == 0x8000)
        sendHello(iface);

    // second message from a neighbour -> rxcost is now finite, pull its routes
    if ((neigh->getHistory() & 0xFC00) == 0xC000)
        sendRouteReq(iface, src, AE::WILDCARD, netPrefix<L3Address>());

    if (neigh->recomputeCost())
        selectRoutes();
}

void Babel::processIhu(const Ptr<const BabelIhuTlv>& ihu, BabelInterface *iface, const L3Address& src, const L3Address& dst)
{
    int ae = ihu->getAe();
    uint16_t rxcost = ihu->getRxcost();
    uint16_t interval = ihu->getInterval();

    L3Address address;
    if (ae == AE::WILDCARD) {
        if (dst.isMulticast())
            return; // a wildcard IHU must be unicast
        address = dst;
    }
    else
        address = ihu->getAddress();

    if (!isMyAddressOnInterface(address, iface))
        return; // not addressed to me on this interface

    BabelNeighbour *neigh = bnt.findNeighbour(iface, src);
    if (neigh != nullptr) {
        neigh->setTxcost(rxcost);
        neigh->setNeighIhuInterval(interval);
        neigh->resetNITimer(CStoS(defval::IHU_HOLD_INTERVAL_MULT * interval));
        neigh->recomputeCost();
    }
}

bool Babel::isMyAddressOnInterface(const L3Address& address, BabelInterface *iface) const
{
    NetworkInterface *ie = iface->getInterface();
    if (address.getType() == L3Address::IPv6) {
        auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>();
        return ipv6Data != nullptr && ipv6Data->hasAddress(address.toIpv6());
    }
    else {
        auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>();
        return ipv4Data != nullptr && ipv4Data->getIPAddress() == address.toIpv4();
    }
}

L3Address Babel::interfaceAddressForAf(BabelInterface *iface, int af) const
{
    NetworkInterface *ie = iface->getInterface();
    if (af == AF::IPv6) {
        auto d = ie->findProtocolData<Ipv6InterfaceData>();
        return d ? L3Address(d->getLinkLocalAddress()) : L3Address();
    }
    else {
        auto d = ie->findProtocolData<Ipv4InterfaceData>();
        return d ? L3Address(d->getIPAddress()) : L3Address();
    }
}

void Babel::multicastGroupsFor(BabelInterface *iface, std::vector<L3Address>& groups) const
{
    int af = iface->getAfSend();
    if (af == AF::IPv4 || af == AF::IPvX)
        groups.push_back(defval::MCASTG4);
    if (af == AF::IPv6 || af == AF::IPvX)
        groups.push_back(defval::MCASTG6);
}

// ---- route origination ----

void Babel::originateConnectedRoutes()
{
    // originate every directly-connected subnet on a Babel interface as a local route
    auto originate = [&](const netPrefix<L3Address>& prefix) {
        if (prefix.getAddr().isUnspecified() || prefix.getAddr().isMulticast() || prefix.getAddr().isLinkLocal())
            return;
        BabelRoute *local = new BabelRoute(prefix, nullptr, routerId, routeDistance(seqno, 0), L3Address(), 0, nullptr, nullptr);
        local->setSelected(true);
        if (btt.addRoute(local) != local)
            delete local;
        else
            EV_INFO << "Babel: originating local route " << prefix << "." << endl;
    };

    for (auto biface : bit.getInterfaces()) {
        NetworkInterface *ie = biface->getInterface();

        // IPv6: prefer the interface's advertised (on-link) prefixes; otherwise derive
        // the connected /64 subnet from each configured global address
        if (auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>()) {
            if (ipv6Data->getNumAdvPrefixes() > 0) {
                for (int i = 0; i < ipv6Data->getNumAdvPrefixes(); i++) {
                    const Ipv6InterfaceData::AdvPrefix& ap = ipv6Data->getAdvPrefix(i);
                    originate(netPrefix<L3Address>(L3Address(ap.prefix), ap.prefixLength));
                }
            }
            else {
                for (int i = 0; i < ipv6Data->getNumAddresses(); i++) {
                    Ipv6Address addr = ipv6Data->getAddress(i);
                    if (!addr.isLinkLocal() && !addr.isMulticast() && !addr.isUnspecified())
                        originate(netPrefix<L3Address>(L3Address(addr), 64));
                }
            }
        }

        // IPv4: derive the connected subnet from the interface address and netmask
        if (auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>()) {
            Ipv4Address addr = ipv4Data->getIPAddress();
            Ipv4Address mask = ipv4Data->getNetmask();
            if (!addr.isUnspecified() && !mask.isUnspecified())
                originate(netPrefix<L3Address>(L3Address(addr), mask.getNetmaskLength()));
        }
    }
}

// ---- update send ----

void Babel::sendUpdateMessage(const L3Address& dst, BabelInterface *iface, const rid& originator,
        const L3Address& nexthop, const netPrefix<L3Address>& prefix, const routeDistance& dist, uint16_t interval, bool reliable)
{
    // record the feasibility distance for everything we advertise (RFC 6126, 3.7.3)
    if (dist.getMetric() != COST_INF)
        addOrUpdateSource(prefix, originator, dist);

    std::vector<Ptr<BabelTlv>> tlvs;

    auto ridTlv = makeShared<BabelRouterIdTlv>();
    ridTlv->setRouterIdHi(originator.getRid()[0]);
    ridTlv->setRouterIdLo(originator.getRid()[1]);
    tlvs.push_back(ridTlv);

    auto nhTlv = makeShared<BabelNextHopTlv>();
    nhTlv->setAe(getAeOfAddr(nexthop));
    nhTlv->setNextHop(nexthop);
    tlvs.push_back(nhTlv);

    auto upd = makeShared<BabelUpdateTlv>();
    upd->setAe(getAeOfPrefix(prefix.getAddr()));
    upd->setInterval(interval);
    upd->setSeqno(dist.getSeqno());
    upd->setMetric(dist.getMetric());
    upd->setPrefix(prefix.getAddr());
    upd->setPrefixLen(prefix.getLen());
    tlvs.push_back(upd);

    sendTLVs(dst, iface, tlvs, SEND_URGENT, reliable);
}

void Babel::sendUpdate(BabelInterface *iface, BabelRoute *route, const L3Address& dst, bool reliable)
{
    // split horizon: don't echo a route back to the neighbour it was learned from
    if (route->getNeighbour() != nullptr && iface->getSplitHorizon() && route->getNeighbour()->getInterface() == iface)
        return;

    int af = (route->getPrefix().getAddr().getType() == L3Address::IPv4) ? AF::IPv4 : AF::IPv6;
    L3Address nh = interfaceAddressForAf(iface, af);
    if (nh.isUnspecified())
        return; // the interface has no address of this family

    sendUpdateMessage(dst, iface, route->getOriginator(), nh, route->getPrefix(),
            routeDistance(route->getRDistance().getSeqno(), route->metric()), iface->getUInterval(), reliable);
}

void Babel::sendUpdate(BabelInterface *iface, BabelRoute *route, bool reliable)
{
    if (iface == nullptr) {
        for (auto bi : bit.getInterfaces())
            if (bi->getEnabled() && bi->getAfSend() != AF::NONE)
                sendUpdate(bi, route, reliable);
        return;
    }
    bool v4prefix = route->getPrefix().getAddr().getType() == L3Address::IPv4;
    sendUpdate(iface, route, v4prefix ? defval::MCASTG4 : defval::MCASTG6, reliable);
}

void Babel::sendFullDump(BabelInterface *iface)
{
    if (iface == nullptr) {
        for (auto bi : bit.getInterfaces())
            if (bi->getEnabled() && bi->getAfSend() != AF::NONE)
                sendFullDump(bi);
        return;
    }
    for (auto route : btt.getRoutes())
        if (route->getSelected())
            sendUpdate(iface, route);
}

void Babel::sendRouteReq(BabelInterface *iface, const L3Address& dst, int ae, const netPrefix<L3Address>& prefix)
{
    auto req = makeShared<BabelRouteReqTlv>();
    req->setAe(ae);
    if (ae != AE::WILDCARD) {
        req->setPrefix(prefix.getAddr());
        req->setPrefixLen(prefix.getLen());
    }
    std::vector<Ptr<BabelTlv>> tlvs;
    tlvs.push_back(req);
    sendTLVs(dst, iface, tlvs);
}

void Babel::triggerUpdate()
{
    // damp a burst of changes into a single update shortly afterwards (RFC 6126, 3.7.1)
    if (triggeredUpdate != nullptr && !triggeredUpdate->isScheduled())
        scheduleAfter(SEND_URGENT, triggeredUpdate);
}

// ---- seqno requests (RFC 6126, 3.8) ----

void Babel::incSeqno()
{
    seqno = plusmod16(seqno, 1);
    for (auto route : btt.getRoutes())
        if (route->getNeighbour() == nullptr)
            route->getRDistance().setSeqno(seqno);
}

void Babel::sendSeqnoReq(BabelInterface *iface, const L3Address& dst, const netPrefix<L3Address>& prefix,
        const rid& orig, uint16_t reqSeqno, uint8_t hopcount, BabelNeighbour *recfrom)
{
    if (iface == nullptr) {
        for (auto bi : bit.getInterfaces())
            if (bi->getEnabled() && bi->getAfSend() != AF::NONE)
                sendSeqnoReq(bi, dst, prefix, orig, reqSeqno, hopcount, recfrom);
        return;
    }

    L3Address actualDst = !dst.isUnspecified() ? dst
            : (prefix.getAddr().getType() == L3Address::IPv4 ? defval::MCASTG4 : defval::MCASTG6);

    // remember the pending request (deduplicated per prefix+interface) with a resend timer
    if (bpsrt.findPenSR(prefix, iface) == nullptr) {
        BabelPenSR *pensr = new BabelPenSR(prefix, orig, reqSeqno, hopcount, recfrom,
                defval::RESEND_NUM, iface, actualDst, createTimer(timerT::SRRESEND, nullptr));
        pensr->resetResendTimer();
        if (bpsrt.addPenSR(pensr) != pensr)
            delete pensr;
    }

    auto sr = makeShared<BabelSeqnoReqTlv>();
    sr->setAe(getAeOfPrefix(prefix.getAddr()));
    sr->setSeqno(reqSeqno);
    sr->setHopcount(hopcount);
    sr->setRouterIdHi(orig.getRid()[0]);
    sr->setRouterIdLo(orig.getRid()[1]);
    sr->setPrefix(prefix.getAddr());
    sr->setPrefixLen(prefix.getLen());

    std::vector<Ptr<BabelTlv>> tlvs;
    tlvs.push_back(sr);
    sendTLVs(actualDst, iface, tlvs);
}

void Babel::processSeqnoReq(const Ptr<const BabelSeqnoReqTlv>& req, BabelInterface *iface, const L3Address& src)
{
    int ae = req->getAe();
    if (ae != AE::IPv4 && ae != AE::IPv6)
        return;

    netPrefix<L3Address> prefix(req->getPrefix(), req->getPrefixLen());
    uint16_t reqSeqno = req->getSeqno();
    uint8_t hopcount = req->getHopcount();
    rid origrid(req->getRouterIdHi(), req->getRouterIdLo());

    if (bpsrt.findPenSR(prefix) != nullptr)
        return; // we are already (forwarding) a request for this prefix -> suppress duplicate

    BabelRoute *intable = btt.findSelectedRoute(prefix);
    if (intable == nullptr || intable->getRDistance().getMetric() == COST_INF)
        return;

    if (origrid != intable->getOriginator() || comparemod16(reqSeqno, intable->getRDistance().getSeqno()) != 1) {
        // we already hold a good-enough route -> answer with an update
        sendUpdate(iface, intable);
    }
    else if (origrid == routerId) {
        // we originate this prefix -> bump our seqno and re-advertise
        incSeqno();
        sendUpdate(iface, intable);
    }
    else {
        // forward the request along another route, if any (RFC 6126, 3.8.2.3)
        BabelRoute *another = btt.findRouteNotNH(prefix, src);
        BabelNeighbour *neigh = bnt.findNeighbour(iface, src);
        if (another != nullptr && another->getNeighbour() != nullptr && neigh != nullptr && hopcount >= 2)
            sendSeqnoReq(another->getNeighbour()->getInterface(), another->getNeighbour()->getAddress(),
                    prefix, origrid, reqSeqno, hopcount - 1, neigh);
    }
}

void Babel::processRSResendTimer(BabelPenSR *request)
{
    if (request->decResendNum() >= 0) {
        sendSeqnoReq(request->getOutIface(), request->getForwardTo(), request->getPrefix(),
                request->getOriginator(), request->getReqSeqno(), request->getHopcount(), request->getReceivedFrom());
        request->resetResendTimer();
    }
    else
        bpsrt.removePenSR(request);
}

// ---- reliable delivery / acknowledgments (RFC 6126, 3.1) ----

uint16_t Babel::generateNonce()
{
    uint16_t nonce;
    do {
        nonce = intuniform(0, UINT16_MAX);
    } while (findToAck(nonce) != nullptr);
    return nonce;
}

BabelToAck *Babel::findToAck(uint16_t n)
{
    for (auto t : ackwait)
        if (t->getNonce() == n)
            return t;
    return nullptr;
}

void Babel::deleteToAck(BabelToAck *todel)
{
    for (auto it = ackwait.begin(); it != ackwait.end(); ++it) {
        if (*it == todel) {
            delete *it;
            ackwait.erase(it);
            return;
        }
    }
}

void Babel::deleteToAcks()
{
    for (auto t : ackwait)
        delete t;
    ackwait.clear();
}

void Babel::checkAndResendToAck(BabelToAck *toack)
{
    if (toack->decResendNum() >= 0 && toack->dstNodesSize() > 0) {
        sendBabelMessage(toack->getDst(), toack->getOutIface(), toack->getTlvs());
        toack->resetResendTimer(CStoS(toack->getOutIface()->getHInterval() / 2));
    }
    else
        deleteToAck(toack);
}

void Babel::processAckReq(const Ptr<const BabelAckReqTlv>& req, BabelInterface *iface, const L3Address& src)
{
    // acknowledge by sending an Ack with the same nonce back to the requester
    auto ack = makeShared<BabelAckTlv>();
    ack->setNonce(req->getNonce());
    std::vector<Ptr<BabelTlv>> tlvs;
    tlvs.push_back(ack);
    sendTLVs(src, iface, tlvs, CStoS(req->getInterval() / 2));
}

void Babel::processAck(const Ptr<const BabelAckTlv>& ack, const L3Address& src)
{
    BabelToAck *toack = findToAck(ack->getNonce());
    if (toack != nullptr) {
        toack->removeDstNode(src);
        if (toack->dstNodesSize() == 0) // everybody acknowledged -> done
            deleteToAck(toack);
    }
}

// ---- update receive ----

bool Babel::processUpdate(const Ptr<const BabelUpdateTlv>& update, BabelInterface *iface, const L3Address& src, const rid& originator, const L3Address& nexthop)
{
    int ae = update->getAe();
    uint16_t interval = update->getInterval();
    routeDistance dist(update->getSeqno(), update->getMetric());

    BabelNeighbour *neigh = bnt.findNeighbour(iface, src);
    if (neigh == nullptr)
        return false; // update from a node we are not adjacent to

    if (interval == 0xFFFF)
        interval = iface->getUInterval();

    // wildcard retraction: retract every route learned from this neighbour
    if (update->getMetric() == COST_INF && ae == AE::WILDCARD) {
        bool changed = false;
        for (auto route : btt.getRoutes())
            if (route->getNeighbour() == neigh)
                changed = addOrUpdateRoute(route->getPrefix(), neigh, route->getOriginator(),
                        routeDistance(route->getRDistance().getSeqno(), COST_INF), route->getNextHop(), interval) || changed;
        return changed;
    }

    if (ae != AE::IPv4 && ae != AE::IPv6)
        return false; // unsupported AE

    netPrefix<L3Address> prefix(update->getPrefix(), update->getPrefixLen());

    // did this update answer a pending Seqno Request? (RFC 6126, 3.8.2.3)
    if (BabelPenSR *pending = bpsrt.findPenSR(prefix, iface)) {
        if (pending->getReceivedFrom() != nullptr) {
            // relay the answer back toward the node that originally asked
            BabelInterface *backIface = pending->getReceivedFrom()->getInterface();
            int af = (prefix.getAddr().getType() == L3Address::IPv4) ? AF::IPv4 : AF::IPv6;
            L3Address backNh = interfaceAddressForAf(backIface, af);
            if (!backNh.isUnspecified()) {
                unsigned int m = std::min<unsigned int>(dist.getMetric() + neigh->getCost(), COST_INF);
                sendUpdateMessage(pending->getReceivedFrom()->getAddress(), backIface, originator, backNh,
                        prefix, routeDistance(dist.getSeqno(), m), backIface->getUInterval());
                bpsrt.removePenSR(pending);
            }
        }
        else if (comparemod16(dist.getSeqno(), pending->getReqSeqno()) != -1)
            bpsrt.removePenSR(prefix); // our own request is satisfied
    }

    BabelRoute *intable = btt.findRoute(prefix, neigh, originator);

    if (intable != nullptr) {
        if (intable->getSelected() && !isFeasible(prefix, originator, dist)) {
            // unfeasible update for a selected route
            if (intable->getOriginator() != originator)
                return addOrUpdateRoute(prefix, neigh, originator, routeDistance(dist.getSeqno(), COST_INF), nexthop, interval);
            // request a fresher seqno so this route can become feasible again (RFC 6126, 3.8.2.2)
            sendSeqnoReq(neigh->getInterface(), neigh->getAddress(), prefix, intable->getOriginator(),
                    plusmod16(intable->getRDistance().getSeqno(), 1), defval::SEQNUMREQ_HOPCOUNT, nullptr);
            return false;
        }
        if (isFeasible(prefix, originator, dist))
            return addOrUpdateRoute(prefix, neigh, originator, dist, nexthop, interval);
        return false;
    }

    // no such route yet
    if (!isFeasible(prefix, originator, dist) || dist.getMetric() == COST_INF)
        return false;
    return addOrUpdateRoute(prefix, neigh, originator, dist, nexthop, interval);
}

void Babel::processRouteReq(const Ptr<const BabelRouteReqTlv>& req, BabelInterface *iface, const L3Address& src, const L3Address& dst)
{
    int ae = req->getAe();
    if (ae == AE::WILDCARD) {
        sendFullDump(iface);
        return;
    }
    if (ae != AE::IPv4 && ae != AE::IPv6)
        return;

    netPrefix<L3Address> prefix(req->getPrefix(), req->getPrefixLen());
    BabelRoute *intable = btt.findSelectedRoute(prefix);
    const L3Address& replyDst = dst.isMulticast() ? (ae == AE::IPv4 ? defval::MCASTG4 : defval::MCASTG6) : src;

    if (intable != nullptr)
        sendUpdate(iface, intable, replyDst);
    else {
        // no route -> reply with a retraction so the requester stops waiting
        L3Address nh = interfaceAddressForAf(iface, AE::toAF(ae));
        if (!nh.isUnspecified())
            sendUpdateMessage(replyDst, iface, rid(), nh, prefix, routeDistance(0, COST_INF), iface->getUInterval());
    }
}

// ---- feasibility + route database ----

bool Babel::isFeasible(const netPrefix<L3Address>& prefix, const rid& orig, const routeDistance& dist)
{
    if (dist.getMetric() == COST_INF)
        return true; // a retraction is always feasible
    BabelSource *source = bst.findSource(prefix, orig);
    if (source == nullptr)
        return true; // no recorded feasibility distance -> feasible
    return dist < source->getFDistance();
}

bool Babel::addOrUpdateRoute(const netPrefix<L3Address>& prefix, BabelNeighbour *neigh, const rid& orig, const routeDistance& dist, const L3Address& nh, uint16_t interval)
{
    bool changed = false;
    bool resetExpiry = true;
    BabelRoute *route = btt.findRoute(prefix, neigh);

    if (route != nullptr) {
        if (route->getRDistance() != dist) {
            route->setRDistance(dist);
            changed = true;
        }
        else if (dist.getMetric() == COST_INF)
            resetExpiry = false; // retraction of an already-retracted route

        if (route->getNextHop() != nh) {
            route->setNextHop(nh);
            changed = true;
        }
        if (route->getOriginator() != orig) {
            route->setOriginator(orig);
            changed = true;
        }

        route->setUpdateInterval(interval);
        if (resetExpiry) {
            route->resetETimer();
            if (route->getRDistance().getMetric() != COST_INF)
                route->resetBETimer();
        }
    }
    else {
        BabelRoute *newroute = new BabelRoute(prefix, neigh, orig, dist, nh, interval,
                createTimer(timerT::ROUTEEXPIRY, nullptr), createTimer(timerT::ROUTEBEFEXPIRY, nullptr));
        newroute->resetETimer();
        newroute->resetBETimer();
        btt.addRoute(newroute);
        EV_INFO << "Babel: learned route " << *newroute << "." << endl;
        changed = true;
    }
    return changed;
}

void Babel::addOrUpdateSource(const netPrefix<L3Address>& p, const rid& orig, const routeDistance& dist)
{
    BabelSource *source = bst.findSource(p, orig);
    if (source != nullptr) {
        if (dist < source->getFDistance())
            source->setFDistance(dist);
        source->resetGCTimer();
    }
    else {
        BabelSource *newsource = new BabelSource(p, orig, dist, createTimer(timerT::SOURCEGC, nullptr));
        newsource->resetGCTimer();
        if (bst.addSource(newsource) != newsource)
            delete newsource;
    }
}

bool Babel::removeRoutesByNeigh(BabelNeighbour *neigh)
{
    bool changed = false;
    for (auto it = btt.getRoutes().begin(); it != btt.getRoutes().end();) {
        if ((*it)->getNeighbour() == neigh) {
            if ((*it)->getRTEntry())
                removeFromRT(*it);
            delete *it;
            it = btt.getRoutes().erase(it);
            changed = true;
        }
        else
            ++it;
    }
    return changed;
}

void Babel::selectRoutes()
{
    std::vector<BabelRoute *> toselect; // best candidate per prefix
    std::vector<BabelRoute *> tocancel; // previously selected/installed routes

    for (auto route : btt.getRoutes()) {
        if (route->getSelected() || route->getRTEntry())
            tocancel.push_back(route);
        if (route->getNeighbour() != nullptr) // keep local routes selected
            route->setSelected(false);

        BabelRoute *cand = nullptr;
        for (auto ts : toselect)
            if (ts->getPrefix() == route->getPrefix()) {
                cand = ts;
                break;
            }
        if (cand != nullptr) {
            if (route->metric() < cand->metric())
                std::replace(toselect.begin(), toselect.end(), cand, route);
        }
        else
            toselect.push_back(route);
    }

    auto findCandidate = [&](const netPrefix<L3Address>& p) -> BabelRoute * {
        for (auto ts : toselect)
            if (ts->getPrefix() == p)
                return ts;
        return nullptr;
    };

    // reconcile previously selected routes with the new candidates
    for (auto tc : tocancel) {
        BabelRoute *ts = findCandidate(tc->getPrefix());
        bool feasibleCand = ts != nullptr && ts->metric() != COST_INF &&
                (ts->getNeighbour() == nullptr || isFeasible(ts->getPrefix(), ts->getOriginator(), ts->getRDistance()));
        if (feasibleCand) {
            if (ts != tc)
                removeFromRT(tc);
        }
        else {
            // the previously-selected route is gone and no feasible replacement exists:
            // ask for a fresher seqno so an unfeasible alternative can be used (RFC 6126, 3.8.2.1)
            sendSeqnoReq(nullptr, L3Address(), tc->getPrefix(), tc->getOriginator(),
                    plusmod16(tc->getRDistance().getSeqno(), 1), defval::SEQNUMREQ_HOPCOUNT, nullptr);
            if (!btt.containShorterCovRoute(tc->getPrefix()))
                removeFromRT(tc);
        }
    }

    // install the chosen routes
    for (auto ts : toselect) {
        bool feasible = (ts->getNeighbour() == nullptr || isFeasible(ts->getPrefix(), ts->getOriginator(), ts->getRDistance()));
        if (feasible && ts->metric() != COST_INF) {
            ts->setSelected(true);
            if (ts->getNeighbour() != nullptr) {
                if (ts->getRTEntry() == nullptr)
                    addToRT(ts);
                else
                    updateRT(ts);
            }
        }
    }
}

// ---- routing-table install ----

bool Babel::prepareToAdd(IRoutingTable *rt, IRoute *ro)
{
    IRoute *old = nullptr;
    for (int i = 0; i < rt->getNumRoutes(); i++) {
        IRoute *tmp = rt->getRoute(i);
        if (tmp->getDestinationAsGeneric() == ro->getDestinationAsGeneric() && tmp->getPrefixLength() == ro->getPrefixLength()) {
            old = tmp;
            break;
        }
    }
    if (old != nullptr) {
        bool isV4 = (rt == rt4.getNullable());
        unsigned int newad = isV4 ? check_and_cast<Ipv4Route *>(ro)->getAdminDist() : check_and_cast<Ipv6Route *>(ro)->getAdminDist();
        unsigned int oldad = isV4 ? check_and_cast<Ipv4Route *>(old)->getAdminDist() : check_and_cast<Ipv6Route *>(old)->getAdminDist();
        if (oldad < newad)
            return false;
        if (oldad == newad && old->getMetric() < ro->getMetric())
            return false;
        rt->deleteRoute(old);
    }
    return true;
}

void Babel::addToRT(BabelRoute *route)
{
    if (route->getNeighbour() == nullptr)
        return; // local route -> already present as a connected route

    bool isV4 = route->getPrefix().getAddr().getType() == L3Address::IPv4;
    IRoutingTable *rt = isV4 ? rt4.getNullable() : rt6.getNullable();
    if (rt == nullptr)
        return;

    IRoute *ro = rt->createRoute();
    ro->setSourceType(IRoute::BABEL);
    ro->setSource(this);
    ro->setDestination(route->getPrefix().getAddr());
    ro->setPrefixLength(route->getPrefix().getLen());
    ro->setInterface(route->getNeighbour()->getInterface()->getInterface());
    ro->setNextHop(route->getNextHop());
    ro->setMetric(route->metric());
    if (isV4)
        check_and_cast<Ipv4Route *>(ro)->setAdminDist(Ipv4Route::dBABEL);
    else
        check_and_cast<Ipv6Route *>(ro)->setAdminDist(Ipv6Route::dBABEL);

    if (prepareToAdd(rt, ro)) {
        rt->addRoute(ro);
        route->setRTEntry(ro);
        EV_INFO << "Babel: installed route to " << route->getPrefix() << " via " << route->getNextHop() << "." << endl;
    }
    else
        delete ro;
}

void Babel::removeFromRT(BabelRoute *route)
{
    if (route == nullptr) {
        for (auto r : btt.getRoutes())
            if (r->getRTEntry())
                removeFromRT(r);
        return;
    }
    if (route->getRTEntry() == nullptr)
        return;

    bool isV4 = route->getPrefix().getAddr().getType() == L3Address::IPv4;
    IRoutingTable *rt = isV4 ? rt4.getNullable() : rt6.getNullable();
    if (rt != nullptr)
        rt->deleteRoute(route->getRTEntry());
    route->setRTEntry(nullptr);
}

void Babel::updateRT(BabelRoute *route)
{
    if (route == nullptr || route->getRTEntry() == nullptr)
        return;
    IRoute *entry = route->getRTEntry();
    entry->setMetric(route->metric());
    entry->setNextHop(route->getNextHop());
}

void Babel::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == interfaceStateChangedSignal) {
        const auto *change = check_and_cast<const NetworkInterfaceChangeDetails *>(obj);
        NetworkInterface *ie = change->getNetworkInterface();
        BabelInterface *biface = bit.findInterfaceById(ie->getInterfaceId());
        if (biface != nullptr) {
            if (ie->isUp() && !biface->getEnabled())
                activateInterface(biface);
            else if (!ie->isUp() && biface->getEnabled())
                deactivateInterface(biface);
        }
    }
    else if (signalID == ipv6AddressAssignedSignal) {
        // a global address became valid -> (re)originate the connected subnets at once
        originateConnectedRoutes();
        selectRoutes();
        triggerUpdate();
    }
    else if (signalID == routeDeletedSignal) {
        // the routing table removed a route (possibly one of ours, e.g. its interface
        // went down) -> drop our now-dangling reference to it
        const IRoute *deleted = check_and_cast<const IRoute *>(obj);
        for (auto route : btt.getRoutes())
            if (route->getRTEntry() == deleted)
                route->setRTEntry(nullptr);
    }
}

} // namespace babel
} // namespace inet
