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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "RandomWPMobility.h"


Define_Module(RandomWPMobility);


void RandomWPMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    if (stage == 0)
    {
        stationary = (par("speed").type()=='L' || par("speed").type()=='D') && (double)par("speed") == 0;
        nextMoveIsWait = false;
    }
}

void RandomWPMobility::setTargetPosition()
{
    if (nextMoveIsWait)
    {
        double waitTime = par("waitTime");
        targetTime += waitTime;
    }
    else
    {
        targetPos = getRandomPosition();
        double speed = par("speed");
        double distance = pos.distance(targetPos);
        double travelTime = distance / speed;
        targetTime += travelTime;
    }

    nextMoveIsWait = !nextMoveIsWait;
}

void RandomWPMobility::fixIfHostGetsOutside()
{
    raiseErrorIfOutside();
}

