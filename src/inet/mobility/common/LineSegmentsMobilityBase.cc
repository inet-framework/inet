//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/mobility/common/LineSegmentsMobilityBase.h"
#include "inet/common/INETMath.h"

namespace inet {

LineSegmentsMobilityBase::LineSegmentsMobilityBase()
{
    targetPosition = Coord::ZERO;
}

void LineSegmentsMobilityBase::initializePosition()
{
    MobilityBase::initializePosition();
    if (!stationary) {
        setTargetPosition();
        EV_INFO << "current target position = " << targetPosition << ", next change = " << nextChange << endl;
        lastSpeed = (targetPosition - lastPosition) / (nextChange - simTime()).dbl();
    }
    lastUpdate = simTime();
    scheduleUpdate();
}

void LineSegmentsMobilityBase::move()
{
    simtime_t now = simTime();
    if (now == nextChange) {
        lastPosition = targetPosition;
        EV_INFO << "reached current target position = " << lastPosition << endl;
        setTargetPosition();
        EV_INFO << "new target position = " << targetPosition << ", next change = " << nextChange << endl;
        lastSpeed = (targetPosition - lastPosition) / (nextChange - simTime()).dbl();
    }
    else if (now > lastUpdate) {
        ASSERT(nextChange == -1 || now < nextChange);
        lastPosition += lastSpeed * (now - lastUpdate).dbl();
    }
}

} // namespace inet

