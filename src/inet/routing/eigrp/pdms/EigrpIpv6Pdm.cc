//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @file EigrpIpv6Pdm.cc
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 * @date 6. 11. 2014
 * @brief EIGRP IPv6 Protocol Dependent Module
 * @detail Main module, it mediates control exchange between DUAL, routing table and
   topology table.
 */

#include "inet/routing/eigrp/pdms/EigrpIpv6Pdm.h"

#include <algorithm>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/routing/eigrp/EigrpDeviceConfigurator.h"
#include "inet/routing/eigrp/pdms/EigrpPrint.h"
#define EIGRP_DEBUG

namespace inet {
namespace eigrp {

Define_Module(EigrpIpv6Pdm);

EigrpIpv6Pdm::EigrpIpv6Pdm() : EIGRP_IPV6_MULT(Ipv6Address("FF02::A"))
{
    SPLITTER_OUTGW = "splitterOut";
    RTP_OUTGW = "rtpOut";

    KVALUES_MAX.K1 = KVALUES_MAX.K2 = KVALUES_MAX.K3 = KVALUES_MAX.K4 = KVALUES_MAX.K5 = KVALUES_MAX.K6 = 255;

    asNum = -1;
    maximumPath = 4;
    variance = 1;
    adminDistInt = 90;
    useClassicMetric = true;
    ribScale = 128;

    kValues.K1 = kValues.K3 = 1;
    kValues.K2 = kValues.K4 = kValues.K5 = kValues.K6 = 0;

    eigrpStubEnabled = false;
}

EigrpIpv6Pdm::~EigrpIpv6Pdm()
{
    delete this->routingForNetworks;
    delete this->eigrpIftDisabled;
    delete this->eigrpDual;
    delete this->eigrpMetric;
}

void EigrpIpv6Pdm::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        host = getContainingNode(this);

        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);
    }

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        eigrpIft = check_and_cast<EigrpInterfaceTable *>(getModuleByPath("^.eigrpInterfaceTable6"));
        eigrpNt = check_and_cast<EigrpIpv6NeighborTable *>(getModuleByPath("^.eigrpIpv6NeighborTable"));
        eigrpTt = check_and_cast<EigrpIpv6TopologyTable *>(getModuleByPath("^.eigrpIpv6TopologyTable"));

        this->eigrpIftDisabled = new EigrpDisabledInterfaces();
        this->routingForNetworks = new EigrpNetworkTable<Ipv6Address>();
        this->eigrpDual = new EigrpDual<Ipv6Address>(this);
        this->eigrpMetric = new EigrpMetricHelper();

        EigrpDeviceConfigurator conf(par("configData"), ift);
        conf.loadEigrpIPv6Config(this);

        // moved to splitter
//        registerService(Protocol::eigrp, nullptr, gate("splitterIn"));
//        registerProtocol(Protocol::eigrp, gate("splitterOut"), nullptr);

        WATCH_PTRVECTOR(*routingForNetworks->getAllNetworks());
        WATCH(asNum);
        WATCH(kValues.K1);
        WATCH(kValues.K2);
        WATCH(kValues.K3);
        WATCH(kValues.K4);
        WATCH(kValues.K5);
        WATCH(kValues.K6);
        WATCH(maximumPath);
        WATCH(variance);
        WATCH(eigrpStub.connectedRt);
        WATCH(eigrpStub.leakMapRt);
        WATCH(eigrpStub.recvOnlyRt);
        WATCH(eigrpStub.redistributedRt);
        WATCH(eigrpStub.staticRt);
        WATCH(eigrpStub.summaryRt);

        host->subscribe(interfaceStateChangedSignal, this);
        host->subscribe(interfaceConfigChangedSignal, this);
        host->subscribe(routeDeletedSignal, this);
    }
}

void EigrpIpv6Pdm::preDelete(cComponent *root)
{
    cancelHelloTimers();
}

//void EigrpIpv6Pdm::receiveChangeNotification(int category, const cObject *details)
void EigrpIpv6Pdm::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == interfaceStateChangedSignal) {
        NetworkInterfaceChangeDetails *ifcecd = check_and_cast<NetworkInterfaceChangeDetails *>(obj);
        processIfaceStateChange(ifcecd->getNetworkInterface());
    }
    else if (signalID == interfaceConfigChangedSignal) {
        NetworkInterfaceChangeDetails *ifcecd = check_and_cast<NetworkInterfaceChangeDetails *>(obj);
        NetworkInterface *iface = ifcecd->getNetworkInterface();
        EigrpInterface *eigrpIface = getInterfaceById(iface->getInterfaceId());
        double ifParam;

        if (eigrpIface == nullptr)
            return;

        ifParam = iface->getDatarate();
        if (ifParam != 0 && ifParam != eigrpIface->getInterfaceDatarate()) { // Bandwidth
            eigrpIface->setInterfaceDatarate(ifParam);
            if (eigrpIface->isEnabled())
                processIfaceConfigChange(eigrpIface);
        }
    }
    else if (signalID == routeDeletedSignal) {
        processRTRouteDel(obj);
    }
}

void EigrpIpv6Pdm::processIfaceStateChange(NetworkInterface *iface)
{
    EigrpInterface *eigrpIface;
    int ifaceId = iface->getInterfaceId();

    if (iface->isUp()) { // an interface goes up

        // add directly-connected routes to RT
        PrefixVector::iterator it;

        for (it = netPrefixes.begin(); it != netPrefixes.end(); ++it) { // through all known prefixes search prefixes belong to this interface
            if (it->ifaceId == ifaceId) { // belonging to same interface -> add
                if (findRoute(it->network, it->prefixLength, Ipv6Address::UNSPECIFIED_ADDRESS) == nullptr) { // route is not in RT -> add
                    rt->addStaticRoute(it->network, it->prefixLength, ifaceId, Ipv6Address::UNSPECIFIED_ADDRESS, 0);
                }
            }
        }

        if ((eigrpIface = getInterfaceById(ifaceId)) != nullptr) { // interface is included in EIGRP process
            if (!eigrpIface->isEnabled()) { // interface disabled -> enable
                enableInterface(eigrpIface);
                startHelloTimer(eigrpIface, simTime() + eigrpIface->getHelloInt() - 0.5);
            }
        }
    }
    else if (!iface->isUp() || !iface->hasCarrier()) { // an interface goes down

        // delete all directly-connected routes from RT
        Ipv6Route *route = nullptr;
        for (int i = 0; i < rt->getNumRoutes(); ++i) {
            route = rt->getRoute(i);

            if (route->getInterface()->getInterfaceId() == ifaceId && route->getNextHop() == Ipv6Address::UNSPECIFIED_ADDRESS && route->getDestPrefix() != Ipv6Address::LINKLOCAL_PREFIX) { // Found Directly-connected (no link-local) route on interface -> remove
                rt->removeRoute(route);
            }
        }

        eigrpIface = this->eigrpIft->findInterfaceById(ifaceId);
        if (eigrpIface != nullptr && eigrpIface->isEnabled()) {
            disableInterface(iface, eigrpIface);
        }
    }
}

void EigrpIpv6Pdm::processIfaceConfigChange(EigrpInterface *eigrpIface)
{
    EigrpRouteSource<Ipv6Address> *source;
    int routeCount = eigrpTt->getNumRoutes();
    int ifaceId = eigrpIface->getInterfaceId();
    EigrpWideMetricPar ifParam = eigrpMetric->getParam(eigrpIface);
    EigrpWideMetricPar newParam;
    uint64_t metric;

    // Update routes through the interface
    for (int i = 0; i < routeCount; i++) {
        source = eigrpTt->getRoute(i);
        if (source->getIfaceId() == ifaceId && source->isValid()) {
            // Update metric of source
            if (source->getNexthopId() == EigrpNeighbor<Ipv6Address>::UNSPEC_ID) { // connected route
                source->setMetricParams(ifParam);
                metric = eigrpMetric->computeClassicMetric(ifParam, this->kValues);
                source->setMetric(metric);
            }
            else {
                newParam = eigrpMetric->adjustParam(ifParam, source->getRdParams());
                source->setMetricParams(newParam);
                metric = eigrpMetric->computeClassicMetric(newParam, this->kValues);
                source->setMetric(metric);
            }

            // Notify DUAL about event
            eigrpDual->processEvent(EigrpDual<Ipv6Address>::RECV_UPDATE, source, source->getNexthopId(), false);
        }
    }

    flushMsgRequests();
    eigrpTt->purgeTable();
}

void EigrpIpv6Pdm::processRTRouteDel(const cObject *details)
{
    if (dynamic_cast<const Ipv6Route *>(details) == nullptr) {
        return; // route is not Ipv6
    }
    const Ipv6Route *changedRt = check_and_cast<const Ipv6Route *>(details);
//    ANSAIPv6Route *changedAnsaRt = dynamic_cast<ANSAIPv6Route *>(changedRt);
    const Ipv6Route *changedAnsaRt = changedRt;
    unsigned adminDist;
    EigrpRouteSource<Ipv6Address> *source = nullptr;

    if (changedAnsaRt != nullptr)
        adminDist = changedAnsaRt->getAdminDist();
    else
        adminDist = changedRt->getAdminDist();

#ifdef EIGRP_DEBUG
//    EV_DEBUG << "EIGRP: received notification about deletion of route in RT with AD=" << adminDist << endl;
#endif

    if (adminDist == this->adminDistInt) { // Deletion of EIGRP internal route
        source = eigrpTt->findRoute(changedRt->getDestPrefix(), makeNetmask(changedRt->getPrefixLength()), changedRt->getNextHop());
        if (source == nullptr) {
            ASSERT(false);
            EV_DEBUG << "EIGRP: removed EIGRP route from RT, not found corresponding route source in TT" << endl;
            return;
        }

        if (source == oldsource)
            return;
        else
            oldsource = source;

        if (source->isSuccessor()) {
            if (!source->getRouteInfo()->isActive()) { // Process route in DUAL (no change of metric)
                this->eigrpDual->processEvent(EigrpDual<Ipv6Address>::LOST_ROUTE, source, IEigrpPdm::UNSPEC_SENDER, false);
            }
        }
        // Else do nothing - EIGRP itself removed route from RT
    }
    else { // Deletion of non EIGRP route

    }
}

void EigrpIpv6Pdm::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) { // Timer
        this->processTimer(msg);
    }
    else { // EIGRP message

        if (strcmp(msg->getArrivalGate()->getBaseName(), "splitterIn") == 0) {
            processMsgFromNetwork(msg);
            delete msg;
            msg = nullptr;
        }
        else if (strcmp(msg->getArrivalGate()->getBaseName(), "rtpIn") == 0) {
            processMsgFromRtp(msg);
            delete msg;
            msg = nullptr;
        }
    }
}

void EigrpIpv6Pdm::processMsgFromNetwork(cMessage *msg)
{
    Packet *packet = check_and_cast<Packet *>(msg);

    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol != &Protocol::eigrp) {
        delete msg;
        return;
    }

    Ipv6Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv6();
    int ifaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    cMessage *msgDup = nullptr;

    NetworkInterface::State status = ift->getInterfaceById(ifaceId)->getState();
    if (status == NetworkInterface::DOWN || status == NetworkInterface::GOING_UP) { // message received on DOWN or GOING_UP iface -> ignore
        EV_DEBUG << "Received message on DOWN interface - message ignored" << endl;
        return;
    }

    const auto& pk = packet->peekAtFront<EigrpMessage>();
    int8_t opcode = pk->getOpcode();

#ifdef EIGRP_DEBUG
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    ASSERT(eigrpIface != nullptr);
    ASSERT(!eigrpIface->isPassive());
#endif

    // Find neighbor if exists
    EigrpNeighbor<Ipv6Address> *neigh;
    if ((neigh = eigrpNt->findNeighbor(srcAddr)) != nullptr) { // Reset hold timer
        resetTimer(neigh->getHoldTimer(), neigh->getHoldInt());
    }

    // Send message to RTP (message must be duplicated)
    msgDup = msg->dup();
    send(msgDup, RTP_OUTGW);

    switch (opcode) {
        case EIGRP_HELLO_MSG:
            if (pk->getAckNum() > 0)
                processAckPacket(packet, srcAddr, ifaceId, neigh);
            else
                processHelloPacket(packet, srcAddr, ifaceId, neigh);
            break;
        case EIGRP_UPDATE_MSG:
            processUpdatePacket(packet, srcAddr, ifaceId, neigh);
            break;
        case EIGRP_QUERY_MSG:
            processQueryPacket(packet, srcAddr, ifaceId, neigh);
            break;
        case EIGRP_REPLY_MSG:
            processReplyPacket(packet, srcAddr, ifaceId, neigh);
            break;
        default:
            if (neigh != nullptr)
                EV_DEBUG << "EIGRP: Received message of unknown type, skipped" << endl;
            else
                EV_DEBUG << "EIGRP: Received message from " << srcAddr << " that is not neighbor, skipped" << endl;
            break;
    }
}

void EigrpIpv6Pdm::processMsgFromRtp(cMessage *msg)
{
    EigrpMsgReq *msgReq = check_and_cast<EigrpMsgReq *>(msg);
    int routeCnt = msgReq->getRoutesArraySize();
    Ipv6Address destAddress;
    int destIface = msgReq->getDestInterface();
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(destIface);

    Packet *pk;

#ifdef EIGRP_DEBUG
    ASSERT(eigrpIface != nullptr && !eigrpIface->isPassive());
#endif

    if (!getDestIpAddress(msgReq->getDestNeighbor(), &destAddress) || eigrpIface == nullptr) { // Discard message request
        return;
    }

    // Print message information
    printSentMsg(routeCnt, destAddress, msgReq);

    // Create EIGRP message
    switch (msgReq->getOpcode()) {
        case EIGRP_HELLO_MSG:
            if (msgReq->getGoodbyeMsg() == true) { // Goodbye message
                pk = createHelloPacket(eigrpIface->getHoldInt(), this->KVALUES_MAX, destAddress, msgReq);
                pk->setName("EIGRP_HELLO_MSG");
            }
            else if (msgReq->getAckNumber() > 0) { // Ack message
                pk = createAckPacket(destAddress, msgReq);
                pk->setName("EIGRP_ACK_MSG");
            }
            else { // Hello message
                pk = createHelloPacket(eigrpIface->getHoldInt(), this->kValues, destAddress, msgReq);
                pk->setName("EIGRP_HELLO_MSG");
            }
            break;

        case EIGRP_UPDATE_MSG:
            pk = createUpdatePacket(destAddress, msgReq);
            pk->setName("EIGRP_UPDATE_MSG");
            break;

        case EIGRP_QUERY_MSG:

            pk = createQueryPacket(destAddress, msgReq);
            pk->setName("EIGRP_QUERY_MSG");
            break;

        case EIGRP_REPLY_MSG:
            pk = createReplyPacket(destAddress, msgReq);
            pk->setName("EIGRP_REPLY_MSG");
            break;

        default:
            ASSERT(false);
            return;
            break;
    }

    // Send message to network
    pk->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::eigrp);
    pk->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destIface);
    pk->addTagIfAbsent<L3AddressReq>()->setDestAddress(destAddress);
    pk->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
    pk->addTagIfAbsent<L3AddressReq>()->setSrcAddress(ift->getInterfaceById(destIface)->getProtocolData<Ipv6InterfaceData>()->getLinkLocalAddress());
    pk->addTagIfAbsent<HopLimitReq>()->setHopLimit(1);
    send(pk, SPLITTER_OUTGW);
}

bool EigrpIpv6Pdm::getDestIpAddress(int destNeigh, Ipv6Address *resultAddress)
{
    EigrpNeighbor<Ipv6Address> *neigh = nullptr;
    const uint32_t *addr = nullptr;

    if (destNeigh == EigrpNeighbor<Ipv6Address>::UNSPEC_ID) { // destination neighbor unset -> use multicast
        addr = EIGRP_IPV6_MULT.words();
        resultAddress->set(addr[0], addr[1], addr[2], addr[3]);
    }
    else { // destination neighbor set -> use unicast
        if ((neigh = eigrpNt->findNeighborById(destNeigh)) == nullptr)
            return false;
        auto ipAddress = neigh->getIPAddress();
        addr = ipAddress.words();
        resultAddress->set(addr[0], addr[1], addr[2], addr[3]);
    }

    return true;
}

void EigrpIpv6Pdm::printSentMsg(int routeCnt, Ipv6Address& destAddress, EigrpMsgReq *msgReq)
{
    int type = msgReq->getOpcode();

    EV_DEBUG << "EIGRP: send " << eigrp::UserMsgs[type];
    if (type == EIGRP_HELLO_MSG) {
        if (msgReq->getAckNumber() > 0)
            EV_DEBUG << " (ack) ";
        else if (msgReq->getGoodbyeMsg() == true)
            EV_DEBUG << " (goodbye) ";
    }

    EV_DEBUG << " message to " << destAddress << " on IF " << msgReq->getDestInterface();

    // Print flags
    EV_DEBUG << ", flags: ";
    if (msgReq->getInit()) EV_DEBUG << "init";
    else if (msgReq->getEot()) EV_DEBUG << "eot";
    else if (msgReq->getCr()) EV_DEBUG << "cr";
    else if (msgReq->getRs()) EV_DEBUG << "rs";

    if (type != EIGRP_HELLO_MSG) EV_DEBUG << ", route count: " << routeCnt;
    EV_DEBUG << ", seq num:" << msgReq->getSeqNumber();
    EV_DEBUG << ", ack num:" << msgReq->getAckNumber();
    EV_DEBUG << endl;
}

void EigrpIpv6Pdm::printRecvMsg(const EigrpMessage *msg, Ipv6Address& addr, int ifaceId)
{
    EV_DEBUG << "EIGRP: received " << eigrp::UserMsgs[msg->getOpcode()];
    if (msg->getOpcode() == EIGRP_HELLO_MSG && msg->getAckNum() > 0)
        EV_DEBUG << " (ack) ";
    EV_DEBUG << " message from " << addr << " on IF " << ifaceId;

    EV_DEBUG << ", flags: ";
    if (msg->getInit()) EV_DEBUG << "init";
    else if (msg->getEot()) EV_DEBUG << "eot";
    else if (msg->getCr()) EV_DEBUG << "cr";
    else if (msg->getRs()) EV_DEBUG << "rs";

    EV_DEBUG << ", seq num:" << msg->getSeqNum();
    EV_DEBUG << ", ack num:" << msg->getAckNum();
    EV_DEBUG << endl;
}

void EigrpIpv6Pdm::processTimer(cMessage *msg)
{
    EigrpTimer *timer = check_and_cast<EigrpTimer *>(msg);
    EigrpInterface *eigrpIface = nullptr;
    EigrpNeighbor<Ipv6Address> *neigh = nullptr;
    cObject *contextBasePtr = nullptr;
    EigrpMsgReq *msgReq = nullptr;
    int ifaceId = -1;

    switch (timer->getTimerKind()) {
        case EIGRP_HELLO_TIMER:
            // get interface that belongs to timer
            contextBasePtr = (cObject *)timer->getContextPointer();
            eigrpIface = check_and_cast<EigrpInterface *>(contextBasePtr);

            // schedule Hello timer
            scheduleAt(simTime() + eigrpIface->getHelloInt() - 0.5, timer);

            // send Hello message
            msgReq = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<Ipv6Address>::UNSPEC_ID, eigrpIface->getInterfaceId());
            send(msgReq, RTP_OUTGW);
            break;

        case EIGRP_HOLD_TIMER:
            // get neighbor from context
            contextBasePtr = (cObject *)timer->getContextPointer();
            neigh = check_and_cast<EigrpNeighbor<Ipv6Address> *>(contextBasePtr);
            ifaceId = neigh->getIfaceId();

            // remove neighbor
            EV_DEBUG << "Neighbor " << neigh->getIPAddress() << " is down, holding time expired" << endl;
            removeNeighbor(neigh);
            neigh = nullptr;
            flushMsgRequests();
            eigrpTt->purgeTable();

            // Send goodbye and restart Hello timer
            eigrpIface = eigrpIft->findInterfaceById(ifaceId);
            resetTimer(eigrpIface->getHelloTimer(), eigrpIface->getHelloInt() - 0.5);
            msgReq = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<Ipv6Address>::UNSPEC_ID, ifaceId);
            msgReq->setGoodbyeMsg(true);
            send(msgReq, RTP_OUTGW);
            break;

        default:
            EV_DEBUG << "Timer with unknown kind was skipped" << endl;
            delete timer;
            timer = nullptr;
            break;
    }
}

void EigrpIpv6Pdm::processAckPacket(Packet *pk, Ipv6Address& srcAddress, int ifaceId, EigrpNeighbor<Ipv6Address> *neigh)
{
    const EigrpIpv6Ack *ack = staticPtrCast<const EigrpIpv6Ack>(pk->peekAtFront<EigrpIpv6Ack>()).get();
    printRecvMsg(ack, srcAddress, ifaceId);
    if (neigh->isStateUp() == false) {
        // If neighbor is "pending", then change its state to "up"
        neigh->setStateUp(true);
        // Send all EIGRP paths from routing table to sender
        sendAllEigrpPaths(eigrpIft->findInterfaceById(ifaceId), neigh);
    }

    if (neigh->getRoutesForDeletion()) { // Remove unreachable routes waiting for Ack from neighbor
        eigrpTt->delayedRemove(neigh->getNeighborId());
        neigh->setRoutesForDeletion(false);
    }
}

void EigrpIpv6Pdm::processHelloPacket(Packet *pk, Ipv6Address& srcAddress, int ifaceId, EigrpNeighbor<Ipv6Address> *neigh)
{
    const EigrpIpv6Hello *hello = staticPtrCast<const EigrpIpv6Hello>(pk->peekAtFront<EigrpIpv6Hello>()).get();
//    EigrpIpv6Hello *hello = check_and_cast<EigrpIpv6Hello *>(msg);

    EigrpTlvParameter tlvParam = hello->getParameterTlv();

    printRecvMsg(hello, srcAddress, ifaceId);

    if (tlvParam.kValues == KVALUES_MAX) { // Received Goodbye message, remove neighbor
        if (neigh != nullptr) {
            EV_DEBUG << "     interface goodbye received" << endl;
            EV_DEBUG << "EIGRP neighbor " << srcAddress << " is down, interface goodbye received" << endl;
            removeNeighbor(neigh);
            flushMsgRequests();
            eigrpTt->purgeTable();
        }
    }
    else if (neigh == nullptr) { // New neighbor
        processNewNeighbor(ifaceId, srcAddress, hello);
    }
    else { // neighbor exists, its state is "up" or "pending"
        // Check K-values
        if (!(tlvParam.kValues == this->kValues)) { // Not satisfied
            EV_DEBUG << "EIGRP neighbor " << srcAddress << " is down, " << eigrp::UserMsgs[eigrp::M_NEIGH_BAD_KVALUES] << endl;
            removeNeighbor(neigh);
            flushMsgRequests();
            eigrpTt->purgeTable();

            // send Goodbye message and reset Hello timer
            EigrpInterface *iface = this->eigrpIft->findInterfaceById(ifaceId);
            resetTimer(iface->getHelloTimer(), iface->getHelloInt() - 0.5);
            EigrpMsgReq *msgReq = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<Ipv6Address>::UNSPEC_ID, ifaceId);
            msgReq->setGoodbyeMsg(true);
            send(msgReq, RTP_OUTGW);
        }
        else if (tlvParam.holdTimer != neigh->getHoldInt()) // Save Hold interval
            neigh->setHoldInt(tlvParam.holdTimer);
    }
}

void EigrpIpv6Pdm::processUpdatePacket(Packet *pk, Ipv6Address& srcAddress, int ifaceId, EigrpNeighbor<Ipv6Address> *neigh)
{
    const EigrpIpv6Update *update = staticPtrCast<const EigrpIpv6Update>(pk->peekAtFront<EigrpIpv6Update>()).get();

    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<Ipv6Address> *src;
    bool skipRoute, notifyDual, isSourceNew;

    printRecvMsg(update, srcAddress, ifaceId);

    if (neigh->isStateUp() == false && update->getAckNum() != 0) { // First ack from neighbor
        // If neighbor is "pending", then change its state to "up"
        neigh->setStateUp(true);
        // Send all EIGRP paths from routing table to sender
        sendAllEigrpPaths(eigrpIface, neigh);
    }

    if (update->getInit()) { // Request to send all paths from routing table
        if (neigh->isStateUp() == true) {
            sendAllEigrpPaths(eigrpIface, neigh);
        }
    }
    else if (neigh->isStateUp()) { // Neighbor is "up", forward message to DUAL
        int cnt = update->getInterRoutesArraySize();
#ifdef EIGRP_DEBUG
        EV_DEBUG << "     Route count:" << cnt << endl;
#endif

        for (int i = 0; i < cnt; i++) {
            skipRoute = false;
            notifyDual = false;
            EigrpMpIpv6Internal tlv = update->getInterRoutes(i);

            if (tlv.routerID == eigrpTt->getRouterId() || eigrpMetric->isParamMaximal(tlv.metric)) { // Route with RID is equal to RID of router or tlv is unreachable route
                Ipv6Address nextHop = getNextHopAddr(tlv.nextHop, srcAddress);
                if (eigrpTt->findRoute(tlv.destAddress, tlv.destMask, nextHop) == nullptr)
                    skipRoute = true; // Route is not found in TT -> discard route
            }

            if (skipRoute) { // Discard route
                EV_DEBUG << "EIGRP: discard route " << tlv.destAddress << endl;
            }
            else { // process route
                src = processInterRoute(tlv, srcAddress, neigh->getNeighborId(), eigrpIface, &notifyDual, &isSourceNew);
                if (notifyDual)
                    eigrpDual->processEvent(EigrpDual<Ipv6Address>::RECV_UPDATE, src, neigh->getNeighborId(), isSourceNew);
                else
                    EV_DEBUG << "EIGRP: route " << tlv.destAddress << " is not processed by DUAL, no change of metric" << endl;
            }
        }
        flushMsgRequests();
        eigrpTt->purgeTable();
    }
//     else ignore message");
}

void EigrpIpv6Pdm::processQueryPacket(Packet *pk, Ipv6Address& srcAddress, int ifaceId, EigrpNeighbor<Ipv6Address> *neigh)
{

    const EigrpIpv6Query *query = staticPtrCast<const EigrpIpv6Query>(pk->peekAtFront<EigrpIpv6Query>()).get();

    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<Ipv6Address> *src = nullptr;
    bool notifyDual, isSourceNew;

    printRecvMsg(query, srcAddress, ifaceId);

    int cnt = query->getInterRoutesArraySize();
#ifdef EIGRP_DEBUG
    EV_DEBUG << "     Route count:" << cnt << endl;
#endif

    for (int i = 0; i < cnt; i++) {
        src = processInterRoute(query->getInterRoutes(i), srcAddress, neigh->getNeighborId(), eigrpIface, &notifyDual, &isSourceNew);
        src->getRouteInfo()->incrementRefCnt(); // RouteInto cannot be removed while processing Query and generating Reply

        // Always notify DUAL
        eigrpDual->processEvent(EigrpDual<Ipv6Address>::RECV_QUERY, src, neigh->getNeighborId(), isSourceNew);
    }
    flushMsgRequests();
    eigrpTt->purgeTable();
}

void EigrpIpv6Pdm::processReplyPacket(Packet *pk, Ipv6Address& srcAddress, int ifaceId, EigrpNeighbor<Ipv6Address> *neigh)
{
    const EigrpIpv6Reply *reply = staticPtrCast<const EigrpIpv6Reply>(pk->peekAtFront<EigrpIpv6Reply>()).get();

    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<Ipv6Address> *src;
    bool notifyDual, isSourceNew;

    printRecvMsg(reply, srcAddress, ifaceId);

    int cnt = reply->getInterRoutesArraySize();
#ifdef EIGRP_DEBUG
    EV_DEBUG << "     Route count:" << cnt << endl;
#endif

    for (int i = 0; i < cnt; i++) {
        src = processInterRoute(reply->getInterRoutes(i), srcAddress, neigh->getNeighborId(), eigrpIface, &notifyDual, &isSourceNew);
        // Always notify DUAL
        eigrpDual->processEvent(EigrpDual<Ipv6Address>::RECV_REPLY, src, neigh->getNeighborId(), isSourceNew);
    }
    flushMsgRequests();
    eigrpTt->purgeTable();
}

/**
 * @param neigh Neighbor which is next hop for a route in TLV.
 */
EigrpRouteSource<Ipv6Address> *EigrpIpv6Pdm::processInterRoute(const EigrpMpIpv6Internal& tlv, Ipv6Address& srcAddr,
        int sourceNeighId, EigrpInterface *eigrpIface, bool *notifyDual, bool *isSourceNew)
{
    Ipv6Address nextHop = getNextHopAddr(tlv.nextHop, srcAddr);
    EigrpNeighbor<Ipv6Address> *nextHopNeigh = eigrpNt->findNeighbor(nextHop);
    EigrpRouteSource<Ipv6Address> *src;
    EigrpWideMetricPar newParam, oldParam, oldNeighParam;
    EigrpWideMetricPar ifParam = eigrpMetric->getParam(eigrpIface);

    // Find route or create one (route source is identified by ID of the next hop - not by ID of sender)
    src = eigrpTt->findOrCreateRoute(tlv.destAddress, tlv.destMask, tlv.routerID, eigrpIface, nextHopNeigh->getNeighborId(), isSourceNew);

    // Compare old and new neighbor's parameters
    oldNeighParam = src->getRdParams();
    if (*isSourceNew || !eigrpMetric->compareParameters(tlv.metric, oldNeighParam, this->kValues)) {
        // Compute reported distance (must be there)
        uint64_t metric = eigrpMetric->computeClassicMetric(tlv.metric, this->kValues);
        src->setRdParams(tlv.metric);
        src->setRd(metric);

        // Get new metric parameters
        newParam = eigrpMetric->adjustParam(ifParam, tlv.metric);
        // Get old metric parameters for comparison
        oldParam = src->getMetricParams();

        if (!eigrpMetric->compareParameters(newParam, oldParam, this->kValues)) {
            // Set source of route
            if (*isSourceNew)
                src->setNextHop(nextHop);
            src->setMetricParams(newParam);
            // Compute metric
            metric = eigrpMetric->computeClassicMetric(newParam, this->kValues);
            src->setMetric(metric);
            *notifyDual = true;
        }
    }

    return src;
}

EigrpTimer *EigrpIpv6Pdm::createTimer(char timerKind, void *context)
{
    EigrpTimer *timer = new EigrpTimer();
    timer->setTimerKind(timerKind);
    timer->setContextPointer(context);

    return timer;
}

Packet *EigrpIpv6Pdm::createHelloPacket(int holdInt, EigrpKValues kValues, Ipv6Address& destAddress, EigrpMsgReq *msgReq)
{

    const auto& msg = makeShared<EigrpIpv6Hello>();
    EigrpTlvParameter paramTlv;

    addMessageHeader(msg, EIGRP_HELLO_MSG, msgReq);

    // Add parameter type TLV
    paramTlv.holdTimer = holdInt;
    paramTlv.kValues = kValues;
    msg->setParameterTlv(paramTlv);

    // Add stub TLV
    if (this->eigrpStubEnabled) {
        EigrpTlvStub stubTlv;
        stubTlv.stub = this->eigrpStub;
        msg->setStubTlv(stubTlv);
    }

    msg->setChunkLength(B(sizeOfMsg + 12));

    Packet *packet = new Packet();
    packet->insertAtBack(msg);
    return packet;
}

Packet *EigrpIpv6Pdm::createAckPacket(Ipv6Address& destAddress, EigrpMsgReq *msgReq)
{
    const auto& msg = makeShared<EigrpIpv6Ack>();
    addMessageHeader(msg, EIGRP_HELLO_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
//    addCtrInfo(msg, msgReq->getDestInterface(), destAddress);

    msg->setChunkLength(B(sizeOfMsg));

    Packet *packet = new Packet();
    packet->insertAtBack(msg);
    return packet;
}

Packet *EigrpIpv6Pdm::createUpdatePacket(const Ipv6Address& destAddress, EigrpMsgReq *msgReq)
{
    const auto& msg = makeShared<EigrpIpv6Update>();
    int routeCnt = msgReq->getRoutesArraySize();

    addMessageHeader(msg, EIGRP_UPDATE_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
    msg->setSeqNum(msgReq->getSeqNumber());

    // Add route TLV
    if (routeCnt > 0) addRoutesToMsg(msg, msgReq);

    msg->setChunkLength(B(sizeOfMsg + routeCnt * 68));

    Packet *packet = new Packet();
    packet->insertAtBack(msg);
    return packet;
}

Packet *EigrpIpv6Pdm::createQueryPacket(Ipv6Address& destAddress, EigrpMsgReq *msgReq)
{
    const auto& msg = makeShared<EigrpIpv6Query>();
    int routeCnt = msgReq->getRoutesArraySize();

    addMessageHeader(msg, EIGRP_QUERY_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
    msg->setSeqNum(msgReq->getSeqNumber());

    // Add route TLV
    if (routeCnt > 0) addRoutesToMsg(msg, msgReq);

    msg->setChunkLength(B(sizeOfMsg + routeCnt * 68));

    Packet *packet = new Packet();
    packet->insertAtBack(msg);
    return packet;
}

Packet *EigrpIpv6Pdm::createReplyPacket(Ipv6Address& destAddress, EigrpMsgReq *msgReq)
{
    const auto& msg = makeShared<EigrpIpv6Reply>();
    int routeCnt = msgReq->getRoutesArraySize();

    addMessageHeader(msg, EIGRP_REPLY_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
    msg->setSeqNum(msgReq->getSeqNumber());

    // Add route TLV
    if (routeCnt > 0) {
        addRoutesToMsg(msg, msgReq);
        unlockRoutes(msgReq); // Locked RouteInfos are unlocked and can be removed
    }

    msg->setChunkLength(B(sizeOfMsg + routeCnt * 68));

    Packet *packet = new Packet();
    packet->insertAtBack(msg);
    return packet;
}

void EigrpIpv6Pdm::addMessageHeader(const Ptr<EigrpMessage>& msg, int opcode, EigrpMsgReq *msgReq)
{
    msg->setOpcode(opcode);
    msg->setAsNum(this->asNum);
    msg->setInit(msgReq->getInit());
    msg->setEot(msgReq->getEot());
    msg->setRs(msgReq->getRs());
    msg->setCr(msgReq->getCr());
}

void EigrpIpv6Pdm::createRouteTlv(EigrpMpIpv6Internal *routeTlv, EigrpRoute<Ipv6Address> *route, bool unreachable)
{
    EigrpWideMetricPar rtMetric = route->getRdPar();
    routeTlv->destAddress = route->getRouteAddress();
    routeTlv->destMask = route->getRouteMask();
    routeTlv->nextHop = Ipv6Address::UNSPECIFIED_ADDRESS;
    setRouteTlvMetric(&routeTlv->metric, &rtMetric);
    if (unreachable) {
        routeTlv->metric.delay = EigrpMetricHelper::DELAY_INF;
        routeTlv->metric.bandwidth = EigrpMetricHelper::BANDWIDTH_INF;
    }
}

void EigrpIpv6Pdm::setRouteTlvMetric(EigrpWideMetricPar *msgMetric, EigrpWideMetricPar *rtMetric)
{
    msgMetric->bandwidth = rtMetric->bandwidth;
    msgMetric->delay = rtMetric->delay;
    msgMetric->hopCount = rtMetric->hopCount;
    msgMetric->load = rtMetric->load;
    msgMetric->mtu = rtMetric->mtu;
    msgMetric->offset = 0; // TODO
    msgMetric->priority = 0; // TODO
    msgMetric->reliability = rtMetric->reliability;
}

void EigrpIpv6Pdm::unlockRoutes(const EigrpMsgReq *msgReq)
{
    int reqCnt = msgReq->getRoutesArraySize();
    EigrpRoute<Ipv6Address> *route = nullptr;

    for (int i = 0; i < reqCnt; i++) {
        EigrpMsgRoute req = msgReq->getRoutes(i);
        route = eigrpTt->findRouteInfoById(req.routeId);

        ASSERT(route != nullptr);
        route->decrementRefCnt();

        if (route->getRefCnt() < 1) { // Route would have been removed be removed if wasn't locked
            delete eigrpTt->removeRouteInfo(route);
        }
    }
}

void EigrpIpv6Pdm::addRoutesToMsg(const Ptr<EigrpIpv6Message>& msg, const EigrpMsgReq *msgReq)
{
    // Add routes to the message
    int reqCnt = msgReq->getRoutesArraySize();
    EigrpRouteSource<Ipv6Address> *source = nullptr;
    EigrpRoute<Ipv6Address> *route = nullptr;

    msg->setInterRoutesArraySize(reqCnt);
    for (int i = 0; i < reqCnt; i++) {
        EigrpMpIpv6Internal routeTlv;
        EigrpMsgRoute req = msgReq->getRoutes(i);

        if ((source = eigrpTt->findRouteById(req.sourceId)) == nullptr) { // Route was removed (only for sent Update messages to stub routers)
            route = eigrpTt->findRouteInfoById(req.routeId);
            ASSERT(route != nullptr);
        }
        else {
            route = source->getRouteInfo();
        }
        routeTlv.routerID = req.originator;
        createRouteTlv(&routeTlv, route, req.unreachable);

        msg->setInterRoutes(i, routeTlv);

#ifdef EIGRP_DEBUG
        EV_DEBUG << "     route: " << routeTlv.destAddress << "/" << getNetmaskLength(routeTlv.destMask);
        EV_DEBUG << " originator: " << routeTlv.routerID;
        if (eigrpMetric->isParamMaximal(routeTlv.metric)) EV_DEBUG << " (unreachable) ";
        EV_DEBUG << ", bw: " << routeTlv.metric.bandwidth;
        EV_DEBUG << ", dly: " << routeTlv.metric.delay;
        EV_DEBUG << ", hopcnt: " << (int)routeTlv.metric.hopCount;
        EV_DEBUG << endl;
#endif
    }
}

EigrpMsgReq *EigrpIpv6Pdm::createMsgReq(HeaderOpcode msgType, int destNeighbor, int destIface)
{
    EigrpMsgReq *msgReq = new EigrpMsgReq(eigrp::UserMsgs[msgType]);

    msgReq->setOpcode(msgType);
    msgReq->setDestNeighbor(destNeighbor);
    msgReq->setDestInterface(destIface);

    return msgReq;
}

void EigrpIpv6Pdm::sendAllEigrpPaths(EigrpInterface *eigrpIface, EigrpNeighbor<Ipv6Address> *neigh)
{
    int routeCnt = eigrpTt->getNumRouteInfo();
    int addedRoutes = 0; // Number of routes in message
    EigrpRoute<Ipv6Address> *route;
    EigrpRouteSource<Ipv6Address> *source;
    EigrpMsgReq *msgReq = createMsgReq(EIGRP_UPDATE_MSG, neigh->getNeighborId(), neigh->getIfaceId());

    msgReq->setRoutesArraySize(routeCnt);

    for (int i = 0; i < routeCnt; i++) {
        route = eigrpTt->getRouteInfo(i);
        if (route->isActive())
            continue;

        if ((source = eigrpTt->getBestSuccessor(route)) != nullptr) {
            if (this->eigrpStubEnabled && applyStubToUpdate(source))
                continue; // Apply stub settings to the route
            if (eigrpIface->isSplitHorizonEn() && applySplitHorizon(eigrpIface, source, route))
                continue; // Apply Split Horizon rule

            EigrpMsgRoute routeReq;
            routeReq.sourceId = source->getSourceId();
            routeReq.routeId = source->getRouteId();
            routeReq.originator = source->getOriginator();
            routeReq.invalid = false;
            msgReq->setRoutes(addedRoutes /* not variable i */, routeReq);
            addedRoutes++;
        }
    }

    if (addedRoutes < routeCnt) // reduce size of array
        msgReq->setRoutesArraySize(addedRoutes);

    msgReq->setEot(true);

    send(msgReq, RTP_OUTGW);
}

void EigrpIpv6Pdm::processNewNeighbor(int ifaceId, Ipv6Address& srcAddress, const EigrpIpv6Hello *rcvMsg)
{
    EigrpMsgReq *msgReqUpdate = nullptr, *msgReqHello = nullptr;
    EigrpNeighbor<Ipv6Address> *neigh;
    EigrpInterface *iface = eigrpIft->findInterfaceById(ifaceId);
    EigrpTlvParameter paramTlv = rcvMsg->getParameterTlv();
    EigrpStub stubConf = rcvMsg->getStubTlv().stub;
    int ecode; // Code of user message

    // Check rules for establishing neighborship
    if ((ecode = checkNeighborshipRules(ifaceId, rcvMsg->getAsNum(), srcAddress,
            paramTlv.kValues)) != eigrp::M_OK)
    {
        EV_DEBUG << "EIGRP can't create neighborship with " << srcAddress << ", " << eigrp::UserMsgs[ecode] << endl;

        if (ecode == eigrp::M_NEIGH_BAD_KVALUES) { // Send Goodbye message and reset Hello timer
            resetTimer(iface->getHelloTimer(), iface->getHelloInt() - 0.5);
            msgReqHello = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<Ipv6Address>::UNSPEC_ID, ifaceId);
            msgReqHello->setGoodbyeMsg(true);
            send(msgReqHello, RTP_OUTGW);
        }
        return;
    }

    EV_DEBUG << "Neighbor " << srcAddress << " is up, new adjacency" << endl;

    neigh = createNeighbor(iface, srcAddress, paramTlv.holdTimer);

    if (stubConf.connectedRt || stubConf.leakMapRt || stubConf.recvOnlyRt || stubConf.redistributedRt || stubConf.staticRt || stubConf.summaryRt) { // Process stub configuration
        neigh->setStubEnable(true);
        neigh->setStubConf(stubConf);
        iface->incNumOfStubs();
        eigrpNt->incStubCount();
    }

    // Reply with Hello message and reset Hello timer
    resetTimer(iface->getHelloTimer(), iface->getHelloInt() - 0.5);
    msgReqHello = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<Ipv6Address>::UNSPEC_ID, ifaceId);
    send(msgReqHello, RTP_OUTGW);

    // Send Update with INIT flag
    msgReqUpdate = createMsgReq(EIGRP_UPDATE_MSG, neigh->getNeighborId(), neigh->getIfaceId());
    msgReqUpdate->setInit(true);
    send(msgReqUpdate, RTP_OUTGW);
}

int EigrpIpv6Pdm::checkNeighborshipRules(int ifaceId, int neighAsNum,
        Ipv6Address& neighAddr, const EigrpKValues& neighKValues)
{
    Ipv6Address ifaceAddr, ifaceMask;

    if (this->eigrpIft->findInterfaceById(ifaceId) == nullptr) { // EIGRP must be enabled on interface
        return eigrp::M_DISABLED_ON_IF;
    }
    else if (this->asNum != neighAsNum) { // AS numbers must be equal
        return eigrp::M_NEIGH_BAD_AS;
    }
    else if (!(this->kValues == neighKValues)) { // K-values must be same
        return eigrp::M_NEIGH_BAD_KVALUES;
    }
    else {
        // check, if the neighbor uses as source address Link-local address
        if (neighAddr.getScope() != Ipv6Address::LINK) { // source address is not Link-local address -> bad
            return eigrp::M_NEIGH_BAD_SUBNET;
        }
    }

    return eigrp::M_OK;
}

EigrpNeighbor<Ipv6Address> *EigrpIpv6Pdm::createNeighbor(EigrpInterface *eigrpIface, Ipv6Address& address, uint16_t holdInt)
{
    // Create record in the neighbor table
    EigrpNeighbor<Ipv6Address> *neigh = new EigrpNeighbor<Ipv6Address>(eigrpIface->getInterfaceId(), eigrpIface->getInterfaceName(), address);
    EigrpTimer *holdt = createTimer(EIGRP_HOLD_TIMER, neigh);
    neigh->setHoldTimer(holdt);
    neigh->setHoldInt(holdInt);
    eigrpNt->addNeighbor(neigh);
    // Start Hold timer
    scheduleAt(simTime() + holdInt, holdt);

    eigrpIface->incNumOfNeighbors();

    return neigh;
}

// Must be there (EigrpNeighborTable has no method cancelEvent)
void EigrpIpv6Pdm::cancelHoldTimer(EigrpNeighbor<Ipv6Address> *neigh)
{
    EigrpTimer *timer;

    if ((timer = neigh->getHoldTimer()) != nullptr) {
        if (timer->isScheduled()) {
            timer->getOwner();
            cancelEvent(timer);
        }
    }
}

void EigrpIpv6Pdm::cancelHelloTimers()
{
    EigrpInterface *iface;
    EigrpTimer *timer;

    int cnt = this->eigrpIft->getNumInterfaces();
    for (int i = 0; i < cnt; i++) {
        iface = this->eigrpIft->getInterface(i);

        if ((timer = iface->getHelloTimer()) != nullptr) {
            if (timer->isScheduled()) {
                timer->getOwner();
                cancelEvent(timer);
            }

            iface->setHelloTimerPtr(nullptr);
            delete timer;
        }
    }

    cnt = this->eigrpIftDisabled->getNumInterfaces();
    for (int i = 0; i < cnt; i++) {
        iface = this->eigrpIftDisabled->getInterface(i);

        if ((timer = iface->getHelloTimer()) != nullptr) {
            if (timer->isScheduled()) {
                timer->getOwner();
                cancelEvent(timer);
            }

            iface->setHelloTimerPtr(nullptr);
            delete timer;
        }
    }
}

void EigrpIpv6Pdm::removeNeighbor(EigrpNeighbor<Ipv6Address> *neigh)
{
    EigrpRouteSource<Ipv6Address> *source = nullptr;
    EigrpRoute<Ipv6Address> *route = nullptr;
    // Find interface (enabled/disabled)
    EigrpInterface *eigrpIface = getInterfaceById(neigh->getIfaceId());

    int nextHopId = neigh->getNeighborId();
    int ifaceId = neigh->getIfaceId();
    const char *ifaceName = neigh->getIfaceName();
    int routeId;

    cancelHoldTimer(neigh);
    // Remove neighbor from NT
    this->eigrpNt->removeNeighbor(neigh);
    eigrpIface->decNumOfNeighbors();
    if (neigh->isStubEnabled()) {
        eigrpIface->decNumOfStubs();
        eigrpNt->decStubCount();
    }
    delete neigh;
    neigh = nullptr;

    for (int i = 0; i < eigrpTt->getNumRouteInfo(); i++) { // Process routes that go via this neighbor
        route = eigrpTt->getRouteInfo(i);
        routeId = route->getRouteId();
        // Note: TT can not contain two or more records with the same network address and next hop address
        source = eigrpTt->findRouteByNextHop(routeId, nextHopId);

#ifdef EIGRP_DEBUG
        EV_DEBUG << "EIGRP: Destination: " << route->getRouteAddress() << "/" << getNetmaskLength(route->getRouteMask()) << ", active " << route->isActive() << endl;
#endif

        if (route->isActive()) {
            if (source == nullptr) { // Create dummy source for active route (instead of Reply)
#ifdef EIGRP_DEBUG
                EV_DEBUG << "     Create dummy route " << route->getRouteAddress() << " via <unspecified> for deletion of reply status handle" << endl;
#endif
                source = new EigrpRouteSource<Ipv6Address>(ifaceId, ifaceName, nextHopId, routeId, route);
                eigrpTt->addRoute(source);
                eigrpDual->processEvent(EigrpDual<Ipv6Address>::NEIGHBOR_DOWN, source, nextHopId, true);
            }
            else
                eigrpDual->processEvent(EigrpDual<Ipv6Address>::NEIGHBOR_DOWN, source, nextHopId, false);
        }
        else { // Notify DUAL about event
            if (source != nullptr)
                eigrpDual->processEvent(EigrpDual<Ipv6Address>::NEIGHBOR_DOWN, source, nextHopId, false);
        }
    }

    // Do not flush messages in transmit queue
}

Ipv6Route *EigrpIpv6Pdm::createRTRoute(EigrpRouteSource<Ipv6Address> *successor)
{
    EigrpRoute<Ipv6Address> *route = successor->getRouteInfo();
//    ANSAIPv6Route *rtEntry = new ANSAIPv6Route(route->getRouteAddress(), getNetmaskLength(route->getRouteMask()), IPv6Route::ROUTING_PROT);
    Ipv6Route *rtEntry = new Ipv6Route(route->getRouteAddress(), getNetmaskLength(route->getRouteMask()), IRoute::EIGRP);
    rtEntry->setInterface(ift->getInterfaceById(successor->getIfaceId()));
    rtEntry->setNextHop(successor->getNextHop());
    setRTRouteMetric(rtEntry, successor->getMetric());

    // Set protocol source and AD
    if (successor->isInternal()) {
//        rtEntry->setSourceType(IPv6Route::EIGRP);
        rtEntry->setAdminDist(Ipv6Route::dEIGRPInternal);
    }
    else {
//        rtEntry->setRoutingProtocolSource(ANSAIPv6Route::pEIGRPext);
        rtEntry->setAdminDist(Ipv6Route::dEIGRPExternal);
    }
    return rtEntry;
}

void EigrpIpv6Pdm::msgToAllIfaces(int destination, HeaderOpcode msgType, EigrpRouteSource<Ipv6Address> *source, bool forcePoisonRev, bool forceUnreachable)
{
    EigrpInterface *eigrpIface;
    int ifCount = eigrpIft->getNumInterfaces();
    int numOfNeigh;

    for (int i = 0; i < ifCount; i++) {
        eigrpIface = eigrpIft->getInterface(i);

        // Send message only to interface with stub neighbor
        if (destination == IEigrpPdm::STUB_RECEIVER && eigrpIface->getNumOfStubs() == 0)
            continue;

        if ((numOfNeigh = eigrpIface->getNumOfNeighbors()) > 0) {
            if (msgType == EIGRP_QUERY_MSG) {
                if (!applyStubToQuery(eigrpIface, numOfNeigh))
                    msgToIface(msgType, source, eigrpIface, forcePoisonRev);
                // Else do not send Query to stub router (skip the route on interface)
            }
            else
                msgToIface(msgType, source, eigrpIface, forcePoisonRev, forceUnreachable);
        }
    }
}

bool EigrpIpv6Pdm::applyStubToQuery(EigrpInterface *eigrpIface, int numOfNeigh)
{
    if (this->eigrpStubEnabled)
        return false; // Send Query to all neighbors
    if (numOfNeigh == eigrpIface->getNumOfStubs())
        return true; // Do not send Query to stub router
    return false;
}

void EigrpIpv6Pdm::msgToIface(HeaderOpcode msgType, EigrpRouteSource<Ipv6Address> *source, EigrpInterface *eigrpIface, bool forcePoisonRev, bool forceUnreachable)
{
    EigrpNeighbor<Ipv6Address> *neigh = nullptr;
    EigrpMsgRoute msgRt;
    int ifaceId, destNeigh = 0;

    if (eigrpIface->isSplitHorizonEn() && applySplitHorizon(eigrpIface, source, source->getRouteInfo())) {
        if (forcePoisonRev) { // Apply Poison Reverse instead of Split Horizon
            msgRt.unreachable = true;
        }
        else // Apply Split Horizon rule - do not send route to the interface
            return;
    }

    msgRt.sourceId = source->getSourceId();
    msgRt.routeId = source->getRouteId();
    msgRt.originator = source->getOriginator();
    msgRt.invalid = false;

    // Get destination interface ID and destination neighbor ID
    ifaceId = eigrpIface->getInterfaceId();
    if (ift->getInterfaceById(ifaceId)->isMulticast()) { // Multicast
        destNeigh = IEigrpPdm::UNSPEC_RECEIVER;
    }
    else { // Unicast
        if (neigh == nullptr)
            neigh = eigrpNt->getFirstNeighborOnIf(ifaceId);
        destNeigh = neigh->getNeighborId();
    }

    pushMsgRouteToQueue(msgType, ifaceId, destNeigh, msgRt);
}

EigrpMsgReq *EigrpIpv6Pdm::pushMsgRouteToQueue(HeaderOpcode msgType, int ifaceId, int neighId, const EigrpMsgRoute& msgRt)
{
    EigrpMsgReq *request = nullptr;
    RequestVector::iterator it;

    // Find or create message
    for (it = reqQueue.begin(); it != reqQueue.end(); it++) {
        if ((*it)->getDestInterface() == ifaceId && (*it)->getOpcode() == msgType) {
            request = *it;
            break;
        }
    }
    if (request == nullptr) { // Create new request
        request = createMsgReq(msgType, neighId, ifaceId);
        request->setRoutesArraySize(1);
        request->setRoutes(0, msgRt);
        reqQueue.push_back(request);
    }
    else { // Use existing request
        int rtSize = request->getRoutesArraySize();
        int index;
        if ((index = request->findMsgRoute(msgRt.routeId)) >= 0) { // Use existing route
            if (msgRt.unreachable)
                request->getRoutesForUpdate(index).unreachable = true;
        }
        else { // Add route to request
            request->setRoutesArraySize(rtSize + 1);
            request->setRoutes(rtSize, msgRt);
        }
    }

    return request;
}

bool EigrpIpv6Pdm::applySplitHorizon(EigrpInterface *destInterface, EigrpRouteSource<Ipv6Address> *source, EigrpRoute<Ipv6Address> *route)
{
    if (route->getNumSucc() <= 1) // Only 1 successor, check its interface ID (source is always successor)
        return destInterface->getInterfaceId() == source->getIfaceId();
    else // There is more than 1 successor. Is any of them on the interface?
        return eigrpTt->getBestSuccessorByIf(route, destInterface->getInterfaceId()) != nullptr;
}

bool EigrpIpv6Pdm::applyStubToUpdate(EigrpRouteSource<Ipv6Address> *src)
{
    if (this->eigrpStub.recvOnlyRt)
        return true; // Do not send any route

    // Send only specified type of route
    else if (this->eigrpStub.connectedRt && src->getNexthopId() == EigrpNeighbor<Ipv6Address>::UNSPEC_ID)
        return false;
    else if (this->eigrpStub.redistributedRt && src->isRedistributed())
        return false;
    else if (this->eigrpStub.summaryRt && src->isSummary())
        return false;
    else if (this->eigrpStub.staticRt && src->isRedistributed())
        return false;

    // TODO leakMapRt

    return true;
}

void EigrpIpv6Pdm::flushMsgRequests()
{
    RequestVector::iterator it;
    Ipv6Address destAddress;

    // Send Query
    for (it = reqQueue.begin(); it != reqQueue.end(); it++) {
        if ((*it)->getOpcode() == EIGRP_QUERY_MSG) {
            // Check if interface exists
            if (eigrpIft->findInterfaceById((*it)->getDestInterface()) == nullptr)
                continue;
            else
                send(*it, RTP_OUTGW);
        }
    }

    // Send other messages
    for (it = reqQueue.begin(); it != reqQueue.end(); it++) {
        // Check if interface exists
        if (eigrpIft->findInterfaceById((*it)->getDestInterface()) == nullptr) {
            delete *it; // Discard request
            continue;
        }

        if ((*it)->getOpcode() != EIGRP_QUERY_MSG)
            send(*it, RTP_OUTGW);
    }

    reqQueue.clear();
}

EigrpInterface *EigrpIpv6Pdm::getInterfaceById(int ifaceId)
{
    EigrpInterface *iface;

    if ((iface = eigrpIft->findInterfaceById(ifaceId)) != nullptr)
        return iface;
    else
        return eigrpIftDisabled->findInterface(ifaceId);
}

void EigrpIpv6Pdm::disableInterface(NetworkInterface *iface, EigrpInterface *eigrpIface)
{
    EigrpTimer *hellot;
    EigrpNeighbor<Ipv6Address> *neigh;
    EigrpRouteSource<Ipv6Address> *source;
    int neighCount;
    int ifaceId = eigrpIface->getInterfaceId();

    EV_DEBUG << "EIGRP disabled on interface " << eigrpIface->getName() << "(" << ifaceId << ")" << endl;

    if (!eigrpIface->isPassive()) {
        // Unregister multicast address
        Ipv6InterfaceData *ipv6int = iface->getProtocolDataForUpdate<Ipv6InterfaceData>();
        ipv6int->leaveMulticastGroup(EIGRP_IPV6_MULT);
        ipv6int->removeAddress(EIGRP_IPV6_MULT);
    }

//    iface->ipv6Data()->leaveMulticastGroup(EIGRP_IPV6_MULT);
//    iface->ipv6Data()->removeAddress(EIGRP_IPV6_MULT);

    // stop hello timer
    if ((hellot = eigrpIface->getHelloTimer()) != nullptr)
        cancelEvent(hellot);

    std::set<int>::iterator it;
    EigrpNetwork<Ipv6Address> *eigrpnet = nullptr;

    for (it = eigrpIface->getNetworksIdsBegin(); it != eigrpIface->getNetworksIdsEnd(); ++it) {
        eigrpnet = routingForNetworks->findNetworkById(*it);
        source = eigrpTt->findRoute(eigrpnet->getAddress(), eigrpnet->getMask(), EigrpNeighbor<Ipv6Address>::UNSPEC_ID);
        ASSERT(source != nullptr);
        // Notify DUAL about event
        eigrpDual->processEvent(EigrpDual<Ipv6Address>::INTERFACE_DOWN, source, EigrpNeighbor<Ipv6Address>::UNSPEC_ID, false);
    }

    eigrpIface->clearNetworkIds();

    // Remove interface from EIGRP interface table (must be there)
    if (eigrpIface->isEnabled()) {
        eigrpIft->removeInterface(eigrpIface);
        eigrpIftDisabled->addInterface(eigrpIface);
        eigrpIface->setEnabling(false);
    }

    // Delete all neighbors on the interface
    neighCount = eigrpNt->getNumNeighbors();
    for (int i = 0; i < neighCount; i++) {
        neigh = eigrpNt->getNeighbor(i);
        if (neigh->getIfaceId() == ifaceId) {
            removeNeighbor(neigh);
        }
    }

    flushMsgRequests();
    eigrpTt->purgeTable();
}

EigrpInterface *EigrpIpv6Pdm::addInterfaceToEigrp(int ifaceId, bool enabled)
{
    NetworkInterface *iface = ift->getInterfaceById(ifaceId);
    // create EIGRP interface
    EigrpInterface *eigrpIface = nullptr;

    eigrpIface = getInterfaceById(ifaceId); // search for existing iface

    if (eigrpIface == nullptr) { // iface not found -> create new
        eigrpIface = new EigrpInterface(iface, EigrpNetworkTable<Ipv6Address>::UNSPEC_NETID, false);
    }

    if (enabled) {
        enableInterface(eigrpIface);
        startHelloTimer(eigrpIface, simTime() + uniform(0, 1));
    }
    else {
        eigrpIftDisabled->addInterface(eigrpIface);
    }

    return eigrpIface;
}

void EigrpIpv6Pdm::enableInterface(EigrpInterface *eigrpIface)
{
    int ifaceId = eigrpIface->getInterfaceId();
    EigrpRouteSource<Ipv6Address> *src;
    EigrpWideMetricPar metricPar;
    bool isSourceNew;

    EV_DEBUG << "EIGRP enabled on interface " << eigrpIface->getName() << "(" << ifaceId << ")" << endl;

    // Move interface to EIGRP interface table
    if (!eigrpIface->isEnabled()) {
        eigrpIftDisabled->removeInterface(eigrpIface);
        eigrpIft->addInterface(eigrpIface);
        eigrpIface->setEnabling(true);
    }

    // Register multicast address on interface      //TODO - should be passive interface joined in multicast group?
//    IPv6InterfaceData *ifaceIpv6 = ift->getInterfaceById(ifaceId)->ipv6Data();
    //ifaceIpv6->joinMulticastGroup(EIGRP_IPV6_MULT); //join to group FF02::A, optionally
    //ifaceIpv6->assignAddress(EIGRP_IPV6_MULT, false, 0, 0); //add group address to interface, mandatory

    Ipv6InterfaceData *ipv6int = ift->getInterfaceById(ifaceId)->getProtocolDataForUpdate<Ipv6InterfaceData>();
    ipv6int->joinMulticastGroup(EIGRP_IPV6_MULT);
    ipv6int->assignAddress(EIGRP_IPV6_MULT, false, 0, 0);

    PrefixVector::iterator it;
    EigrpNetwork<Ipv6Address> *eigrpnet = nullptr;
    Ipv6Address network;
    Ipv6Address mask;

    for (it = netPrefixes.begin(); it != netPrefixes.end(); ++it) { // through all known prefixes search prefixes belonging to enabling interface
        if (it->ifaceId == ifaceId) { // found prefix belonging to interface
            network = it->network;
            mask = Ipv6Address(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff).getPrefix(it->prefixLength);

            eigrpnet = addNetwork(network, mask);

            if (eigrpnet) { // successfully added to EigrpNetworkTable -> tie with interface
                eigrpIface->insertToNetworksIds(eigrpnet->getNetworkId()); // pak predelat na protected a pridat metody
            }

            // Create route
            src = eigrpTt->findOrCreateRoute(network, mask, eigrpTt->getRouterId(), eigrpIface, EigrpNeighbor<Ipv6Address>::UNSPEC_ID, &isSourceNew);
            ASSERT(isSourceNew == true);
            // Compute metric
            metricPar = eigrpMetric->getParam(eigrpIface);
            src->setMetricParams(metricPar);
            uint64_t metric = eigrpMetric->computeClassicMetric(metricPar, this->kValues);
            src->setMetric(metric);

            // Notify DUAL about event
            eigrpDual->processEvent(EigrpDual<Ipv6Address>::INTERFACE_UP, src, EigrpNeighbor<Ipv6Address>::UNSPEC_ID, isSourceNew);
        }
    }

    flushMsgRequests();
    eigrpTt->purgeTable();
}

void EigrpIpv6Pdm::startHelloTimer(EigrpInterface *eigrpIface, simtime_t interval)
{
    EigrpTimer *hellot;

    // Start Hello timer on interface
    if (!eigrpIface->isPassive()) {
        if ((hellot = eigrpIface->getHelloTimer()) == nullptr) {
            hellot = createTimer(EIGRP_HELLO_TIMER, eigrpIface);
            eigrpIface->setHelloTimerPtr(hellot);
        }

        scheduleAt(interval, hellot);
    }
}

//
// interface IEigrpModule
//

EigrpNetwork<Ipv6Address> *EigrpIpv6Pdm::addNetwork(Ipv6Address address, Ipv6Address mask)
{
    return routingForNetworks->addNetwork(address, mask);
}

void EigrpIpv6Pdm::setHelloInt(int interval, int ifaceId)
{
    EigrpInterface *iface = getInterfaceById(ifaceId);
    if (iface == nullptr)
        iface = addInterfaceToEigrp(ifaceId, false);
    iface->setHelloInt(interval);
}

void EigrpIpv6Pdm::setHoldInt(int interval, int ifaceId)
{
    EigrpInterface *iface = getInterfaceById(ifaceId);
    if (iface == nullptr)
        iface = addInterfaceToEigrp(ifaceId, false);
    iface->setHoldInt(interval);
}

void EigrpIpv6Pdm::setSplitHorizon(bool shenabled, int ifaceId)
{
    EigrpInterface *iface = getInterfaceById(ifaceId);
    if (iface == nullptr)
        iface = addInterfaceToEigrp(ifaceId, false);
    iface->setSplitHorizon(shenabled);
}

void EigrpIpv6Pdm::setPassive(bool passive, int ifaceId)
{
    EigrpInterface *eigrpIface = getInterfaceById(ifaceId);

    if (eigrpIface == nullptr)
        eigrpIface = addInterfaceToEigrp(ifaceId, false);
    else if (eigrpIface->isEnabled()) { // Disable sending and receiving of messages
        NetworkInterface *iface = ift->getInterfaceById(ifaceId);

        Ipv6InterfaceData *ipv6int = iface->getProtocolDataForUpdate<Ipv6InterfaceData>();
        ipv6int->leaveMulticastGroup(EIGRP_IPV6_MULT);

//        iface->ipv6Data()->leaveMulticastGroup(EIGRP_IPV6_MULT);

        /*
        if(iface->ipv6Data()->hasAddress(EIGRP_IPV6_MULT))
        {
            iface->ipv6Data()->removeAddress(EIGRP_IPV6_MULT);
        }
         */

        if (ipv6int->hasAddress(EIGRP_IPV6_MULT)) {
            ipv6int->removeAddress(EIGRP_IPV6_MULT);
        }

        // Stop and delete hello timer
        EigrpTimer *hellot = eigrpIface->getHelloTimer();
        if (hellot != nullptr) {
            cancelEvent(hellot);
            delete hellot;
            eigrpIface->setHelloTimerPtr(nullptr);
        }
    }
    // Else do nothing (interface is not part of EIGRP)

    eigrpIface->setPassive(passive);
}

//
// interface IEigrpPdm
//

void EigrpIpv6Pdm::sendUpdate(int destNeighbor, EigrpRoute<Ipv6Address> *route, EigrpRouteSource<Ipv6Address> *source, bool forcePoisonRev, const char *reason)
{
    EV_DEBUG << "DUAL: send Update message about " << route->getRouteAddress() << " to all neighbors, " << reason << endl;
    if (this->eigrpStubEnabled && applyStubToUpdate(source)) {
        EV_DEBUG << "     Stub routing applied, message will not be sent" << endl;
        return;
    }
    msgToAllIfaces(destNeighbor, EIGRP_UPDATE_MSG, source, forcePoisonRev, false);
}

void EigrpIpv6Pdm::sendQuery(int destNeighbor, EigrpRoute<Ipv6Address> *route, EigrpRouteSource<Ipv6Address> *source, bool forcePoisonRev)
{
    bool forceUnreachable = false;

    EV_DEBUG << "DUAL: send Query message about " << route->getRouteAddress() << " to all neighbors" << endl;

    if (this->eigrpStubEnabled)
        forceUnreachable = true; // Send Query with infinite metric to all neighbors
    else {
        if (this->eigrpNt->getStubCount() > 0) { // Apply Poison Reverse instead of Split Horizon rule
            forcePoisonRev = true;
        }
    }

    msgToAllIfaces(destNeighbor, EIGRP_QUERY_MSG, source, forcePoisonRev, forceUnreachable);
}

void EigrpIpv6Pdm::sendReply(EigrpRoute<Ipv6Address> *route, int destNeighbor, EigrpRouteSource<Ipv6Address> *source, bool forcePoisonRev, bool isUnreachable)
{
    EigrpMsgRoute msgRt;
    EigrpNeighbor<Ipv6Address> *neigh = eigrpNt->findNeighborById(destNeighbor);

    EV_DEBUG << "DUAL: send Reply message about " << route->getRouteAddress() << endl;
    if (neigh == nullptr)
        return;

    msgRt.invalid = false;
    msgRt.sourceId = source->getSourceId();
    msgRt.routeId = source->getRouteId();
    msgRt.originator = source->getOriginator();
    msgRt.unreachable = isUnreachable;

    if (this->eigrpStubEnabled)
        msgRt.unreachable = true; // Stub router always reply with infinite metric

    // Apply Poison Reverse (instead of Split Horizon)
    if (!isUnreachable && eigrpIft->findInterfaceById(neigh->getIfaceId())->isSplitHorizonEn()) { // Poison Reverse is enabled when Split Horizon is enabled
        // Note: destNeighbor is also neighbor that sent Query
        if (forcePoisonRev)
            msgRt.unreachable = true;
    }

    pushMsgRouteToQueue(EIGRP_REPLY_MSG, neigh->getIfaceId(), neigh->getNeighborId(), msgRt);
}

/**
* @return if route is found in routing table then returns true.
*/
bool EigrpIpv6Pdm::removeRouteFromRT(EigrpRouteSource<Ipv6Address> *source, IRoute::SourceType *removedRtSrc)
{
    EigrpRoute<Ipv6Address> *route = source->getRouteInfo();
    Ipv6Route *rtEntry = findRoute(route->getRouteAddress(), getNetmaskLength(route->getRouteMask()), source->getNextHop());
//    ANSAIPv6Route *ansaRtEntry = dynamic_cast<ANSAIPv6Route *>(rtEntry);
    Ipv6Route *ansaRtEntry = rtEntry;
    if (ansaRtEntry != nullptr) {
        *removedRtSrc = ansaRtEntry->getSourceType();
        //if (*removedRtSrc == ANSAIPv6Route::pEIGRP)
        if (ansaRtEntry->getSourceType() == IRoute::EIGRP) {
            EV_DEBUG << "EIGRP: delete route " << route->getRouteAddress() << " via " << source->getNextHop() << " from RT" << endl;
            delete rt->removeRoute(rtEntry);
        }
    }
    else {
#ifdef EIGRP_DEBUG
        EV_DEBUG << "EIGRP: EIGRP route " << route->getRouteAddress() << " via " << source->getNextHop() << " can not be removed, not found in RT" << endl;
#endif
    }
    return rtEntry != nullptr;
}

bool EigrpIpv6Pdm::isRTSafeForAdd(EigrpRoute<Ipv6Address> *route, unsigned int eigrpAd)
{
    Ipv6Route *routeInTable = findRoute(route->getRouteAddress(), getNetmaskLength(route->getRouteMask()));
    Ipv6Route *ansaRoute = nullptr;

    if (routeInTable == nullptr)
        return true; // Route not found

    ansaRoute = routeInTable;
    if (ansaRoute != nullptr) { // AnsaIPv4Route use own AD attribute
        if (ansaRoute->getAdminDist() < eigrpAd)
            return false;
        return true;
    }
    if (ansaRoute == nullptr && routeInTable->getAdminDist() == Ipv6Route::dUnknown)
        return false; // Connected route has AD = 255 (dUnknown) in IPv4Route
    if (routeInTable != nullptr && routeInTable->getAdminDist() < eigrpAd)
        return false; // Other IPv4Route with right AD
    return true;
}

EigrpRouteSource<Ipv6Address> *EigrpIpv6Pdm::updateRoute(EigrpRoute<Ipv6Address> *route, uint64_t dmin, bool *rtableChanged, bool removeUnreach)
{
    EigrpRouteSource<Ipv6Address> *source = nullptr, *bestSuccessor = nullptr;
    Ipv6Route *rtEntry = nullptr;
    int routeNum = eigrpTt->getNumRoutes();
    int routeId = route->getRouteId();
    int pathsInRT = 0; // Number of paths in RT (equal to number of successors)
    int sourceCounter = 0; // Number of route sources
    uint64_t routeFd = route->getFd();

    EV_DEBUG << "EIGRP: Search successor for route " << route->getRouteAddress() << ", FD is " << route->getFd() << endl;

    for (int i = 0; i < routeNum; i++) {
        source = eigrpTt->getRoute(i);
        if (source->getRouteId() != routeId || !source->isValid())
            continue;

        sourceCounter++;

        if (source->getRd() < routeFd /* FC, use this FD (not dmin) */ &&
            source->getMetric() <= dmin * this->variance && pathsInRT < this->maximumPath) /* Load Balancing */
        {
            EV_DEBUG << "     successor " << source->getNextHop() << " (" << source->getMetric() << "/" << source->getRd() << ")" << endl;

            if ((rtEntry = findRoute(route->getRouteAddress(), getNetmaskLength(route->getRouteMask()), source->getNextHop())) == nullptr)
                if (!isRTSafeForAdd(route, adminDistInt)) { // In RT exists route with smaller AD, do not mark the route source as successor
                    source->setSuccessor(false);
                    EV_DEBUG << "           route can not be added into RT, there is route with smaller AD" << endl;
                    continue; // Skip the route
                }

            if (installRouteToRT(route, source, dmin, rtEntry))
                *rtableChanged = true;

            pathsInRT++;
            source->setSuccessor(true);
        }
        else if (source->isSuccessor()) { // Remove old successor from RT
            if (removeOldSuccessor(source, route))
                *rtableChanged = true;
        }

        if (removeUnreach && source->isUnreachable() && source->getDelayedRemove() == 0 && source->isValid()) { // Invalidate unreachable routes in TT
            source->setValid(false);
            EV_DEBUG << "     invalidate route via " << source->getNextHop() << " in TT" << endl;
        }
    }

    route->setNumSucc(pathsInRT);

    if ((bestSuccessor = eigrpTt->getBestSuccessor(route)) != nullptr) { // Update route with best Successor
        if (dmin < routeFd) // Set only if there is a successor
            route->setFd(dmin);
        route->setDij(dmin);
        route->setRdPar(bestSuccessor->getMetricParams());
        route->setSuccessor(bestSuccessor);

        if (sourceCounter == 1) { // Set FD of route to Successor metric (according to Cisco EIGRP implementation)
            route->setFd(bestSuccessor->getMetric());
        }
    }
    else
        route->setSuccessor(nullptr);

    return bestSuccessor;
}

bool EigrpIpv6Pdm::removeOldSuccessor(EigrpRouteSource<Ipv6Address> *source, EigrpRoute<Ipv6Address> *route)
{
//    ANSAIPv4Route::RoutingProtocolSource srcProto = ANSAIPv4Route::pUnknown;
    IRoute::SourceType srcProto = IRoute::UNKNOWN;
    bool rtFound, rtableChanged = false;

    // To distinguish the route deleted by EIGRP in ReceiveChangeNotification method
    source->setSuccessor(false);

    rtFound = removeRouteFromRT(source, &srcProto);
#ifdef EIGRP_DEBUG
//    EV_DEBUG << "EIGRP: removing old successor: rt found: " << rtFound << " src: " << srcProto << endl;
#endif

//    if (!rtFound || (rtFound && (srcProto == ANSAIPv6Route::pEIGRP || srcProto == ANSAIPv6Route::pEIGRPext)))
    if (!rtFound || (rtFound && (srcProto == IRoute::EIGRP))) {
        if (route->getSuccessor() == source)
            route->setSuccessor(nullptr);
        rtableChanged = true;
    }
    else // Route from other sources can not be removed, do not change Successor's record in TT
        source->setSuccessor(true);

    return rtableChanged;
}

bool EigrpIpv6Pdm::installRouteToRT(EigrpRoute<Ipv6Address> *route, EigrpRouteSource<Ipv6Address> *source, uint64_t dmin, Ipv6Route *rtEntry)
{
    Ipv6Route *ansaRtEntry = rtEntry;
    bool rtableChanged = false;

    //if (rtEntry != nullptr && (ansaRtEntry = dynamic_cast<ANSAIPv6Route *>(rtEntry)) != nullptr)
    if (ansaRtEntry != nullptr) {
        //if (ansaRtEntry->getRoutingProtocolSource() != ANSAIPv4Route::pEIGRP)
        if (ansaRtEntry->getSourceType() != IRoute::EIGRP)
            return rtableChanged; // Do not add route to RT
        else if ((unsigned int)ansaRtEntry->getMetric() != source->getMetric()) { // Update EIGRP route in RT
            EV_DEBUG << "EIGRP: Update EIGRP route " << route->getRouteAddress() << " via " << source->getNextHop() << " in RT" << endl;
            setRTRouteMetric(ansaRtEntry, source->getMetric());
        }
    }
    else { // Insert new route to RT
        EV_DEBUG << "EIGRP: add EIGRP route " << route->getRouteAddress() << " via " << source->getNextHop() << " to RT" << endl;
        ansaRtEntry = createRTRoute(source);
//        rt->prepareForAddRoute(ansaRtEntry);    // Do not check safety (already checked)
        rt->addRoutingProtocolRoute(ansaRtEntry);

        rtableChanged = true;
    }

    return rtableChanged;
}

bool EigrpIpv6Pdm::setReplyStatusTable(EigrpRoute<Ipv6Address> *route, EigrpRouteSource<Ipv6Address> *source, bool forcePoisonRev, int *neighCount, int *stubCount)
{
    int neighTotalCount = eigrpNt->getNumNeighbors();
    EigrpNeighbor<Ipv6Address> *neigh;
    EigrpInterface *eigrpIface = nullptr;

    for (int i = 0; i < neighTotalCount; i++) {
        neigh = eigrpNt->getNeighbor(i);
        if ((eigrpIface = eigrpIft->findInterfaceById(neigh->getIfaceId())) == nullptr)
            continue; // The interface has been removed

        // Apply stub routing
        if (!this->eigrpStubEnabled) {
            if (neigh->isStubEnabled()) {
                (*stubCount)++;
                (*neighCount)++;
                continue; // Non stub router can not send Query to stub router
            }

            if (this->eigrpNt->getStubCount() > 0) { // Apply Poison Reverse instead of Split Horizon rule
                forcePoisonRev = true;
            }
        }

        // Apply Split Horizon and Poison Reverse
        if (eigrpIface->isSplitHorizonEn() && applySplitHorizon(eigrpIface, source, route)) {
            if (forcePoisonRev) { // Apply Poison Reverse instead of Split Horizon
                route->setReplyStatus(neigh->getNeighborId());
            }
            else
                continue; // Do not send route
        }
        else
            route->setReplyStatus(neigh->getNeighborId());

        (*neighCount)++;
    }

    return route->getReplyStatusSum() > 0;
}

bool EigrpIpv6Pdm::hasNeighborForUpdate(EigrpRouteSource<Ipv6Address> *source)
{
    int ifaceCnt = this->eigrpIft->getNumInterfaces();
    EigrpInterface *eigrpIface;

    for (int i = 0; i < ifaceCnt; i++) {
        eigrpIface = eigrpIft->getInterface(i);

        // Do not apply Split Horizon rule
        if (eigrpIface != nullptr && eigrpIface->getNumOfNeighbors() > 0) {
            return true;
        }
    }

    return false;
}

void EigrpIpv6Pdm::setDelayedRemove(int neighId, EigrpRouteSource<Ipv6Address> *src)
{
    EigrpNeighbor<Ipv6Address> *neigh = eigrpNt->findNeighborById(neighId);

    ASSERT(neigh != nullptr);
    neigh->setRoutesForDeletion(true);
    src->setDelayedRemove(neighId);
    src->setValid(true); // Can not be invalid

#ifdef EIGRP_DEBUG
    EV_DEBUG << "DUAL: route via " << src->getNextHop() << " will be removed from TT after receiving Ack from neighbor" << endl;
#endif
}

void EigrpIpv6Pdm::sendUpdateToStubs(EigrpRouteSource<Ipv6Address> *succ, EigrpRouteSource<Ipv6Address> *oldSucc, EigrpRoute<Ipv6Address> *route)
{
    if (!this->eigrpStubEnabled && eigrpNt->getStubCount() > 0) {
        if (succ == nullptr) { // Send old successor
            // Route will be removed after router receives Ack from neighbor
            if (oldSucc->isUnreachable()) {
                route->setUpdateSent(true);
                route->setNumSentMsgs(route->getNumSentMsgs() + 1);
            }
            sendUpdate(IEigrpPdm::STUB_RECEIVER, route, oldSucc, true, "notify stubs about change");
        }
        else // Send successor
            sendUpdate(IEigrpPdm::STUB_RECEIVER, route, succ, true, "notify stubs about change");
    }
}

Ipv6Route *EigrpIpv6Pdm::findRoute(const Ipv6Address& prefix,
        int prefixLength) {
    Ipv6Route *route = nullptr;

    for (int i = 0; i < rt->getNumRoutes(); i++) {
        auto it = rt->getRoute(i);

        if (it->getDestPrefix() == prefix && it->getPrefixLength() == prefixLength) {
            route = it;
            break;
        }
    }

    return route;
}

Ipv6Route *EigrpIpv6Pdm::findRoute(const Ipv6Address& prefix, int prefixLength,
        const Ipv6Address& nexthop) {
    Ipv6Route *route = nullptr;
    for (int i = 0; i < rt->getNumRoutes(); i++) {
        auto it = rt->getRoute(i);
        if (it->getDestPrefix() == prefix && it->getPrefixLength() == prefixLength && it->getNextHop() == nexthop) {
            route = it;
            break;
        }
    }
    return route;
}

bool EigrpIpv6Pdm::addNetPrefix(const Ipv6Address& network, const short int prefixLen, const int ifaceId)
{
    PrefixVector::iterator it;

    for (it = netPrefixes.begin(); it != netPrefixes.end(); ++it) { // through all known prefixes search same prefix
        if (it->network == network && it->prefixLength == prefixLen) { // found same prefix
            if (it->ifaceId == ifaceId) { // belonging to same interface = more than one IPv6 addresses from same prefix on interface = ok -> already added
                return true;
            }
            else { // same prefix on different interfaces = bad -> do not add
                return false;
            }
        }
    }

    // Add new prefix
    IPv6netPrefix newprefix;
    newprefix.network = network;
    newprefix.prefixLength = prefixLen;
    newprefix.ifaceId = ifaceId;
    this->netPrefixes.push_back(newprefix);

//    EV_DEBUG << "Added prefix: " << this->netPrefixes.back().network << "/" << this->netPrefixes.back().prefixLength << " on iface " << this->netPrefixes.back().ifaceId << endl;

    return true;
}

} // namespace eigrp

} // namespace inet

