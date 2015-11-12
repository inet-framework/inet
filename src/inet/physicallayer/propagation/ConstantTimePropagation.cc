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

#include "inet/physicallayer/propagation/ConstantTimePropagation.h"
#include "inet/physicallayer/common/packetlevel/Arrival.h"

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
    const Coord position = mobility->getCurrentPosition();
    const EulerAngles orientation = mobility->getCurrentAngularPosition();
    const simtime_t startTime = transmission->getStartTime();
    const simtime_t endTime = transmission->getEndTime();
    const simtime_t preambleDuration = transmission->getPreambleDuration();
    const simtime_t headerDuration = transmission->getHeaderDuration();
    const simtime_t dataDuration = transmission->getDataDuration();
    return new Arrival(propagationTime, propagationTime, startTime + propagationTime, endTime + propagationTime, preambleDuration, headerDuration, dataDuration, position, position, orientation, orientation);
}

std::ostream& ConstantTimePropagation::printToStream(std::ostream& stream, int level) const
{
    stream << "ConstantTimePropagation";
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", propagationTime = " << propagationTime;
    return PropagationBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

