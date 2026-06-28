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

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/routing/babel/BabelSerializer.h"

namespace inet {
namespace babel {

Define_Module(Babel);

Babel::~Babel()
{
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
        udpPort = par("udpPort");
        seqno = intuniform(0, UINT16_MAX);
        socket.setOutputGate(gate("socketOut"));
        host->subscribe(interfaceStateChangedSignal, this);
        WATCH(routerId);
        WATCH(seqno);
        WATCH(udpPort);
        WATCH_PTRVECTOR(bit.getInterfaces());
        WATCH_PTRVECTOR(bnt.getNeighbours());
    }
}

void Babel::finish()
{
    EV_INFO << "Babel on " << host->getFullName() << " (router-id " << routerId << "):" << endl;
    for (auto neigh : bnt.getNeighbours())
        EV_INFO << "  neighbour " << neigh->str() << endl;
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
    }

    iface->setEnabled(true);

    // say hello to others right away
    if (iface->getAfSend() != AF::NONE) {
        std::vector<Ptr<BabelTlv>> tlvs;
        auto hello = makeShared<BabelHelloTlv>();
        hello->setSeqno(iface->getIncHSeqno());
        hello->setInterval(iface->getHInterval());
        tlvs.push_back(hello);
        sendBabelMessage(defval::MCASTG6, iface, tlvs);
    }

    EV_INFO << "Babel: interface " << iface->getIfaceName() << " activated." << endl;
}

void Babel::deactivateInterface(BabelInterface *iface)
{
    ASSERT(iface != nullptr);
    iface->deleteHTimer();
    iface->deleteUTimer();
    bnt.removeNeighboursOnIface(iface);
    iface->setEnabled(false);
}

cMessage *Babel::createTimer(short kind, void *context)
{
    cMessage *timer = new cMessage(timerT::toStr(kind).c_str(), kind);
    timer->setContextPointer(context);
    return timer;
}

void Babel::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
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
        case timerT::NEIGHHELLO:
            processNeighHelloTimer(check_and_cast<BabelNeighbour *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        case timerT::NEIGHIHU:
            processNeighIhuTimer(check_and_cast<BabelNeighbour *>(static_cast<cObject *>(timer->getContextPointer())));
            break;
        default:
            throw cRuntimeError("Babel: received timer of unknown type: %d", timer->getKind());
    }
}

void Babel::processNeighHelloTimer(BabelNeighbour *neigh)
{
    neigh->noteLoss();
    neigh->setExpHSeqno(plusmod16(neigh->getExpHSeqno(), 1));

    if (neigh->getHistory() == 0) {
        EV_INFO << "Babel: neighbour " << neigh->getAddress() << " on " << neigh->getInterface()->getIfaceName() << " lost." << endl;
        bnt.removeNeighbour(neigh); // route cleanup added in a later phase
    }
    else {
        neigh->resetNHTimer();
        neigh->recomputeCost();
    }
}

void Babel::processNeighIhuTimer(BabelNeighbour *neigh)
{
    neigh->setTxcost(COST_INF);
    neigh->recomputeCost();
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

    while (packet->getDataLength() > b(0)) {
        auto tlv = packet->popAtFront<BabelTlv>();
        switch (tlv->getTlvType()) {
            case BABEL_HELLO:
                processHello(staticPtrCast<const BabelHelloTlv>(tlv), iface, src);
                break;
            case BABEL_IHU:
                processIhu(staticPtrCast<const BabelIhuTlv>(tlv), iface, src, dst);
                break;
            default:
                // Pad1/PadN/RouterId/NextHop/Update/RouteReq/SeqnoReq: added in later phases
                break;
        }
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

    neigh->recomputeCost();
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
}

} // namespace babel
} // namespace inet
