//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/analogmodel/bitlevel/LayeredDimensionalAnalogModel.h"
#include "inet/physicallayer/analogmodel/bitlevel/LayeredSnir.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/common/bitlevel/LayeredReception.h"
#include "inet/physicallayer/common/bitlevel/LayeredTransmission.h"

namespace inet {

namespace physicallayer {

Define_Module(LayeredDimensionalAnalogModel);

std::ostream& LayeredDimensionalAnalogModel::printToStream(std::ostream& stream, int level) const
{
    stream << "LayeredDimensionalAnalogModel";
    return DimensionalAnalogModelBase::printToStream(stream, level);
}

const IReception *LayeredDimensionalAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Quaternion receptionStartOrientation = arrival->getStartOrientation();
    const Quaternion receptionEndOrientation = arrival->getEndOrientation();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    const ITransmissionAnalogModel *transmissionAnalogModel = check_and_cast<const ITransmissionAnalogModel *>(transmission->getAnalogModel());
    const INarrowbandSignal *narrowbandAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& receptionPower = computeReceptionPower(receiverRadio, transmission, arrival);
    const DimensionalReceptionSignalAnalogModel *receptionSignalAnalogModel = new DimensionalReceptionSignalAnalogModel(transmissionAnalogModel->getPreambleDuration(), transmissionAnalogModel->getHeaderDuration(), transmissionAnalogModel->getDataDuration(), narrowbandAnalogModel->getCenterFrequency(), narrowbandAnalogModel->getBandwidth(), receptionPower);
    return new LayeredReception(receptionSignalAnalogModel, receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation);
}

const ISnir *LayeredDimensionalAnalogModel::computeSNIR(const IReception *reception, const INoise *noise) const
{
    Enter_Method_Silent();
    const LayeredReception *layeredReception = check_and_cast<const LayeredReception *>(reception);
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    return new LayeredSnir(layeredReception, dimensionalNoise);
}

} // namespace physicallayer

} // namespace inet

