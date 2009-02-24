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

#include <algorithm>   // min,max
#include "LineSegmentsMobilityBase.h"
#include "FWMath.h"


void LineSegmentsMobilityBase::initialize(int stage)
{
    BasicMobility::initialize(stage);

    if (stage == 1)
    {
        updateInterval = par("updateInterval");
        stationary = false;
        targetPos = pos;
        targetTime = simTime();

        // host moves the first time after some random delay to avoid synchronized movements
        scheduleAt(simTime() + uniform(0, updateInterval), new cMessage("move"));
    }
}

void LineSegmentsMobilityBase::beginNextMove(cMessage *msg)
{
    // go to exact position where previous statement was supposed to finish
    pos = targetPos;
    simtime_t now = targetTime;

    // choose new targetTime and targetPos
    setTargetPosition();

    if (targetTime<now)
        error("LineSegmentsMobilityBase: targetTime<now was set in %s's beginNextMove()", getClassName());

    if (stationary)
    {
        // end of movement
        step.x = step.y = 0;
        delete msg;
    }
    else if (targetPos==pos)
    {
        // no movement, just wait
        step.x = step.y = 0;
        scheduleAt(std::max(targetTime,simTime()), msg);
    }
    else
    {
        // keep moving
        double numIntervals = SIMTIME_DBL(targetTime-now) / updateInterval;
        // int numSteps = floor(numIntervals); -- currently unused,
        // although we could use step counting instead of comparing
        // simTime() to targetTime each step.

        // Note: step = speed*updateInterval = distance/time*updateInterval =
        //        = (targetPos-pos) / (targetTime-now) * updateInterval =
        //        = (targetPos-pos) / numIntervals
        step = (targetPos - pos) / numIntervals;
        scheduleAt(simTime() + updateInterval, msg);
    }
}

void LineSegmentsMobilityBase::handleSelfMsg(cMessage *msg)
{
    if (stationary)
    {
        delete msg;
        return;
    }
    else if (simTime()+updateInterval >= targetTime)
    {
        beginNextMove(msg);
    }
    else
    {
        scheduleAt(simTime() + updateInterval, msg);
    }

    // update position
    pos += step;

    // do something if we reach the wall
    fixIfHostGetsOutside();

    //EV << " xpos=" << pos.x << " ypos=" << pos.y << endl;

    updatePosition();
}


