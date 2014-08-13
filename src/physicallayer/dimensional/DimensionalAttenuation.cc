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

#include "DimensionalAttenuation.h"
#include "DimensionalTransmission.h"
#include "DimensionalReception.h"
#include "IRadioMedium.h"

namespace inet {

namespace physicallayer {

Define_Module(DimensionalAttenuation);

const IReception *DimensionalAttenuation::computeReception(const IRadio *receiverRadio, const ITransmission *transmission) const
{
    const IRadioMedium *channel = receiverRadio->getMedium();
    const IRadio *transmitterRadio = transmission->getTransmitter();
    const IAntenna *receiverAntenna = receiverRadio->getAntenna();
    const IAntenna *transmitterAntenna = transmitterRadio->getAntenna();
    const DimensionalTransmission *dimensionalTransmission = check_and_cast<const DimensionalTransmission *>(transmission);
    const IArrival *arrival = channel->getArrival(receiverRadio, transmission);
    const simtime_t transmissionStartTime = transmission->getStartTime();
    const simtime_t transmissionEndTime = transmission->getEndTime();
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
    mps propagationSpeed = channel->getPropagation()->getPropagationSpeed();
    const ConstMapping *transmissionPower = dimensionalTransmission->getPower();
    EV_DEBUG << "Transmission power begin " << endl;
    transmissionPower->print(EVSTREAM);
    EV_DEBUG << "Transmission power end" << endl;
    ConstMappingIterator *it = transmissionPower->createConstIterator();
    Mapping *receptionPower = MappingUtils::createMapping(Argument::MappedZero, transmissionPower->getDimensionSet(), Mapping::LINEAR);
    while (true) {
        const Argument& position = it->getPosition();
        Hz carrierFrequency = transmissionPower->getDimensionSet().hasDimension(Dimension::frequency) ? Hz(position.getArgValue(Dimension::frequency)) : dimensionalTransmission->getCarrierFrequency();
        double pathLoss = channel->getPathLoss()->computePathLoss(propagationSpeed, carrierFrequency, distance);
        double obstacleLoss = channel->getObstacleLoss() ? channel->getObstacleLoss()->computeObstacleLoss(carrierFrequency, transmission->getStartPosition(), receptionStartPosition) : 1;
        W elementTransmissionPower = W(it->getValue());
        W elementReceptionPower = elementTransmissionPower * std::min(1.0, transmitterAntennaGain * receiverAntennaGain * pathLoss * obstacleLoss);
        Argument receptionPosition = position;
        double alpha = (position.getTime() - transmissionStartTime).dbl() / (transmissionEndTime - transmissionStartTime).dbl();
        receptionPosition.setTime(receptionStartTime + alpha * (receptionEndTime - receptionStartTime).dbl());
        receptionPower->setValue(receptionPosition, elementReceptionPower.get());
        if (it->hasNext())
            it->next();
        else
            break;
    }
    EV_DEBUG << "Reception power begin " << endl;
    receptionPower->print(EVSTREAM);
    EV_DEBUG << "Reception power end" << endl;
    return new DimensionalReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation, dimensionalTransmission->getCarrierFrequency(), dimensionalTransmission->getBandwidth(), receptionPower);
}

} // namespace physicallayer

} // namespace inet

