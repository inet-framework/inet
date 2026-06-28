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
        host->subscribe(interfaceStateChangedSignal, this);
        host->subscribe(ipv6AddressAssignedSignal, this);
        WATCH(routerId);
        WATCH(seqno);
        WATCH(udpPort);
        WATCH_PTRVECTOR(bit.getInterfaces());
        WATCH_PTRVECTOR(bnt.getNeighbours());
        WATCH_PTRVECTOR(btt.getRoutes());
        WATCH_PTRVECTOR(bst.getSources());
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
    // P3 default: run Babel (IPv6) on every multicast, non-loopback interface
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        NetworkInterface *ie = ift->getInterface(i);
        if (ie->isMulticast() && !ie->isLoopback()) {
            BabelInterface *biface = new BabelInterface(ie, seqno);
            biface->setAfSend(AF::IPv6);
            biface->setAfDist(AF::IPv6);
            biface->setWired(!ie->isWireless());
            biface->setNominalRxcost(ie->isWireless() ? defval::NOM_RXCOST_WIRELESS : defval::NOM_RXCOST_WIRED);
            biface->setCostComputationModule(&wiredCost);
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

    EV_INFO << "Babel started on UDP port " << udpPort << "." << endl;
}

void Babel::handleStopOperation(LifecycleOperation *operation)
{
    for (auto biface : bit.getInterfaces())
        deactivateInterface(biface);
    socket.close();
}

void Babel::handleCrashOperation(LifecycleOperation *operation)
{
    socket.destroy();
}

void Babel::activateInterface(BabelInterface *iface)
{
    ASSERT(iface != nullptr);
    if (!iface->getInterface()->isUp())
        return;

    socket.joinMulticastGroup(defval::MCASTG6, iface->getInterfaceId());

    if (iface->getAfSend() != AF::NONE) {
        if (iface->getHTimer() == nullptr)
            iface->setHTimer(createTimer(timerT::HELLO, iface));
        iface->resetHTimer(uniform(SEND_URGENT, CStoS(iface->getHInterval()))); // randomized first Hello

        if (iface->getUTimer() == nullptr)
            iface->setUTimer(createTimer(timerT::UPDATE, iface));
        iface->resetUTimer(uniform(SEND_URGENT, CStoS(iface->getUInterval()))); // randomized first Update
    }

    iface->setEnabled(true);

    // say hello and request a full route dump
    if (iface->getAfSend() != AF::NONE) {
        std::vector<Ptr<BabelTlv>> tlvs;
        auto hello = makeShared<BabelHelloTlv>();
        hello->setSeqno(iface->getIncHSeqno());
        hello->setInterval(iface->getHInterval());
        tlvs.push_back(hello);
        sendBabelMessage(defval::MCASTG6, iface, tlvs);

        sendRouteReq(iface, defval::MCASTG6, AE::WILDCARD, netPrefix<L3Address>());
    }

    EV_INFO << "Babel: interface " << iface->getIfaceName() << " activated." << endl;
}

void Babel::deactivateInterface(BabelInterface *iface)
{
    ASSERT(iface != nullptr);
    iface->deleteHTimer();
    iface->deleteUTimer();

    // drop routes learned via this interface before deleting their neighbours
    bool changed = false;
    for (auto neigh : bnt.getNeighbours())
        if (neigh->getInterface() == iface)
            changed = removeRoutesByNeigh(neigh) || changed;
    bnt.removeNeighboursOnIface(iface);

    iface->setEnabled(false);
    if (changed)
        selectRoutes();
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
        sendUpdate(nullptr, route); // multicast retraction on all interfaces
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

    std::vector<Ptr<BabelTlv>> tlvs;
    auto hello = makeShared<BabelHelloTlv>();
    hello->setSeqno(hseqno);
    hello->setInterval(iface->getHInterval());
    tlvs.push_back(hello);

    // append an IHU per neighbour every IHU_INTERVAL_MULT Hellos, or sooner on a lossy link
    bool periodicIhu = (hseqno % defval::IHU_INTERVAL_MULT == 0);
    for (auto neigh : bnt.getNeighbours()) {
        if (neigh->getInterface() != iface)
            continue;
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

    sendBabelMessage(defval::MCASTG6, iface, tlvs);
    iface->resetHTimer();
}

void Babel::sendBabelMessage(const L3Address& dst, BabelInterface *iface, const std::vector<Ptr<BabelTlv>>& tlvs)
{
    if (!iface->getEnabled() || !iface->getInterface()->isUp() || iface->getAfSend() == AF::NONE || tlvs.empty())
        return;

    B bodyLength = B(0);
    for (auto& tlv : tlvs) {
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
            default:
                // Pad1/PadN/SeqnoReq: SeqnoReq added in P5
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
    }

    // IPv4: connected routes are stored in the routing table
    if (IRoutingTable *rt = rt4.getNullable()) {
        for (int i = 0; i < rt->getNumRoutes(); i++) {
            IRoute *r = rt->getRoute(i);
            if (!r->getNextHopAsGeneric().isUnspecified())
                continue;
            NetworkInterface *ie = r->getInterface();
            if (ie == nullptr || bit.findInterfaceById(ie->getInterfaceId()) == nullptr)
                continue;
            if (r->getPrefixLength() <= 0 || r->getPrefixLength() >= 32)
                continue;
            originate(netPrefix<L3Address>(r->getDestinationAsGeneric(), r->getPrefixLength()));
        }
    }
}

// ---- update send ----

void Babel::sendUpdateMessage(const L3Address& dst, BabelInterface *iface, const rid& originator,
        const L3Address& nexthop, const netPrefix<L3Address>& prefix, const routeDistance& dist, uint16_t interval)
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

    sendBabelMessage(dst, iface, tlvs);
}

void Babel::sendUpdate(BabelInterface *iface, BabelRoute *route, const L3Address& dst)
{
    // split horizon: don't echo a route back to the neighbour it was learned from
    if (route->getNeighbour() != nullptr && iface->getSplitHorizon() && route->getNeighbour()->getInterface() == iface)
        return;

    int af = (route->getPrefix().getAddr().getType() == L3Address::IPv4) ? AF::IPv4 : AF::IPv6;
    L3Address nh = interfaceAddressForAf(iface, af);
    if (nh.isUnspecified())
        return; // the interface has no address of this family

    sendUpdateMessage(dst, iface, route->getOriginator(), nh, route->getPrefix(),
            routeDistance(route->getRDistance().getSeqno(), route->metric()), iface->getUInterval());
}

void Babel::sendUpdate(BabelInterface *iface, BabelRoute *route)
{
    if (iface == nullptr) {
        for (auto bi : bit.getInterfaces())
            if (bi->getEnabled() && bi->getAfSend() != AF::NONE)
                sendUpdate(bi, route);
        return;
    }
    bool v4prefix = route->getPrefix().getAddr().getType() == L3Address::IPv4;
    sendUpdate(iface, route, v4prefix ? defval::MCASTG4 : defval::MCASTG6);
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
    sendBabelMessage(dst, iface, tlvs);
}

void Babel::triggerUpdate()
{
    // damp a burst of changes into a single update shortly afterwards (RFC 6126, 3.7.1)
    if (triggeredUpdate != nullptr && !triggeredUpdate->isScheduled())
        scheduleAfter(SEND_URGENT, triggeredUpdate);
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
    BabelRoute *intable = btt.findRoute(prefix, neigh, originator);

    if (intable != nullptr) {
        if (intable->getSelected() && !isFeasible(prefix, originator, dist)) {
            // unfeasible update for a selected route
            if (intable->getOriginator() != originator)
                return addOrUpdateRoute(prefix, neigh, originator, routeDistance(dist.getSeqno(), COST_INF), nexthop, interval);
            return false; // (a Seqno Request would speed recovery here -- added in P5)
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
        else if (!btt.containShorterCovRoute(tc->getPrefix()))
            removeFromRT(tc); // (a Seqno Request would speed recovery here -- added in P5)
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
}

} // namespace babel
} // namespace inet
