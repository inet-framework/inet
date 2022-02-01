//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/propagation/ConstantSpeedPropagation.h"

#include "inet/physicallayer/wireless/common/signal/Arrival.h"

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
        // TODO
        if (!ignoreMovementDuringTransmission)
            throw cRuntimeError("ignoreMovementDuringTransmission is yet not implemented");
    }
}

const Coord ConstantSpeedPropagation::computeArrivalPosition(const simtime_t time, const Coord& position, IMobility *mobility) const
{
    // TODO return mobility->getPosition(time);
    throw cRuntimeError("Movement approximation is not implemented");
}

std::ostream& ConstantSpeedPropagation::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ConstantSpeedPropagation";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(ignoreMovementDuringTransmission)
               << EV_FIELD(ignoreMovementDuringPropagation)
               << EV_FIELD(ignoreMovementDuringReception);
    return PropagationBase::printToStream(stream, level);
}

const IArrival *ConstantSpeedPropagation::computeArrival(const ITransmission *transmission, IMobility *mobility) const
{
    arrivalComputationCount++;
    const simtime_t startTime = transmission->getStartTime();
    const simtime_t endTime = transmission->getEndTime();
    const Coord& startPosition = transmission->getStartPosition();
    const Coord& endPosition = transmission->getEndPosition();
    const Coord& startArrivalPosition = ignoreMovementDuringPropagation ? mobility->getCurrentPosition() : computeArrivalPosition(startTime, startPosition, mobility);
    const simtime_t startPropagationTime = startPosition.distance(startArrivalPosition) / propagationSpeed.get();
    const simtime_t startArrivalTime = startTime + startPropagationTime;
    const Quaternion& startArrivalOrientation = mobility->getCurrentAngularPosition();
    if (ignoreMovementDuringReception) {
        const Coord& endArrivalPosition = startArrivalPosition;
        const simtime_t endPropagationTime = startPropagationTime;
        const simtime_t endArrivalTime = endTime + startPropagationTime;
        const simtime_t preambleDuration = transmission->getPreambleDuration();
        const simtime_t headerDuration = transmission->getHeaderDuration();
        const simtime_t dataDuration = transmission->getDataDuration();
        const Quaternion& endArrivalOrientation = mobility->getCurrentAngularPosition();
        return new Arrival(startPropagationTime, endPropagationTime, startArrivalTime, endArrivalTime, preambleDuration, headerDuration, dataDuration, startArrivalPosition, endArrivalPosition, startArrivalOrientation, endArrivalOrientation);
    }
    else {
        const Coord& endArrivalPosition = computeArrivalPosition(endTime, endPosition, mobility);
        const simtime_t endPropagationTime = endPosition.distance(endArrivalPosition) / propagationSpeed.get();
        const simtime_t endArrivalTime = endTime + endPropagationTime;
        const simtime_t preambleDuration = transmission->getPreambleDuration();
        const simtime_t headerDuration = transmission->getHeaderDuration();
        const simtime_t dataDuration = transmission->getDataDuration();
        const Quaternion& endArrivalOrientation = mobility->getCurrentAngularPosition();
        return new Arrival(startPropagationTime, endPropagationTime, startArrivalTime, endArrivalTime, preambleDuration, headerDuration, dataDuration, startArrivalPosition, endArrivalPosition, startArrivalOrientation, endArrivalOrientation);
    }
}

} // namespace physicallayer

} // namespace inet

