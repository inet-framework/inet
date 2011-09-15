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


#include "LineSegmentsMobilityBase.h"
#include "FWMath.h"


LineSegmentsMobilityBase::LineSegmentsMobilityBase()
{
    targetPosition = Coord::ZERO;
}

void LineSegmentsMobilityBase::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);
    EV << "initializing LineSegmentsMobilityBase stage " << stage << endl;
    if (stage == 1)
    {
        if (!stationary) {
            setTargetPosition();
            lastSpeed = (targetPosition - lastPosition) / (nextChange - simTime()).dbl();
        }
    }
}

void LineSegmentsMobilityBase::move()
{
    simtime_t now = simTime();
    if (now == nextChange) {
        lastPosition = targetPosition;
        EV << "destination reached. x = " << lastPosition.x << " y = " << lastPosition.y << " z = " << lastPosition.z << endl;
        setTargetPosition();
        lastSpeed = (targetPosition - lastPosition) / (nextChange - simTime()).dbl();
    }
    else if (now > lastUpdate) {
        ASSERT(nextChange == -1 || now < nextChange);
        lastPosition += lastSpeed * (now - lastUpdate).dbl();
        EV << "going forward. x = " << lastPosition.x << " y = " << lastPosition.y << " z = " << lastPosition.z << endl;
    }
}
