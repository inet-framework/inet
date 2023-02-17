//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskMediumAnalogModel.h"

#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskNoise.h"
#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskReceptionAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskSnir.h"
#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskTransmissionAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/Reception.h"

namespace inet {
namespace physicallayer {

Define_Module(UnitDiskMediumAnalogModel);

std::ostream& UnitDiskMediumAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "UnitDiskMediumAnalogModel";
}

const IReception *UnitDiskMediumAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
    const UnitDiskTransmissionAnalogModel *analogModel = check_and_cast<const UnitDiskTransmissionAnalogModel *>(transmission->getAnalogModel());
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Coord& receptionStartPosition = arrival->getStartPosition();
    const Coord& receptionEndPosition = arrival->getEndPosition();
    const Quaternion& receptionStartOrientation = arrival->getStartOrientation();
    const Quaternion& receptionEndOrientation = arrival->getEndOrientation();
    m distance = m(transmission->getStartPosition().distance(receptionStartPosition));
    double obstacleLoss = radioMedium->getObstacleLoss() ? radioMedium->getObstacleLoss()->computeObstacleLoss(Hz(NaN), transmission->getStartPosition(), receptionStartPosition) : 1;
    ASSERT(obstacleLoss == 0 || obstacleLoss == 1);
    UnitDiskReceptionAnalogModel::Power power;
    if (obstacleLoss == 0)
        power = UnitDiskReceptionAnalogModel::POWER_UNDETECTABLE;
    else if (distance <= analogModel->getCommunicationRange())
        power = UnitDiskReceptionAnalogModel::POWER_RECEIVABLE;
    else if (distance <= analogModel->getInterferenceRange())
        power = UnitDiskReceptionAnalogModel::POWER_INTERFERING;
    else if (distance <= analogModel->getDetectionRange())
        power = UnitDiskReceptionAnalogModel::POWER_DETECTABLE;
    else
        power = UnitDiskReceptionAnalogModel::POWER_UNDETECTABLE;
    auto receptionAnalogModel = new UnitDiskReceptionAnalogModel(analogModel->getPreambleDuration(), analogModel->getHeaderDuration(), analogModel->getDataDuration(), power);
    return new Reception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation, receptionAnalogModel);
}

const INoise *UnitDiskMediumAnalogModel::computeNoise(const IListening *listening, const IInterference *interference) const
{
    using Power = UnitDiskReceptionAnalogModel::Power;

    Power minPower = Power::POWER_RECEIVABLE;
    Power maxPower = Power::POWER_UNDETECTABLE;

    for (auto interferingReception : *interference->getInterferingReceptions()) {
        Power p = check_and_cast<const UnitDiskReceptionAnalogModel *>(interferingReception->getAnalogModel())->getPower();
        if (p < minPower)
            minPower = p;
        if (p > maxPower)
            maxPower = p;
    }

    if (minPower > maxPower)
        minPower = maxPower;

    return new UnitDiskNoise(listening->getStartTime(), listening->getEndTime(), minPower, maxPower);
}

const INoise *UnitDiskMediumAnalogModel::computeNoise(const IReception *reception, const INoise *noise) const
{
    using Power = UnitDiskReceptionAnalogModel::Power;

    ASSERT(reception->getEndTime() == noise->getEndTime() && reception->getStartTime() == noise->getStartTime());
        Power receptionPower = check_and_cast<const UnitDiskReceptionAnalogModel *>(reception->getAnalogModel())->getPower();

    Power noiseMinPower = check_and_cast<const UnitDiskNoise *>(noise)->getMinPower();
    Power noiseMaxPower = check_and_cast<const UnitDiskNoise *>(noise)->getMaxPower();

    if (receptionPower > noiseMinPower)
        noiseMinPower = receptionPower;
    if (receptionPower > noiseMaxPower)
        noiseMaxPower = receptionPower;

    return new UnitDiskNoise(reception->getStartTime(), reception->getEndTime(), noiseMinPower, noiseMaxPower);
}

const ISnir *UnitDiskMediumAnalogModel::computeSNIR(const IReception *reception, const INoise *noise) const
{
    return new UnitDiskSnir(reception, noise);
}

} // namespace physicallayer
} // namespace inet

