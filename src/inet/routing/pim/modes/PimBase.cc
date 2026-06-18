//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#include "inet/routing/pim/modes/PimBase.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6MulticastRoute.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"

namespace inet {

const PimBase::AssertMetric PimBase::AssertMetric::PIM_INFINITE;

simsignal_t PimBase::sentHelloPkSignal = registerSignal("sentHelloPk");
simsignal_t PimBase::rcvdHelloPkSignal = registerSignal("rcvdHelloPk");

PimBase::~PimBase()
{
    cancelAndDelete(helloTimer);
}

void PimBase::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *addressFamily = par("addressFamily");
        if (!strcmp(addressFamily, "ipv4")) {
            networkProtocol = &Protocol::ipv4;
            allPimRoutersMcast = Ipv4Address("224.0.0.13");
        }
        else if (!strcmp(addressFamily, "ipv6")) {
            networkProtocol = &Protocol::ipv6;
            allPimRoutersMcast = Ipv6Address("ff02::d");
        }
        else
            throw cRuntimeError("PimBase: invalid addressFamily '%s' (must be 'ipv4' or 'ipv6')", addressFamily);

        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);
        pimIft.reference(this, "pimInterfaceTableModule", true);
        pimNbt.reference(this, "pimNeighborTableModule", true);

        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PimBase: containing node not found.");

        hostname = host->getName();

        helloPeriod = par("helloPeriod");
        holdTime = par("holdTime");
        designatedRouterPriority = mode == PimInterface::SparseMode ? par("designatedRouterPriority") : -1;
        pimModule = check_and_cast<Pim *>(getParentModule());
    }
}

void PimBase::handleStartOperation(LifecycleOperation *operation)
{
    generationID = intrand(UINT32_MAX);

    // to receive PIM messages, join to ALL_PIM_ROUTERS multicast group
    isEnabled = false;
    for (int i = 0; i < pimIft->getNumInterfaces(); i++) {
        PimInterface *pimInterface = pimIft->getInterface(i);
        if (pimInterface->getMode() == mode) {
            joinMulticastGroup(pimInterface->getInterfacePtr(), allPimRoutersMcast);
            isEnabled = true;
        }
    }

    if (isEnabled) {
        EV_INFO << "PIM is enabled on device " << hostname << endl;
        helloTimer = new cMessage("PIM HelloTimer", HelloTimer);
        scheduleAfter(par("triggeredHelloDelay"), helloTimer);
    }
}

void PimBase::handleStopOperation(LifecycleOperation *operation)
{
    // TODO unregister IP_PROT_PIM
    cancelAndDelete(helloTimer);
    helloTimer = nullptr;
}

void PimBase::handleCrashOperation(LifecycleOperation *operation)
{
    // TODO unregister IP_PROT_PIM
    cancelAndDelete(helloTimer);
    helloTimer = nullptr;
}

void PimBase::processHelloTimer(cMessage *timer)
{
    ASSERT(timer == helloTimer);
    EV_DETAIL << "Hello Timer expired.\n";
    sendHelloPackets();
    scheduleAfter(helloPeriod, helloTimer);
}

void PimBase::sendHelloPackets()
{
    for (int i = 0; i < pimIft->getNumInterfaces(); i++) {
        PimInterface *pimInterface = pimIft->getInterface(i);
        if (pimInterface->getMode() == mode)
            sendHelloPacket(pimInterface);
    }
}

void PimBase::sendHelloPacket(PimInterface *pimInterface)
{
    EV_INFO << "Sending Hello packet on interface '" << pimInterface->getInterfacePtr()->getInterfaceName() << "'\n";

    Packet *pk = new Packet("PimHello");
    const auto& msg = makeShared<PimHello>();

    B byteLength = PIM_HEADER_LENGTH + B(6) + B(8); // HoldTime + GenerationID option

    msg->setOptionsArraySize(designatedRouterPriority < 0 ? 2 : 3);
    HoldtimeOption *holdtimeOption = new HoldtimeOption();
    holdtimeOption->setHoldTime(holdTime < 0 ? (uint16_t)0xffff : (uint16_t)holdTime);
    msg->setOptions(0, holdtimeOption);

    GenerationIdOption *genIdOption = new GenerationIdOption();
    genIdOption->setGenerationID(generationID);
    msg->setOptions(1, genIdOption);

    if (designatedRouterPriority >= 0) {
        DrPriorityOption *drPriorityOption = new DrPriorityOption();
        drPriorityOption->setPriority(designatedRouterPriority);
        msg->setOptions(2, drPriorityOption);
        byteLength += B(8);
    }

    msg->setChunkLength(byteLength);
    msg->setChecksumMode(pimModule->getChecksumMode());
    Pim::insertChecksum(msg);
    pk->insertAtFront(msg);
    pk->addTag<PacketProtocolTag>()->setProtocol(&Protocol::pim);
    pk->addTag<InterfaceReq>()->setInterfaceId(pimInterface->getInterfaceId());
    pk->addTag<DispatchProtocolInd>()->setProtocol(&Protocol::pim);
    pk->addTag<DispatchProtocolReq>()->setProtocol(networkProtocol);
    auto addresses = pk->addTag<L3AddressReq>();
    // RFC 7761: PIM-over-IPv6 messages are sourced from the interface link-local
    // address. Set it explicitly, otherwise IPv6 would fill in the (global)
    // preferred source, and neighbors would then learn this router by its global
    // address -- inconsistent with the link-local RPF next hop used for Joins/Grafts.
    if (isIpv6())
        addresses->setSrcAddress(getInterfaceAddress(pimInterface->getInterfacePtr()));
    addresses->setDestAddress(allPimRoutersMcast);
    pk->addTag<HopLimitReq>()->setHopLimit(1);

    emit(sentHelloPkSignal, pk);

    send(pk, "ipOut");
}

void PimBase::processHelloPacket(Packet *packet)
{
    int interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();

    L3Address address = packet->getTag<L3AddressInd>()->getSrcAddress();
    const auto& pimPacket = packet->peekAtFront<PimHello>();
    int version = pimPacket->getVersion();

    emit(rcvdHelloPkSignal, packet);

    // process options
    double holdTime = 3.5 * 30;
    long drPriority = -1L;
    unsigned int generationId = 0;
    for (unsigned int i = 0; i < pimPacket->getOptionsArraySize(); i++) {
        const HelloOption *option = pimPacket->getOptions(i);
        switch (option->getType()) {
            case Holdtime:
                holdTime = check_and_cast<const HoldtimeOption *>(option)->getHoldTime();
                break;
            case DRPriority:
                drPriority = check_and_cast<const DrPriorityOption *>(option)->getPriority();
                break;
            case GenerationID:
                generationId = check_and_cast<const GenerationIdOption *>(option)->getGenerationID();
                break;
            default:
                break;
        }
    }

    NetworkInterface *ie = ift->getInterfaceById(interfaceId);

    EV_INFO << "Received PIM Hello from neighbor: interface=" << ie->getInterfaceName() << " address=" << address << "\n";

    PimNeighbor *neighbor = pimNbt->findNeighbor(interfaceId, address);
    if (neighbor)
        pimNbt->restartLivenessTimer(neighbor, holdTime);
    else {
        neighbor = new PimNeighbor(ie, address, version);
        pimNbt->addNeighbor(neighbor, holdTime);

        // TODO If a Hello message is received from a new neighbor, the
        // receiving router SHOULD send its own Hello message after a random
        // delay between 0 and Triggered_Hello_Delay.
    }

    neighbor->setGenerationId(generationId);
    neighbor->setDRPriority(drPriority);

    delete packet;
}

bool PimBase::AssertMetric::operator==(const AssertMetric& other) const
{
    return rptBit == other.rptBit && preference == other.preference &&
           metric == other.metric && address == other.address;
}

bool PimBase::AssertMetric::operator!=(const AssertMetric& other) const
{
    return rptBit != other.rptBit || preference != other.preference ||
           metric != other.metric || address != other.address;
}

bool PimBase::AssertMetric::operator<(const AssertMetric& other) const
{
    if (isInfinite())
        return false;
    if (other.isInfinite())
        return true;
    if (rptBit != other.rptBit)
        return rptBit < other.rptBit;
    if (preference != other.preference)
        return preference < other.preference;
    if (metric != other.metric)
        return metric < other.metric;
    return address > other.address;
}

L3Address PimBase::getInterfaceAddress(NetworkInterface *ie) const
{
    // RFC 7761/RFC 3973 over IPv6: PIM uses the interface's link-local address as
    // its own address on the link -- as the Hello source and as the Encoded-Unicast
    // upstream-neighbor/assert/originator address. The link-local address is also
    // what unicast next hops resolve to (ND), so the RPF neighbor learned from a
    // unicast route matches the neighbor's PIM address. (For IPv4 it is the
    // interface IPv4 address, unchanged.)
    if (isIpv6())
        return ie->getProtocolData<Ipv6InterfaceData>()->getLinkLocalAddress();
    else
        return ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
}

void PimBase::joinMulticastGroup(NetworkInterface *ie, const L3Address& group)
{
    if (isIpv6())
        ie->getProtocolDataForUpdate<Ipv6InterfaceData>()->joinMulticastGroup(group.toIpv6());
    else
        ie->getProtocolDataForUpdate<Ipv4InterfaceData>()->joinMulticastGroup(group.toIpv4());
}

bool PimBase::hasMulticastListener(NetworkInterface *ie, const L3Address& group) const
{
    if (isIpv6())
        return ie->getProtocolData<Ipv6InterfaceData>()->hasMulticastListener(group.toIpv6());
    else
        return ie->getProtocolData<Ipv4InterfaceData>()->hasMulticastListener(group.toIpv4());
}

bool PimBase::isMemberOfMulticastGroup(NetworkInterface *ie, const L3Address& group) const
{
    if (isIpv6()) {
        auto data = ie->findProtocolData<Ipv6InterfaceData>();
        return data != nullptr && data->isMemberOfMulticastGroup(group.toIpv6());
    }
    else {
        auto data = ie->findProtocolData<Ipv4InterfaceData>();
        return data != nullptr && data->isMemberOfMulticastGroup(group.toIpv4());
    }
}

IMulticastRoute *PimBase::createMulticastRoute()
{
    return isIpv6() ? static_cast<IMulticastRoute *>(new Ipv6MulticastRoute())
                    : static_cast<IMulticastRoute *>(new Ipv4MulticastRoute());
}

NetworkInterface *PimBase::getInInterface(IMulticastRoute *route)
{
    if (auto ipv6Route = dynamic_cast<Ipv6MulticastRoute *>(route)) {
        auto in = ipv6Route->getInInterface();
        return in ? in->getInterface() : nullptr;
    }
    auto ipv4Route = check_and_cast<Ipv4MulticastRoute *>(route);
    auto in = ipv4Route->getInInterface();
    return in ? in->getInterface() : nullptr;
}

bool PimBase::hasOutInterface(IMulticastRoute *route, const NetworkInterface *ie)
{
    if (auto ipv6Route = dynamic_cast<Ipv6MulticastRoute *>(route))
        return ipv6Route->hasOutInterface(ie);
    return check_and_cast<Ipv4MulticastRoute *>(route)->hasOutInterface(ie);
}

unsigned int PimBase::getAdminDist(IRoute *route)
{
    if (auto ipv6Route = dynamic_cast<Ipv6Route *>(route))
        return ipv6Route->getAdminDist();
    return check_and_cast<Ipv4Route *>(route)->getAdminDist();
}

IMulticastRoute *PimBase::findMulticastRoute(L3Address group, L3Address source)
{
    int numRoutes = rt->getNumMulticastRoutes();
    for (int i = 0; i < numRoutes; i++) {
        IMulticastRoute *route = rt->getMulticastRoute(i);
        if (route->getSource() == this && route->getMulticastGroupAsGeneric() == group && route->getOriginAsGeneric() == source)
            return route;
    }
    return nullptr;
}

bool PimBase::isRoutableMulticastSource(const L3Address& srcAddr) const
{
    // An IPv6 multicast packet sourced from a link-local address (e.g. an MLD
    // Multicast Listener Report, sent from fe80::/10 to the group it reports)
    // does not define a routable (S,G): link-local sources are not reachable by
    // a unicast route, so no RPF tree can be built toward them (RFC 4291). IPv4
    // has no such case here, so it is always treated as routable, preserving the
    // existing IPv4 behavior.
    if (isIpv6())
        return !srcAddr.toIpv6().isLinkLocal();
    return true;
}

bool PimBase::isRoutableMulticastGroup(const L3Address& group) const
{
    // PIM builds distribution trees only for routable multicast groups. Link-local
    // (and interface-local) multicast control groups -- IPv6 scope <= 2, ff02::/16
    // such as the all-routers group ff02::2 and all-PIM-routers ff02::d that the
    // stack joins on every router interface, or IPv4 224.0.0.0/24 -- are never
    // managed by PIM and must be ignored when learned as local memberships.
    // (Note: for IPv6 multicast, Ipv6Address::getScope() returns MULTICAST, so the
    // scope nibble from getMulticastScope() is what distinguishes link-local.)
    if (isIpv6())
        return group.toIpv6().getMulticastScope() > 2;
    return !group.toIpv4().isLinkLocalMulticast();
}

bool PimBase::isSsmGroup(const L3Address& group) const
{
    // RFC 4607 source-specific multicast range: IPv4 232.0.0.0/8, IPv6 FF3x::/32.
    return isIpv6() ? group.toIpv6().isSsm() : group.toIpv4().isSsm();
}

void PimBase::getMulticastPacketAddresses(cObject *obj, L3Address& srcAddr, L3Address& destAddr, unsigned short& ttl) const
{
    if (isIpv6()) {
        auto ipv6Header = check_and_cast<const Ipv6Header *>(obj);
        srcAddr = ipv6Header->getSrcAddress();
        destAddr = ipv6Header->getDestAddress();
        ttl = ipv6Header->getHopLimit();
    }
    else {
        auto ipv4Header = check_and_cast<const Ipv4Header *>(obj);
        srcAddr = ipv4Header->getSrcAddress();
        destAddr = ipv4Header->getDestAddress();
        ttl = ipv4Header->getTimeToLive();
    }
}

void PimBase::getMulticastGroupInfo(cObject *obj, NetworkInterface *& ie, L3Address& groupAddress) const
{
    if (isIpv6()) {
        auto info = check_and_cast<const Ipv6MulticastGroupInfo *>(obj);
        ie = info->ie;
        groupAddress = info->groupAddress;
    }
    else {
        auto info = check_and_cast<const Ipv4MulticastGroupInfo *>(obj);
        ie = info->ie;
        groupAddress = info->groupAddress;
    }
}

void PimBase::getMulticastListenerSources(cObject *obj, NetworkInterface *& ie, L3Address& groupAddress, McastSourceFilterMode& filterMode, std::vector<L3Address>& sources) const
{
    sources.clear();
    if (isIpv6()) {
        auto info = check_and_cast<const Ipv6MulticastGroupSourceInfo *>(obj);
        ie = info->ie;
        groupAddress = info->groupAddress;
        filterMode = info->sourceList.filterMode;
        for (auto& source : info->sourceList.sources)
            sources.push_back(source);
    }
    else {
        auto info = check_and_cast<const Ipv4MulticastGroupSourceInfo *>(obj);
        ie = info->ie;
        groupAddress = info->groupAddress;
        filterMode = info->sourceList.filterMode;
        for (auto& source : info->sourceList.sources)
            sources.push_back(source);
    }
}

std::ostream& operator<<(std::ostream& out, const PimBase::SourceAndGroup& sourceGroup)
{
    out << "(source: " << (sourceGroup.source.isUnspecified() ? "*" : sourceGroup.source.str()) << ", "
        << "group: " << (sourceGroup.group.isUnspecified() ? "*" : sourceGroup.group.str()) << ")";
    return out;
}

} // namespace inet

