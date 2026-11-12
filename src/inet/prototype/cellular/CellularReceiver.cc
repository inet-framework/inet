//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/prototype/cellular/CellularReceiver.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/prototype/cellular/CellularErrorModel.h"
#include "inet/prototype/cellular/CellularTransmission.h"
#include "inet/prototype/cellular/EffectiveSnirInd_m.h"

namespace inet {

namespace cellular {

Define_Module(CellularReceiver);

std::ostream& CellularReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "CellularReceiver";
    return FlatReceiverBase::printToStream(stream, level);
}

bool CellularReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto cellularTransmission = check_and_cast<const CellularTransmission *>(transmission);
    return cellularTransmission->getIntendedReceiverId() == listening->getReceiverRadio()->getId() &&
           FlatReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool CellularReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    return computeIsReceptionPossible(listening, reception->getTransmission()) &&
           FlatReceiverBase::computeIsReceptionPossible(listening, reception, part);
}

bool CellularReceiver::computeIsReceptionAttempted(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const
{
    return computeIsReceptionPossible(listening, reception, part);
}

bool CellularReceiver::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    // the SNIR threshold of the base class applies to the whole band, most of which is outside
    // the allocation, so only the error model decides
    double blockErrorRate = errorModel->computePacketErrorRate(snir, part);
    return dblrand() >= blockErrorRate;
}

const IReceptionResult *CellularReceiver::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    // NarrowbandReceiverBase, not FlatReceiverBase, so that the effective SNIR is computed once
    auto receptionResult = NarrowbandReceiverBase::computeReceptionResult(listening, reception, interference, snir, decisions);
    auto cellularErrorModel = check_and_cast<const CellularErrorModel *>(errorModel);
    auto transmission = check_and_cast<const CellularTransmission *>(reception->getTransmission());
    double effectiveSnir = cellularErrorModel->computeEffectiveSnir(snir);
    auto packet = const_cast<Packet *>(receptionResult->getPacket());
    packet->addTagIfAbsent<EffectiveSnirInd>()->setEffectiveSnir(effectiveSnir);
    auto errorRateInd = packet->addTagIfAbsent<ErrorRateInd>();
    errorRateInd->setPacketErrorRate(cellularErrorModel->computeBlockErrorRate(effectiveSnir, transmission->getMcs()));
    return receptionResult;
}

} // namespace cellular

} // namespace inet

