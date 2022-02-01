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
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

const Ipv4Address PimBase::ALL_PIM_ROUTERS_MCAST("224.0.0.13");

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
            pimInterface->getInterfacePtr()->getProtocolDataForUpdate<Ipv4InterfaceData>()->joinMulticastGroup(ALL_PIM_ROUTERS_MCAST);
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
    msg->setCrcMode(pimModule->getCrcMode());
    Pim::insertCrc(msg);
    pk->insertAtFront(msg);
    pk->addTag<PacketProtocolTag>()->setProtocol(&Protocol::pim);
    pk->addTag<InterfaceReq>()->setInterfaceId(pimInterface->getInterfaceId());
    pk->addTag<DispatchProtocolInd>()->setProtocol(&Protocol::pim);
    pk->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    pk->addTag<L3AddressReq>()->setDestAddress(ALL_PIM_ROUTERS_MCAST);
    pk->addTag<HopLimitReq>()->setHopLimit(1);

    emit(sentHelloPkSignal, pk);

    send(pk, "ipOut");
}

void PimBase::processHelloPacket(Packet *packet)
{
    int interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();

    Ipv4Address address = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv4();
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

std::ostream& operator<<(std::ostream& out, const PimBase::SourceAndGroup& sourceGroup)
{
    out << "(source: " << (sourceGroup.source.isUnspecified() ? "*" : sourceGroup.source.str()) << ", "
        << "group: " << (sourceGroup.group.isUnspecified() ? "*" : sourceGroup.group.str()) << ")";
    return out;
}

} // namespace inet

