//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/generic/GenericReceiver.h"

#include "inet/physicallayer/wireless/generic/GenericTransmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskNoise.h"
#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskReceptionAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INoise.h"

namespace inet {
namespace physicallayer {

Define_Module(GenericReceiver);

void GenericReceiver::initialize(int stage)
{
    SnirReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        ignoreInterference = par("ignoreInterference");
        energyDetection = mW(math::dBmW2mW(par("energyDetection")));
    }
}

std::ostream& GenericReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "GenericReceiver";
    if (level <= PRINT_LEVEL_INFO)
        stream << (ignoreInterference ? ", ignoring interference" : ", considering interference");
    return stream;
}

bool GenericReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto genericTransmission = dynamic_cast<const GenericTransmission *>(reception->getTransmission());
    return genericTransmission && getAnalogModel()->computeIsReceptionPossible(listening, reception, W(NaN));
}

bool GenericReceiver::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    return ignoreInterference || SnirReceiverBase::computeIsReceptionSuccessful(listening, reception, part, interference, snir);
}

const IListening *GenericReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const
{
    return getAnalogModel()->createListening(radio, startTime, endTime, startPosition, endPosition, Hz(NaN), Hz(NaN));
}

const IListeningDecision *GenericReceiver::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    const INoise *noise = listening->getReceiver()->getMedium()->getAnalogModel()->computeNoise(listening, interference);
    bool isListeningPossible = noise->computeMaxPower(listening->getStartTime(), listening->getEndTime()) > energyDetection;
    delete noise;

    return new ListeningDecision(listening, isListeningPossible);
}


} // namespace physicallayer
} // namespace inet

