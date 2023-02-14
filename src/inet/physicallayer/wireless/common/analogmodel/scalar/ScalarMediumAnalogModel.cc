//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarMediumAnalogModel.h"

#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarTransmitterAnalogModel.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/Reception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"


namespace inet {

namespace physicallayer {

Define_Module(ScalarMediumAnalogModel);

std::ostream& ScalarMediumAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "ScalarMediumAnalogModel";
}

const IReception *ScalarMediumAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
    const ScalarSignalAnalogModel *analogModel = check_and_cast<const ScalarSignalAnalogModel *>(transmission->getAnalogModel());


    //const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Quaternion& receptionStartOrientation = arrival->getStartOrientation();
    const Quaternion& receptionEndOrientation = arrival->getEndOrientation();
    const Coord& receptionStartPosition = arrival->getStartPosition();
    const Coord& receptionEndPosition = arrival->getEndPosition();
    W receptionPower = computeReceptionPower(receiverRadio, transmission, arrival);
    auto receptionAnalogModel = new ScalarReceptionSignalAnalogModel(-1, -1, -1, analogModel->getCenterFrequency(), analogModel->getBandwidth(), receptionPower);
    auto reception = new Reception(receptionAnalogModel, receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation);
    return reception;
}

} // namespace physicallayer

} // namespace inet

