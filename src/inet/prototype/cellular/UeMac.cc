//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/prototype/cellular/UeMac.h"

#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/prototype/cellular/CellularTransmitter.h"
#include "inet/prototype/cellular/EffectiveSnirInd_m.h"
#include "inet/prototype/cellular/GnbMac.h"
#include "inet/prototype/cellular/IdealControlChannel.h"
#include "inet/prototype/cellular/TransportBlock.h"
#include "inet/prototype/cellular/TransportBlockHeader_m.h"

namespace inet {

namespace cellular {

Define_Module(UeMac);

simsignal_t UeMac::transportBlockReceivedSignal = cComponent::registerSignal("transportBlockReceived");
simsignal_t UeMac::transportBlockFailedSignal = cComponent::registerSignal("transportBlockFailed");
simsignal_t UeMac::effectiveSnirMeasuredSignal = cComponent::registerSignal("effectiveSnirMeasured");

void UeMac::initialize(int stage)
{
    SimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        controlChannel = getModuleFromPar<IdealControlChannel>(par("controlChannelModule"), this);
        ueIndex = controlChannel->registerUeMac(this);
        maxTransmissionPower = W(par("maxTransmissionPower"));
        carriers.resize(gateSize("lowerLayerOut"));
        for (int i = 0; i < (int)carriers.size(); i++)
            carriers[i].radio = check_and_cast<IRadio *>(gate("lowerLayerOut", i)->getPathEndGate()->getOwnerModule());
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        for (auto& carrier : carriers)
            carrier.transmitter = check_and_cast<const CellularTransmitter *>(carrier.radio->getTransmitter());
    }
    else if (stage == INITSTAGE_NETWORK_LAYER)
        attachToBestCell();
}

void UeMac::attachToBestCell()
{
    // the sectors initialize their carriers in the link layer stage, so this comes after it
    const Coord& position = carriers.at(0).radio->getAntenna()->getMobility()->getCurrentPosition();
    double bestMetric = -INFINITY;
    for (auto gnbMac : controlChannel->getGnbMacs()) {
        double metric = gnbMac->computeAttachmentMetric(position);
        if (metric > bestMetric) {
            bestMetric = metric;
            servingGnbMac = gnbMac;
        }
    }
    if (servingGnbMac == nullptr)
        throw cRuntimeError("No base station sector to attach to");
    EV_INFO << "Attaching to the best sector" << EV_FIELD(sector, servingGnbMac->getFullPath()) << EV_FIELD(metric, bestMetric) << EV_ENDL;
    servingGnbMac->attach(this);
}

void UeMac::handleMessage(cMessage *message)
{
    if (message->isSelfMessage()) {
        auto it = pendingGrants.find(simTime());
        ASSERT(it != pendingGrants.end());
        transmitGrants(it->second);
        pendingGrants.erase(it);
        delete message;
    }
    else
        handleLowerPacket(check_and_cast<Packet *>(message));
}

void UeMac::grantUplink(const UplinkGrant& grant)
{
    Enter_Method("grantUplink");
    if (grant.startTime <= simTime())
        throw cRuntimeError("The uplink grant starts at %s, not after the current time", grant.startTime.str().c_str());
    auto& grants = pendingGrants[grant.startTime];
    if (grants.empty())
        scheduleAt(grant.startTime, new cMessage("uplinkTransmission"));
    grants.push_back(grant);
}

void UeMac::transmitGrants(const std::vector<UplinkGrant>& grants)
{
    // each carrier asks for its maximum power; if the sum exceeds the maximum of the UE, all of
    // them are scaled down equally, a simplified form of the uplink power sharing of TS 38.213
    W requestedPower = W(0);
    for (const auto& grant : grants)
        requestedPower += carriers.at(grant.carrierIndex).transmitter->getMaxPower();
    double scale = std::min(1.0, unit(maxTransmissionPower / requestedPower).get());
    for (const auto& grant : grants) {
        W power = carriers.at(grant.carrierIndex).transmitter->getMaxPower() * scale;
        auto packet = TransportBlock::create("UL-TB", ueIndex, grant.carrierIndex, grant.mcs, grant.prbs, grant.numSymbols, power / grant.prbs.size(), -1, grant.intendedReceiverId);
        send(packet, "lowerLayerOut", grant.carrierIndex);
    }
}

void UeMac::handleLowerPacket(Packet *packet)
{
    int carrierIndex = packet->getArrivalGate()->getIndex();
    double effectiveSnir = math::fraction2dB(packet->getTag<EffectiveSnirInd>()->getEffectiveSnir());
    emit(effectiveSnirMeasuredSignal, effectiveSnir);
    emit(packet->hasBitError() ? transportBlockFailedSignal : transportBlockReceivedSignal, packet);
    servingGnbMac->reportDownlinkSnir(ueIndex, carrierIndex, effectiveSnir);
    delete packet;
}

} // namespace cellular

} // namespace inet

