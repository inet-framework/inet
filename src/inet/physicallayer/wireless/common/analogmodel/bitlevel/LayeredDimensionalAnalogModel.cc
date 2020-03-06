//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredDimensionalAnalogModel.h"

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredSnir.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalSnir.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"


namespace inet {

namespace physicallayer {

Define_Module(LayeredDimensionalAnalogModel);

std::ostream& LayeredDimensionalAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "LayeredDimensionalAnalogModel";
    return DimensionalAnalogModelBase::printToStream(stream, level);
}

const IReception *LayeredDimensionalAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Quaternion& receptionStartOrientation = arrival->getStartOrientation();
    const Quaternion& receptionEndOrientation = arrival->getEndOrientation();
    const Coord& receptionStartPosition = arrival->getStartPosition();
    const Coord& receptionEndPosition = arrival->getEndPosition();
    const ITransmissionAnalogModel *transmissionAnalogModel = check_and_cast<const ITransmissionAnalogModel *>(transmission->getAnalogModel());
    const INarrowbandSignal *narrowbandAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& receptionPower = computeReceptionPower(receiverRadio, transmission, arrival);
    const DimensionalReceptionSignalAnalogModel *receptionSignalAnalogModel = new DimensionalReceptionSignalAnalogModel(transmissionAnalogModel->getPreambleDuration(), transmissionAnalogModel->getHeaderDuration(), transmissionAnalogModel->getDataDuration(), narrowbandAnalogModel->getCenterFrequency(), narrowbandAnalogModel->getBandwidth(), receptionPower);
    return new LayeredReception(receptionSignalAnalogModel, receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation);
}

const ISnir *LayeredDimensionalAnalogModel::computeSNIR(const IReception *reception, const INoise *noise) const
{
    const LayeredReception *layeredReception = check_and_cast<const LayeredReception *>(reception);
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    return new LayeredSnir(layeredReception, dimensionalNoise);
}

} // namespace physicallayer

} // namespace inet

