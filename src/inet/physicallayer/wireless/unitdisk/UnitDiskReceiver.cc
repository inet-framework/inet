//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskReceiver.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskListening.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskNoise.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskReception.h"

namespace inet {
namespace physicallayer {

Define_Module(UnitDiskReceiver);

UnitDiskReceiver::UnitDiskReceiver() :
    ReceiverBase(),
    ignoreInterference(false)
{
}

void UnitDiskReceiver::initialize(int stage)
{
    ReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        ignoreInterference = par("ignoreInterference");
}

std::ostream& UnitDiskReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskReceiver";
    if (level <= PRINT_LEVEL_INFO)
        stream << (ignoreInterference ? ", ignoring interference" : ", considering interference");
    return stream;
}

bool UnitDiskReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto power = check_and_cast<const UnitDiskReception *>(reception)->getPower();
    return power == UnitDiskReception::POWER_RECEIVABLE;
}

bool UnitDiskReceiver::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    auto power = check_and_cast<const UnitDiskReception *>(reception)->getPower();
    if (power == UnitDiskReception::POWER_RECEIVABLE) {
        if (ignoreInterference)
            return true;
        else {
            auto startTime = reception->getStartTime(part);
            auto endTime = reception->getEndTime(part);
            auto interferingReceptions = interference->getInterferingReceptions();
            for (auto interferingReception : *interferingReceptions) {
                auto interferingPower = check_and_cast<const UnitDiskReception *>(interferingReception)->getPower();
                if (interferingPower >= UnitDiskReception::POWER_INTERFERING && startTime <= interferingReception->getEndTime() && endTime >= interferingReception->getStartTime())
                    return false;
            }
            return true;
        }
    }
    else
        return false;
}

const IListening *UnitDiskReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const
{
    return new UnitDiskListening(radio, startTime, endTime, startPosition, endPosition);
}

const IListeningDecision *UnitDiskReceiver::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    auto interferingReceptions = interference->getInterferingReceptions();
    for (auto interferingReception : *interferingReceptions) {
        auto interferingPower = check_and_cast<const UnitDiskReception *>(interferingReception)->getPower();
        if (interferingPower != UnitDiskReception::POWER_UNDETECTABLE)
            return new ListeningDecision(listening, true);
    }
    return new ListeningDecision(listening, false);
}

const IReceptionResult *UnitDiskReceiver::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    auto noise = check_and_cast_nullable<const UnitDiskNoise *>(snir->getNoise());
    double errorRate = check_and_cast<const UnitDiskReception *>(reception)->getPower() == UnitDiskReception::POWER_RECEIVABLE && (noise == nullptr || !noise->isInterfering()) ? 0 : 1;
    auto receptionResult = ReceiverBase::computeReceptionResult(listening, reception, interference, snir, decisions);
    auto errorRateInd = const_cast<Packet *>(receptionResult->getPacket())->addTagIfAbsent<ErrorRateInd>();
    errorRateInd->setSymbolErrorRate(errorRate);
    errorRateInd->setBitErrorRate(errorRate);
    errorRateInd->setPacketErrorRate(errorRate);
    return receptionResult;
}

} // namespace physicallayer
} // namespace inet

