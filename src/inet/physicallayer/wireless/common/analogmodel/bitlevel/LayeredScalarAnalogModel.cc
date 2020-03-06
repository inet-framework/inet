//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredScalarAnalogModel.h"

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarNoise.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarSnir.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarTransmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(LayeredScalarAnalogModel);

std::ostream& LayeredScalarAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "LayeredScalarAnalogModel";
}

const IReception *LayeredScalarAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Quaternion& receptionStartOrientation = arrival->getStartOrientation();
    const Quaternion& receptionEndOrientation = arrival->getEndOrientation();
    const Coord& receptionStartPosition = arrival->getStartPosition();
    const Coord& receptionEndPosition = arrival->getEndPosition();
    const LayeredTransmission *layeredTransmission = check_and_cast<const LayeredTransmission *>(transmission);
    const ScalarTransmissionSignalAnalogModel *transmissionSignalAnalogModel = check_and_cast<const ScalarTransmissionSignalAnalogModel *>(layeredTransmission->getAnalogModel());
    const W receptionPower = computeReceptionPower(receiverRadio, transmission, arrival);
    const ScalarReceptionSignalAnalogModel *receptionSignalAnalogModel = new ScalarReceptionSignalAnalogModel(transmissionSignalAnalogModel->getPreambleDuration(), transmissionSignalAnalogModel->getHeaderDuration(), transmissionSignalAnalogModel->getDataDuration(), transmissionSignalAnalogModel->getCenterFrequency(), transmissionSignalAnalogModel->getBandwidth(), receptionPower);
    return new LayeredReception(receptionSignalAnalogModel, receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation);
}

} // namespace physicallayer

} // namespace inet

