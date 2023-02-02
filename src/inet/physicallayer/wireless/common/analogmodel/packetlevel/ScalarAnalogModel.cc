//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarAnalogModel.h"

#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarReceptionAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarTransmitterAnalogModel.h"


namespace inet {

namespace physicallayer {

Define_Module(ScalarAnalogModel);

std::ostream& ScalarAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "ScalarAnalogModel";
}

const IReception *ScalarAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
    const ScalarTransmissionAnalogModel *analogModel = check_and_cast<const ScalarTransmissionAnalogModel *>(transmission->getNewAnalogModel());


    //const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Quaternion& receptionStartOrientation = arrival->getStartOrientation();
    const Quaternion& receptionEndOrientation = arrival->getEndOrientation();
    const Coord& receptionStartPosition = arrival->getStartPosition();
    const Coord& receptionEndPosition = arrival->getEndPosition();
    W receptionPower = computeReceptionPower(receiverRadio, transmission, arrival);
    auto reception = new ReceptionBase(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation);
    reception->analogModel = new ScalarReceptionAnalogModel(analogModel->getCenterFrequency(), analogModel->getBandwidth(), receptionPower);
    return reception;
}

} // namespace physicallayer

} // namespace inet

