//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceiverBase.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceptionBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmissionBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"

namespace inet {
namespace physicallayer {

FlatReceiverBase::FlatReceiverBase() :
    NarrowbandReceiverBase(),
    errorModel(nullptr),
    energyDetection(W(NaN)),
    sensitivity(W(NaN))
{
}

void FlatReceiverBase::initialize(int stage)
{
    NarrowbandReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        errorModel = dynamic_cast<IErrorModel *>(getSubmodule("errorModel"));
        energyDetection = mW(math::dBmW2mW(par("energyDetection")));
        sensitivity = mW(math::dBmW2mW(par("sensitivity")));
    }
}

std::ostream& FlatReceiverBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(errorModel, printFieldToString(errorModel, level + 1, evFlags));
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(energyDetection)
               << EV_FIELD(sensitivity);
    return NarrowbandReceiverBase::printToStream(stream, level);
}

const IListeningDecision *FlatReceiverBase::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    const IRadio *receiver = listening->getReceiver();
    const IRadioMedium *radioMedium = receiver->getMedium();
    const IAnalogModel *analogModel = radioMedium->getAnalogModel();
    const INoise *noise = analogModel->computeNoise(listening, interference);
    const NarrowbandNoiseBase *narrowbandNoise = check_and_cast<const NarrowbandNoiseBase *>(noise);
    W maxPower = narrowbandNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    bool isListeningPossible = maxPower >= energyDetection;
    delete noise;
    EV_DEBUG << "Computing whether listening is possible: maximum power = " << maxPower << ", energy detection = " << energyDetection << " -> listening is " << (isListeningPossible ? "possible" : "impossible") << endl;
    return new ListeningDecision(listening, isListeningPossible);
}

// TODO this is not purely functional, see interface comment
bool FlatReceiverBase::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    if (!NarrowbandReceiverBase::computeIsReceptionPossible(listening, reception, part))
        return false;
    else {
        const FlatReceptionBase *flatReception = check_and_cast<const FlatReceptionBase *>(reception);
        W minReceptionPower = flatReception->computeMinPower(reception->getStartTime(part), reception->getEndTime(part));
        ASSERT(W(0.0) <= minReceptionPower);
        bool isReceptionPossible = minReceptionPower >= sensitivity;
        EV_DEBUG << "Computing whether reception is possible" << EV_FIELD(minReceptionPower) << EV_FIELD(sensitivity) << " -> reception is " << (isReceptionPossible ? "possible" : "impossible") << endl;
        return isReceptionPossible;
    }
}

bool FlatReceiverBase::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    if (!SnirReceiverBase::computeIsReceptionSuccessful(listening, reception, part, interference, snir))
        return false;
    else if (!errorModel)
        return true;
    else {
        double packetErrorRate = errorModel->computePacketErrorRate(snir, part);
        if (packetErrorRate == 0.0)
            return true;
        else if (packetErrorRate == 1.0)
            return false;
        else {
            ASSERT(0.0 < packetErrorRate && packetErrorRate < 1.0);
            return dblrand() > packetErrorRate;
        }
    }
}

const IReceptionResult *FlatReceiverBase::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    auto receptionResult = NarrowbandReceiverBase::computeReceptionResult(listening, reception, interference, snir, decisions);
    auto errorRateInd = const_cast<Packet *>(receptionResult->getPacket())->addTagIfAbsent<ErrorRateInd>();
    errorRateInd->setPacketErrorRate(errorModel ? errorModel->computePacketErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
    errorRateInd->setBitErrorRate(errorModel ? errorModel->computeBitErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
    errorRateInd->setSymbolErrorRate(errorModel ? errorModel->computeSymbolErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE) : 0.0);
    return receptionResult;
}

Packet *FlatReceiverBase::computeReceivedPacket(const ISnir *snir, bool isReceptionSuccessful) const
{
    if (errorModel == nullptr || isReceptionSuccessful)
        return ReceiverBase::computeReceivedPacket(snir, isReceptionSuccessful);
    else
        return errorModel->computeCorruptedPacket(snir);
}

} // namespace physicallayer
} // namespace inet

