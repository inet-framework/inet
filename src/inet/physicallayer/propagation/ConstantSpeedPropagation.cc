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

#include "inet/physicallayer/propagation/ConstantSpeedPropagation.h"
#include "inet/physicallayer/common/Arrival.h"

namespace inet {

namespace physicallayer {

Define_Module(ConstantSpeedPropagation);

ConstantSpeedPropagation::ConstantSpeedPropagation() :
    PropagationBase(),
    mobilityApproximationCount(0)
{
}

const Coord ConstantSpeedPropagation::computeArrivalPosition(const simtime_t time, const Coord position, IMobility *mobility) const
{
    switch (mobilityApproximationCount) {
        case 0:
            return mobility->getCurrentPosition();

// TODO: revive
//        case 1:
//            return mobility->getPosition(time);
//        case 2:
//        {
//            // NOTE: repeat once again to approximate the movement during propagation
//            double distance = position.distance(mobility->getPosition(time));
//            simtime_t propagationTime = distance / propagationSpeed.get();
//            return mobility->getPosition(time + propagationTime);
//        }
        default:
            throw cRuntimeError("Unknown mobility approximation count '%d'", mobilityApproximationCount);
    }
}

void ConstantSpeedPropagation::printToStream(std::ostream& stream) const
{
    stream << "ConstantSpeedPropagation, "
           << "mobilityApproximationCount = " << mobilityApproximationCount << ", ";
    PropagationBase::printToStream(stream);
}

const IArrival *ConstantSpeedPropagation::computeArrival(const ITransmission *transmission, IMobility *mobility) const
{
    arrivalComputationCount++;
    const simtime_t startTime = transmission->getStartTime();
    const simtime_t endTime = transmission->getEndTime();
    const Coord startPosition = transmission->getStartPosition();
    const Coord endPosition = transmission->getEndPosition();
    const Coord startArrivalPosition = computeArrivalPosition(startTime, startPosition, mobility);
    const Coord endArrivalPosition = computeArrivalPosition(endTime, endPosition, mobility);
    const simtime_t startPropagationTime = startPosition.distance(startArrivalPosition) / propagationSpeed.get();
    const simtime_t endPropagationTime = endPosition.distance(endArrivalPosition) / propagationSpeed.get();
    const simtime_t startArrivalTime = startTime + startPropagationTime;
    const simtime_t endArrivalTime = endTime + endPropagationTime;
    const EulerAngles startArrivalOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endArrivalOrientation = mobility->getCurrentAngularPosition();
    return new Arrival(startPropagationTime, endPropagationTime, startArrivalTime, endArrivalTime, startArrivalPosition, endArrivalPosition, startArrivalOrientation, endArrivalOrientation);
}

} // namespace physicallayer

} // namespace inet

