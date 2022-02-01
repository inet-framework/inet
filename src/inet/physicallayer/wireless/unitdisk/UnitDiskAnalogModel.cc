//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskAnalogModel.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskNoise.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskReception.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskSnir.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskTransmission.h"

namespace inet {
namespace physicallayer {

Define_Module(UnitDiskAnalogModel);

std::ostream& UnitDiskAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "UnitDiskAnalogModel";
}

const IReception *UnitDiskAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
    const UnitDiskTransmission *idealTransmission = check_and_cast<const UnitDiskTransmission *>(transmission);
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Coord& receptionStartPosition = arrival->getStartPosition();
    const Coord& receptionEndPosition = arrival->getEndPosition();
    const Quaternion& receptionStartOrientation = arrival->getStartOrientation();
    const Quaternion& receptionEndOrientation = arrival->getEndOrientation();
    m distance = m(transmission->getStartPosition().distance(receptionStartPosition));
    double obstacleLoss = radioMedium->getObstacleLoss() ? radioMedium->getObstacleLoss()->computeObstacleLoss(Hz(NaN), transmission->getStartPosition(), receptionStartPosition) : 1;
    ASSERT(obstacleLoss == 0 || obstacleLoss == 1);
    UnitDiskReception::Power power;
    if (obstacleLoss == 0)
        power = UnitDiskReception::POWER_UNDETECTABLE;
    else if (distance <= idealTransmission->getCommunicationRange())
        power = UnitDiskReception::POWER_RECEIVABLE;
    else if (distance <= idealTransmission->getInterferenceRange())
        power = UnitDiskReception::POWER_INTERFERING;
    else if (distance <= idealTransmission->getDetectionRange())
        power = UnitDiskReception::POWER_DETECTABLE;
    else
        power = UnitDiskReception::POWER_UNDETECTABLE;
    return new UnitDiskReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation, power);
}

const INoise *UnitDiskAnalogModel::computeNoise(const IListening *listening, const IInterference *interference) const
{
    bool isInterfering = false;
    for (auto interferingReception : *interference->getInterferingReceptions())
        if (check_and_cast<const UnitDiskReception *>(interferingReception)->getPower() >= UnitDiskReception::POWER_INTERFERING)
            isInterfering = true;
    return new UnitDiskNoise(listening->getStartTime(), listening->getEndTime(), isInterfering);
}

const INoise *UnitDiskAnalogModel::computeNoise(const IReception *reception, const INoise *noise) const
{
    bool isInterfering = check_and_cast<const UnitDiskReception *>(reception)->getPower() >= UnitDiskReception::POWER_INTERFERING;
    return new UnitDiskNoise(reception->getStartTime(), reception->getEndTime(), isInterfering);
}

const ISnir *UnitDiskAnalogModel::computeSNIR(const IReception *reception, const INoise *noise) const
{
    return new UnitDiskSnir(reception, noise);
}

} // namespace physicallayer
} // namespace inet

