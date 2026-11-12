//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/prototype/cellular/GnbMac.h"

#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/prototype/cellular/CellularReceiver.h"
#include "inet/prototype/cellular/CellularTransmitter.h"
#include "inet/prototype/cellular/EffectiveSnirInd_m.h"
#include "inet/prototype/cellular/IdealControlChannel.h"
#include "inet/prototype/cellular/McsTable.h"
#include "inet/prototype/cellular/PanelAntenna.h"
#include "inet/prototype/cellular/SharedAntennaRef.h"
#include "inet/prototype/cellular/TransportBlock.h"
#include "inet/prototype/cellular/TransportBlockHeader_m.h"

namespace inet {

namespace cellular {

Define_Module(GnbMac);

simsignal_t GnbMac::transportBlockReceivedSignal = cComponent::registerSignal("transportBlockReceived");
simsignal_t GnbMac::transportBlockFailedSignal = cComponent::registerSignal("transportBlockFailed");
simsignal_t GnbMac::effectiveSnirMeasuredSignal = cComponent::registerSignal("effectiveSnirMeasured");

// the SNIR of a link that has no report yet, which selects the lowest MCS
static const double UNKNOWN_SNIR = -100;

GnbMac::~GnbMac()
{
    for (auto& carrier : carriers)
        cancelAndDelete(carrier.slotTimer);
}

void GnbMac::initialize(int stage)
{
    SimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        controlChannel = getModuleFromPar<IdealControlChannel>(par("controlChannelModule"), this);
        controlChannel->registerGnbMac(this);
        scheduledUesPerSlot = par("scheduledUesPerSlot");
        tddPattern = par("tddPattern").stdstringValue();
        downlinkFirstSymbol = par("downlinkFirstSymbol");
        downlinkNumSymbols = par("downlinkNumSymbols");
        uplinkNumSymbols = par("uplinkNumSymbols");
        uplinkGrantDelay = par("uplinkGrantDelay");
        bitmapAllocation = !strcmp(par("allocationType"), "bitmap");
        rbgSize = par("rbgSize");
        mcsMargin = par("mcsMargin");
        if (tddPattern.empty() || tddPattern.find_first_not_of("DSU") != std::string::npos)
            throw cRuntimeError("The TDD pattern '%s' must consist of D, S and U slots", tddPattern.c_str());
        if (downlinkFirstSymbol + downlinkNumSymbols > CarrierGrid::SYMBOLS_PER_SLOT || uplinkNumSymbols > CarrierGrid::SYMBOLS_PER_SLOT)
            throw cRuntimeError("The downlink or uplink symbols do not fit into one slot");
        if (uplinkGrantDelay < 1)
            throw cRuntimeError("The uplink grant delay must be at least one slot");
        carriers.resize(gateSize("lowerLayerOut"));
        for (int i = 0; i < (int)carriers.size(); i++) {
            carriers[i].radio = check_and_cast<IRadio *>(gate("lowerLayerOut", i)->getPathEndGate()->getOwnerModule());
            carriers[i].slotTimer = new cMessage("slot", i);
        }
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        for (auto& carrier : carriers) {
            carrier.transmitter = check_and_cast<const CellularTransmitter *>(carrier.radio->getTransmitter());
            carrier.panel = check_and_cast<const PanelAntenna *>(check_and_cast<const SharedAntennaRef *>(carrier.radio->getAntenna())->getAntenna());
            carrier.grid = carrier.transmitter->getGrid();
            auto receiver = check_and_cast<const CellularReceiver *>(carrier.radio->getReceiver());
            carrier.isTdd = receiver->getCenterFrequency() == carrier.grid.getCenterFrequency();
            scheduleAt(simTime(), carrier.slotTimer);
        }
    }
}

void GnbMac::handleMessage(cMessage *message)
{
    if (message->isSelfMessage())
        handleSlot(message->getKind());
    else
        handleLowerPacket(check_and_cast<Packet *>(message));
}

char GnbMac::getSlotType(const Carrier& carrier, long slotNumber) const
{
    return carrier.isTdd ? tddPattern[slotNumber % tddPattern.length()] : 'F';
}

std::vector<std::vector<int>> GnbMac::dividePrbs(int numPrbs, int numUes, bool interleaved) const
{
    std::vector<std::vector<int>> prbSets(numUes);
    if (interleaved) {
        for (int prb = 0; prb < numPrbs; prb++)
            prbSets[(prb / rbgSize) % numUes].push_back(prb);
    }
    else {
        for (int i = 0, prb = 0; i < numUes; i++) {
            int size = numPrbs / numUes + (i < numPrbs % numUes ? 1 : 0);
            for (int j = 0; j < size; j++)
                prbSets[i].push_back(prb++);
        }
    }
    return prbSets;
}

void GnbMac::handleSlot(int carrierIndex)
{
    auto& carrier = carriers.at(carrierIndex);
    // the special slot of TDD carries downlink only, because the dummy has no guard period
    char slotType = getSlotType(carrier, carrier.slotNumber);
    if (slotType != 'U')
        scheduleDownlink(carrierIndex);
    char grantedSlotType = getSlotType(carrier, carrier.slotNumber + uplinkGrantDelay);
    if (grantedSlotType == 'U' || grantedSlotType == 'F')
        scheduleUplink(carrierIndex, simTime() + carrier.grid.getSlotDuration() * uplinkGrantDelay);
    carrier.slotNumber++;
    scheduleAfter(carrier.grid.getSlotDuration(), carrier.slotTimer);
}

void GnbMac::scheduleDownlink(int carrierIndex)
{
    auto& carrier = carriers.at(carrierIndex);
    if (ues.empty())
        return;
    int numUes = std::min<int>(scheduledUesPerSlot, ues.size());
    auto prbSets = dividePrbs(carrier.grid.getNumPrbs(), numUes, bitmapAllocation);
    W powerPerPrb = carrier.transmitter->getMaxPower() / carrier.grid.getNumPrbs();
    simtime_t delay = carrier.grid.getSymbolDuration() * downlinkFirstSymbol;
    for (int i = 0; i < numUes; i++) {
        const auto& ue = ues[(carrier.nextDownlinkUe + i) % ues.size()];
        int mcs = McsTable::selectMcs(ue.downlinkSnirs[carrierIndex], mcsMargin);
        auto ueRadio = ue.ueMac->getRadio(carrierIndex);
        int beam = carrier.panel->selectBeam(ueRadio->getAntenna()->getMobility()->getCurrentPosition());
        auto packet = TransportBlock::create("DL-TB", ue.ueMac->getUeIndex(), carrierIndex, mcs, prbSets[i], downlinkNumSymbols, powerPerPrb, beam, ueRadio->getId());
        sendDelayed(packet, delay, "lowerLayerOut", carrierIndex);
    }
    carrier.nextDownlinkUe = (carrier.nextDownlinkUe + numUes) % ues.size();
}

void GnbMac::scheduleUplink(int carrierIndex, simtime_t startTime)
{
    auto& carrier = carriers.at(carrierIndex);
    if (ues.empty())
        return;
    int numUes = std::min<int>(scheduledUesPerSlot, ues.size());
    // uplink allocations are contiguous, as DFT-s-OFDM needs
    auto prbSets = dividePrbs(carrier.grid.getNumPrbs(), numUes, false);
    for (int i = 0; i < numUes; i++) {
        const auto& ue = ues[(carrier.nextUplinkUe + i) % ues.size()];
        UplinkGrant grant;
        grant.carrierIndex = carrierIndex;
        grant.startTime = startTime;
        grant.prbs = prbSets[i];
        grant.numSymbols = uplinkNumSymbols;
        grant.mcs = McsTable::selectMcs(ue.uplinkSnirs[carrierIndex], mcsMargin);
        grant.intendedReceiverId = carrier.radio->getId();
        ue.ueMac->grantUplink(grant);
    }
    carrier.nextUplinkUe = (carrier.nextUplinkUe + numUes) % ues.size();
}

void GnbMac::handleLowerPacket(Packet *packet)
{
    int carrierIndex = packet->getArrivalGate()->getIndex();
    const auto& header = packet->peekAtFront<TransportBlockHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT);
    double effectiveSnir = math::fraction2dB(packet->getTag<EffectiveSnirInd>()->getEffectiveSnir());
    ues.at(ueIndexToUe.at(header->getUeIndex())).uplinkSnirs.at(carrierIndex) = effectiveSnir;
    emit(effectiveSnirMeasuredSignal, effectiveSnir);
    emit(packet->hasBitError() ? transportBlockFailedSignal : transportBlockReceivedSignal, packet);
    delete packet;
}

double GnbMac::computeAttachmentMetric(const Coord& position) const
{
    const auto& panel = carriers.at(0).panel;
    IMobility *mobility = panel->getMobility();
    Coord direction = position - mobility->getCurrentPosition();
    Quaternion localDirection = Quaternion(mobility->getCurrentAngularPosition()).inverse() * Quaternion::rotationFromTo(Coord::X_AXIS, direction);
    double gain = panel->getGain()->computeGain(localDirection);
    return math::fraction2dB(gain) - 30 * std::log10(std::max(1.0, direction.length()));
}

void GnbMac::attach(UeMac *ueMac)
{
    Enter_Method("attach");
    if (ueMac->getNumCarriers() != (int)carriers.size())
        throw cRuntimeError("The UE has %d carriers, the sector %d", ueMac->getNumCarriers(), (int)carriers.size());
    UeState ue;
    ue.ueMac = ueMac;
    ue.downlinkSnirs.resize(carriers.size(), UNKNOWN_SNIR);
    ue.uplinkSnirs.resize(carriers.size(), UNKNOWN_SNIR);
    ueIndexToUe[ueMac->getUeIndex()] = ues.size();
    ues.push_back(ue);
}

void GnbMac::reportDownlinkSnir(int ueIndex, int carrierIndex, double snir)
{
    Enter_Method("reportDownlinkSnir");
    ues.at(ueIndexToUe.at(ueIndex)).downlinkSnirs.at(carrierIndex) = snir;
}

} // namespace cellular

} // namespace inet

