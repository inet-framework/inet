//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/base/LineSegmentsMobilityBase.h"

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
        lastVelocity = (targetPosition - lastPosition) / (nextChange - simTime()).dbl();
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
        lastVelocity = (targetPosition - lastPosition) / (nextChange - simTime()).dbl();
    }
    else if (now > lastUpdate) {
        ASSERT(nextChange == -1 || now < nextChange);
        lastPosition += lastVelocity * (now - lastUpdate).dbl();
    }
}

} // namespace inet

