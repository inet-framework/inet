//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/propagation/ConstantTimePropagation.h"

#include "inet/physicallayer/wireless/common/signal/Arrival.h"

namespace inet {

namespace physicallayer {

Define_Module(ConstantTimePropagation);

ConstantTimePropagation::ConstantTimePropagation() :
    PropagationBase()
{
}

void ConstantTimePropagation::initialize(int stage)
{
    PropagationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        propagationTime = par("propagationTime");
}

const IArrival *ConstantTimePropagation::computeArrival(const ITransmission *transmission, IMobility *mobility) const
{
    arrivalComputationCount++;
    const Coord& position = mobility->getCurrentPosition();
    const Quaternion& orientation = mobility->getCurrentAngularPosition();
    const simtime_t startTime = transmission->getStartTime();
    const simtime_t endTime = transmission->getEndTime();
    const simtime_t preambleDuration = transmission->getPreambleDuration();
    const simtime_t headerDuration = transmission->getHeaderDuration();
    const simtime_t dataDuration = transmission->getDataDuration();
    return new Arrival(propagationTime, propagationTime, startTime + propagationTime, endTime + propagationTime, preambleDuration, headerDuration, dataDuration, position, position, orientation, orientation);
}

std::ostream& ConstantTimePropagation::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ConstantTimePropagation";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(propagationTime);
    return PropagationBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

