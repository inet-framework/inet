//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/scalar/ScalarAttenuation.h"
#include "inet/physicallayer/scalar/ScalarTransmission.h"
#include "inet/physicallayer/scalar/ScalarReception.h"
#include "inet/physicallayer/contract/IRadioMedium.h"

namespace inet {

namespace physicallayer {

Define_Module(ScalarAttenuation);

const IReception *ScalarAttenuation::computeReception(const IRadio *receiverRadio, const ITransmission *transmission) const
{
    const IRadioMedium *channel = receiverRadio->getMedium();
    const IRadio *transmitterRadio = transmission->getTransmitter();
    const IAntenna *receiverAntenna = receiverRadio->getAntenna();
    const IAntenna *transmitterAntenna = transmitterRadio->getAntenna();
    const ScalarTransmission *scalarTransmission = check_and_cast<const ScalarTransmission *>(transmission);
    const IArrival *arrival = channel->getArrival(receiverRadio, transmission);
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    const EulerAngles receptionStartOrientation = arrival->getStartOrientation();
    const EulerAngles receptionEndOrientation = arrival->getEndOrientation();
    const EulerAngles transmissionDirection = computeTransmissionDirection(transmission, arrival);
    const EulerAngles transmissionAntennaDirection = transmission->getStartOrientation() - transmissionDirection;
    const EulerAngles receptionAntennaDirection = transmissionDirection - arrival->getStartOrientation();
    double transmitterAntennaGain = transmitterAntenna->computeGain(transmissionAntennaDirection);
    double receiverAntennaGain = receiverAntenna->computeGain(receptionAntennaDirection);
    m distance = m(receptionStartPosition.distance(transmission->getStartPosition()));
    double pathLoss = channel->getPathLoss()->computePathLoss(channel->getPropagation()->getPropagationSpeed(), scalarTransmission->getCarrierFrequency(), distance);
    double obstacleLoss = channel->getObstacleLoss() ? channel->getObstacleLoss()->computeObstacleLoss(scalarTransmission->getCarrierFrequency(), transmission->getStartPosition(), receptionStartPosition) : 1;
    W transmissionPower = scalarTransmission->getPower();
    W receptionPower = transmissionPower * std::min(1.0, transmitterAntennaGain * receiverAntennaGain * pathLoss * obstacleLoss);
    return new ScalarReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation, scalarTransmission->getCarrierFrequency(), scalarTransmission->getBandwidth(), receptionPower);
}

} // namespace physicallayer

} // namespace inet

