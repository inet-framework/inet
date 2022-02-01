//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandReceiverBase.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandReceptionBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandTransmissionBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"

namespace inet {

namespace physicallayer {

NarrowbandReceiverBase::NarrowbandReceiverBase() :
    SnirReceiverBase(),
    modulation(nullptr),
    centerFrequency(Hz(NaN)),
    bandwidth(Hz(NaN))
{
}

void NarrowbandReceiverBase::initialize(int stage)
{
    SnirReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        modulation = ApskModulationBase::findModulation(par("modulation"));
        centerFrequency = Hz(par("centerFrequency"));
        bandwidth = Hz(par("bandwidth"));
    }
}

std::ostream& NarrowbandReceiverBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(modulation, printFieldToString(modulation, level + 1, evFlags))
               << EV_FIELD(centerFrequency)
               << EV_FIELD(bandwidth);
    return SnirReceiverBase::printToStream(stream, level);
}

const IListening *NarrowbandReceiverBase::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, centerFrequency, bandwidth);
}

bool NarrowbandReceiverBase::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    // TODO check if modulation matches?
    const NarrowbandTransmissionBase *narrowbandTransmission = check_and_cast<const NarrowbandTransmissionBase *>(transmission);
    return centerFrequency == narrowbandTransmission->getCenterFrequency() && bandwidth >= narrowbandTransmission->getBandwidth();
}

// TODO this is not purely functional, see interface comment
bool NarrowbandReceiverBase::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const NarrowbandReceptionBase *narrowbandReception = check_and_cast<const NarrowbandReceptionBase *>(reception);
    if (bandListening->getCenterFrequency() != narrowbandReception->getCenterFrequency() || bandListening->getBandwidth() < narrowbandReception->getBandwidth()) {
        EV_DEBUG << "Computing whether reception is possible: listening and reception bands are different -> reception is impossible" << endl;
        return false;
    }
    else
        return true;
}

const IReceptionDecision *NarrowbandReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const NarrowbandReceptionBase *narrowbandReception = check_and_cast<const NarrowbandReceptionBase *>(reception);
    if (bandListening->getCenterFrequency() == narrowbandReception->getCenterFrequency() && bandListening->getBandwidth() >= narrowbandReception->getBandwidth())
        return SnirReceiverBase::computeReceptionDecision(listening, reception, part, interference, snir);
    else
        return new ReceptionDecision(reception, part, false, false, false);
}

} // namespace physicallayer

} // namespace inet

