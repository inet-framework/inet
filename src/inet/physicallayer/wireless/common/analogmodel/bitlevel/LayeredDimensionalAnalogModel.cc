//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredDimensionalAnalogModel.h"

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalNoise.h"
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
    const LayeredTransmission *layeredTransmission = check_and_cast<const LayeredTransmission *>(transmission);
    const DimensionalTransmissionSignalAnalogModel *transmissionSignalAnalogModel = check_and_cast<const DimensionalTransmissionSignalAnalogModel *>(layeredTransmission->getAnalogModel());
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& receptionPower = computeReceptionPower(receiverRadio, transmission, arrival);
    const DimensionalReceptionSignalAnalogModel *receptionSignalAnalogModel = new DimensionalReceptionSignalAnalogModel(transmissionSignalAnalogModel->getDuration(), transmissionSignalAnalogModel->getCenterFrequency(), transmissionSignalAnalogModel->getBandwidth(), receptionPower);
    return new LayeredReception(receptionSignalAnalogModel, receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation);
}

} // namespace physicallayer

} // namespace inet

