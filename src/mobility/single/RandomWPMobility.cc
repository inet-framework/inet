//
// Copyright (C) 2005 Georg Lutz, Institut fuer Telematik, University of Karlsruhe
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

#include "inet/mobility/single/RandomWPMobility.h"

namespace inet {

Define_Module(RandomWPMobility);

RandomWPMobility::RandomWPMobility()
{
    nextMoveIsWait = false;
}

void RandomWPMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        stationary = (par("speed").getType() == 'L' || par("speed").getType() == 'D') && (double)par("speed") == 0;
    }
}

void RandomWPMobility::setTargetPosition()
{
    if (nextMoveIsWait) {
        simtime_t waitTime = par("waitTime");
        nextChange = simTime() + waitTime;
    }
    else {
        targetPosition = getRandomPosition();
        double speed = par("speed");
        double distance = lastPosition.distance(targetPosition);
        simtime_t travelTime = distance / speed;
        nextChange = simTime() + travelTime;
    }
    nextMoveIsWait = !nextMoveIsWait;
}

void RandomWPMobility::move()
{
    LineSegmentsMobilityBase::move();
    raiseErrorIfOutside();
}

double RandomWPMobility::getMaxSpeed() const
{
    return NaN;
}

} // namespace inet

