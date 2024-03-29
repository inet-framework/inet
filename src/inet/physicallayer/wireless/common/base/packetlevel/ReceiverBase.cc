//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/ReceiverBase.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionResult.h"
#include "inet/physicallayer/wireless/common/signal/Interference.h"

namespace inet {
namespace physicallayer {

void ReceiverBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        ignoreInterference = par("ignoreInterference");
}

std::ostream& ReceiverBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_INFO)
        stream << (ignoreInterference ? ", ignoring interference" : ", considering interference");
    return stream;
}

bool ReceiverBase::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    return true;
}

bool ReceiverBase::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    return true;
}

bool ReceiverBase::computeIsReceptionAttempted(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const
{
    if (!computeIsReceptionPossible(listening, reception, part))
        return false;
    else if (simTime() == reception->getStartTime(part)) {
        // TODO isn't there a better way for this optimization? see also in RadioMedium::isReceptionAttempted
        auto transmission = reception->getReceiverRadio()->getReceptionInProgress();
        return transmission == nullptr || transmission == reception->getTransmission();
    }
    else {
        // determining whether the reception is attempted or not for the future
        auto radio = reception->getReceiverRadio();
        auto radioMedium = radio->getMedium();
        auto interferingReceptions = interference->getInterferingReceptions();
        for (auto interferingReception : *interferingReceptions) {
            auto isPrecedingReception = interferingReception->getStartTime() < reception->getStartTime() ||
                (interferingReception->getStartTime() == reception->getStartTime() &&
                 interferingReception->getTransmission()->getId() < reception->getTransmission()->getId());
            if (isPrecedingReception) {
                auto interferingTransmission = interferingReception->getTransmission();
                if (interferingReception->getStartTime() <= simTime()) {
                    if (radio->getReceptionInProgress() == interferingTransmission)
                        return false;
                }
                else if (radioMedium->isReceptionAttempted(radio, interferingTransmission, part))
                    return false;
            }
        }
        return true;
    }
}

const IReceptionDecision *ReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    auto isReceptionPossible = computeIsReceptionPossible(listening, reception, part);
    auto isReceptionAttempted = isReceptionPossible && computeIsReceptionAttempted(listening, reception, part, interference);
    auto isReceptionSuccessful = isReceptionAttempted && computeIsReceptionSuccessful(listening, reception, part, interference, snir);
    return new ReceptionDecision(reception, part, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful);
}

const IReceptionResult *ReceiverBase::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    bool isReceptionSuccessful = true;
    for (auto decision : *decisions)
        isReceptionSuccessful &= decision->isReceptionSuccessful();
    auto packet = computeReceivedPacket(snir, isReceptionSuccessful);
    auto signalPower = computeSignalPower(listening, snir, interference);
    if (!std::isnan(signalPower.get())) {
        auto signalPowerInd = packet->addTagIfAbsent<SignalPowerInd>();
        signalPowerInd->setPower(signalPower);
    }
    auto snirInd = packet->addTagIfAbsent<SnirInd>();
    snirInd->setMinimumSnir(snir->getMin());
    snirInd->setMaximumSnir(snir->getMax());
    snirInd->setAverageSnir(snir->getMean());
    auto signalTimeInd = packet->addTagIfAbsent<SignalTimeInd>();
    signalTimeInd->setStartTime(reception->getStartTime());
    signalTimeInd->setEndTime(reception->getEndTime());
    if (snir->getMax() == 0) {
        auto errorRateInd = packet->addTagIfAbsent<ErrorRateInd>();
        errorRateInd->setSymbolErrorRate(1);
        errorRateInd->setBitErrorRate(1);
        errorRateInd->setPacketErrorRate(1);
    }
    else if (snir->getMin() == INFINITY) {
        auto errorRateInd = packet->addTagIfAbsent<ErrorRateInd>();
        errorRateInd->setSymbolErrorRate(0);
        errorRateInd->setBitErrorRate(0);
        errorRateInd->setPacketErrorRate(0);
    }
    return new ReceptionResult(reception, decisions, packet);
}

W ReceiverBase::computeSignalPower(const IListening *listening, const ISnir *snir, const IInterference *interference) const
{
    if (!dynamic_cast<const NarrowbandNoiseBase *>(snir->getNoise()))
        return W(0);
    else {
        auto analogModel = snir->getReception()->getTransmission()->getMedium()->getAnalogModel();
        auto signalPlusNoise = check_and_cast<const NarrowbandNoiseBase *>(analogModel->computeNoise(snir->getReception(), snir->getNoise()));
        auto signalPower = signalPlusNoise == nullptr ? W(NaN) : signalPlusNoise->computeMinPower(listening->getStartTime(), listening->getEndTime());
        delete signalPlusNoise;
        return signalPower;
    }
}

Packet *ReceiverBase::computeReceivedPacket(const ISnir *snir, bool isReceptionSuccessful) const
{
    auto transmittedPacket = snir->getReception()->getTransmission()->getPacket();
    auto receivedPacket = transmittedPacket->dup();
    receivedPacket->clearTags();
    receivedPacket->addTag<PacketProtocolTag>()->setProtocol(transmittedPacket->getTag<PacketProtocolTag>()->getProtocol());
    if (!isReceptionSuccessful)
        receivedPacket->setBitError(true);
    return receivedPacket;
}

} // namespace physicallayer
} // namespace inet

