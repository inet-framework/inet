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

#include "inet/physicallayer/common/packetlevel/Arrival.h"
#include "inet/physicallayer/propagation/ConstantSpeedPropagation.h"

namespace inet {

namespace physicallayer {

Define_Module(ConstantSpeedPropagation);

ConstantSpeedPropagation::ConstantSpeedPropagation() :
    PropagationBase(),
    ignoreMovementDuringTransmission(false),
    ignoreMovementDuringPropagation(false),
    ignoreMovementDuringReception(false)
{
}

void ConstantSpeedPropagation::initialize(int stage)
{
    PropagationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        ignoreMovementDuringTransmission = par("ignoreMovementDuringTransmission");
        ignoreMovementDuringPropagation = par("ignoreMovementDuringPropagation");
        ignoreMovementDuringReception = par("ignoreMovementDuringReception");
        // TODO:
        if (!ignoreMovementDuringTransmission)
            throw cRuntimeError("ignoreMovementDuringTransmission is yet not implemented");
    }
}

const Coord ConstantSpeedPropagation::computeArrivalPosition(const simtime_t time, const Coord position, IMobility *mobility) const
{
    // TODO: return mobility->getPosition(time);
    throw cRuntimeError("Movement approximation is not implemented");
}

std::ostream& ConstantSpeedPropagation::printToStream(std::ostream& stream, int level) const
{
    stream << "ConstantSpeedPropagation";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", ignoreMovementDuringTransmission = " << ignoreMovementDuringTransmission
               << ", ignoreMovementDuringPropagation = " << ignoreMovementDuringPropagation
               << ", ignoreMovementDuringReception = " << ignoreMovementDuringReception;
    return PropagationBase::printToStream(stream, level);
}

const IArrival *ConstantSpeedPropagation::computeArrival(const ITransmission *transmission, IMobility *mobility) const
{
    arrivalComputationCount++;
    const simtime_t startTime = transmission->getStartTime();
    const simtime_t endTime = transmission->getEndTime();
    const Coord startPosition = transmission->getStartPosition();
    const Coord endPosition = transmission->getEndPosition();
    const Coord startArrivalPosition = ignoreMovementDuringPropagation ? mobility->getCurrentPosition() : computeArrivalPosition(startTime, startPosition, mobility);
    const simtime_t startPropagationTime = startPosition.distance(startArrivalPosition) / propagationSpeed.get();
    const simtime_t startArrivalTime = startTime + startPropagationTime;
    const Quaternion startArrivalOrientation = mobility->getCurrentAngularPosition();
    if (ignoreMovementDuringReception) {
        const Coord endArrivalPosition = startArrivalPosition;
        const simtime_t endPropagationTime = startPropagationTime;
        const simtime_t endArrivalTime = endTime + startPropagationTime;
        const simtime_t preambleDuration = transmission->getPreambleDuration();
        const simtime_t headerDuration = transmission->getHeaderDuration();
        const simtime_t dataDuration = transmission->getDataDuration();
        const Quaternion endArrivalOrientation = mobility->getCurrentAngularPosition();
        return new Arrival(startPropagationTime, endPropagationTime, startArrivalTime, endArrivalTime, preambleDuration, headerDuration, dataDuration, startArrivalPosition, endArrivalPosition, startArrivalOrientation, endArrivalOrientation);
    }
    else {
        const Coord endArrivalPosition = computeArrivalPosition(endTime, endPosition, mobility);
        const simtime_t endPropagationTime = endPosition.distance(endArrivalPosition) / propagationSpeed.get();
        const simtime_t endArrivalTime = endTime + endPropagationTime;
        const simtime_t preambleDuration = transmission->getPreambleDuration();
        const simtime_t headerDuration = transmission->getHeaderDuration();
        const simtime_t dataDuration = transmission->getDataDuration();
        const Quaternion endArrivalOrientation = mobility->getCurrentAngularPosition();
        return new Arrival(startPropagationTime, endPropagationTime, startArrivalTime, endArrivalTime, preambleDuration, headerDuration, dataDuration, startArrivalPosition, endArrivalPosition, startArrivalOrientation, endArrivalOrientation);
    }
}

} // namespace physicallayer

} // namespace inet

