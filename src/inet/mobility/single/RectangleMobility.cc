//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/single/RectangleMobility.h"

#include "inet/common/INETMath.h"

namespace inet {

Define_Module(RectangleMobility);

RectangleMobility::RectangleMobility()
{
    speed = 0.0;
    startPosition = 0.0;
    corner1 = corner2 = corner3 = corner4 = 0.0;
}

void RectangleMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);

    EV_TRACE << "initializing RectangleMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        speed = par("speed");
        stationary = (speed == 0);
        startTime = simTime();

        // calculate helper variables
        double dx = constraintAreaMax.x - constraintAreaMin.x;
        double dy = constraintAreaMax.y - constraintAreaMin.y;
        corner1 = dx;
        corner2 = corner1 + dy;
        corner3 = corner2 + dx;
        corner4 = corner3 + dy;

        // determine start position
        double startPos = par("startPos");
        startPos = fmod(startPos, 4);
        if (startPos < 1)
            startPosition = startPos * dx; // top side
        else if (startPos < 2)
            startPosition = corner1 + (startPos - 1) * dy; // right side
        else if (startPos < 3)
            startPosition = corner2 + (startPos - 2) * dx; // bottom side
        else
            startPosition = corner3 + (startPos - 3) * dy; // left side
        WATCH(startPosition);
    }
}

void RectangleMobility::setInitialPosition()
{
    MovingMobilityBase::setInitialPosition();
    move();
}

void RectangleMobility::move()
{
    double elapsedTime = (simTime() - startTime).dbl();
    double d = fmod(startPosition + speed * elapsedTime, corner4);

    if (d < corner1) {
        // top side
        lastPosition.x = constraintAreaMin.x + d;
        lastPosition.y = constraintAreaMin.y;
        lastVelocity = Coord(speed, 0, 0);
    }
    else if (d < corner2) {
        // right side
        lastPosition.x = constraintAreaMax.x;
        lastPosition.y = constraintAreaMin.y + d - corner1;
        lastVelocity = Coord(0, speed, 0);
    }
    else if (d < corner3) {
        // bottom side
        lastPosition.x = constraintAreaMax.x - d + corner2;
        lastPosition.y = constraintAreaMax.y;
        lastVelocity = Coord(-speed, 0, 0);
    }
    else {
        // left side
        lastPosition.x = constraintAreaMin.x;
        lastPosition.y = constraintAreaMax.y - d + corner3;
        lastVelocity = Coord(0, -speed, 0);
    }
}

} // namespace inet

