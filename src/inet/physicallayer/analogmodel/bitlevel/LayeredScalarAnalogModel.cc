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

#include "inet/physicallayer/contract/IRadioMedium.h"
#include "inet/physicallayer/common/BandListening.h"
#include "inet/physicallayer/analogmodel/layered/LayeredScalarAnalogModel.h"
#include "inet/physicallayer/analogmodel/ScalarTransmission.h"
#include "inet/physicallayer/analogmodel/ScalarReception.h"
#include "inet/physicallayer/analogmodel/ScalarNoise.h"
#include "inet/physicallayer/analogmodel/ScalarSNIR.h"
#include "inet/physicallayer/common/layered/LayeredTransmission.h"

namespace inet {

namespace physicallayer {

Define_Module(LayeredScalarAnalogModel);

const IReception *LayeredScalarAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const EulerAngles receptionStartOrientation = arrival->getStartOrientation();
    const EulerAngles receptionEndOrientation = arrival->getEndOrientation();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    const LayeredTransmission *layeredTransmission = check_and_cast<const LayeredTransmission *>(transmission);
    const ScalarTransmissionSignalAnalogModel *transmissionSignalAnalogModel = dynamic_cast<const ScalarTransmissionSignalAnalogModel *>(layeredTransmission->getAnalogModel());
    const W receptionPower = computeReceptionPower(receiverRadio, transmission);
    const ScalarReceptionSignalAnalogModel *receptionSignalAnalogModel = new const ScalarReceptionSignalAnalogModel(transmissionSignalAnalogModel->getDuration(), transmissionSignalAnalogModel->getCarrierFrequency(), transmissionSignalAnalogModel->getBandwidth(), receptionPower);
    return new LayeredReception(receptionSignalAnalogModel, receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation);
}

} // namespace physicallayer

} // namespace inet

