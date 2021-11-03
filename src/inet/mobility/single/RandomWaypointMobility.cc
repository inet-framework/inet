//
// Copyright (C) 2005 Georg Lutz, Institut fuer Telematik, University of Karlsruhe
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#include "inet/mobility/single/RandomWaypointMobility.h"

namespace inet {

Define_Module(RandomWaypointMobility);

RandomWaypointMobility::RandomWaypointMobility()
{
    nextMoveIsWait = false;
    borderPolicy = RAISEERROR;
}

void RandomWaypointMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        borderPolicy = RAISEERROR;
        waitTimeParameter = &par("waitTime");
        hasWaitTime = waitTimeParameter->isExpression() || waitTimeParameter->doubleValue() != 0;
        speedParameter = &par("speed");
        stationary = !speedParameter->isExpression() && speedParameter->doubleValue() == 0;
    }
}

void RandomWaypointMobility::setTargetPosition()
{
    if (nextMoveIsWait) {
        simtime_t waitTime = waitTimeParameter->doubleValue();
        targetPosition = segmentStartPosition = lastPosition;
        segmentStartOrientation = lastOrientation;
        segmentStartVelocity = Coord::ZERO;
        nextChange = simTime() + waitTime;
        nextMoveIsWait = false;
    }
    else {
        segmentStartPosition = lastPosition;
        targetPosition = getRandomPosition();
        double speed = speedParameter->doubleValue();
        double distance = segmentStartPosition.distance(targetPosition);
        simtime_t travelTime = distance / speed;
        segmentStartVelocity = (targetPosition - segmentStartPosition).normalize() * speed;
        segmentStartOrientation = faceForward ? getOrientOfVelocity(segmentStartVelocity)

        nextChange = simTime() + travelTime;
        nextMoveIsWait = hasWaitTime;
    }
}

double RandomWaypointMobility::getMaxSpeed() const
{
    return speedParameter->isExpression() ? NaN : speedParameter->doubleValue();
}

} // namespace inet

