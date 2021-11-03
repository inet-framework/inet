//
// Copyright (C) 2005 Emin Ilker Cetinbas
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
//

#include "inet/mobility/single/LinearMobility.h"

#include "inet/common/INETMath.h"

namespace inet {

Define_Module(LinearMobility);

LinearMobility::LinearMobility()
{
    speed = 0;
}

void LinearMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);

    EV_TRACE << "initializing LinearMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        speed = par("speed");
        stationary = (speed == 0);
        rad heading = deg(fmod(par("initialMovementHeading").doubleValue(), 360));
        rad elevation = deg(fmod(par("initialMovementElevation").doubleValue(), 360));
        Coord direction = Quaternion(EulerAngles(heading, -elevation, rad(0))).rotate(Coord::X_AXIS);
        startTime = simTime();

        lastVelocity = startVelocity = direction * speed;
    }
    else if (stage == INITSTAGE_SINGLE_MOBILITY) {
        startPosition = lastPosition;
    }
}

void LinearMobility::move()
{
    // TODO longer elapsed time --> smaller precision.
    double elapsedTime = (simTime() - startTime).dbl();
    lastPosition = startPosition + startVelocity * elapsedTime;
    lastVelocity = startVelocity;

    // do something if we reach the wall
    Coord dummyCoord;
    handleIfOutside(REFLECT, dummyCoord, lastVelocity);

    if (faceForward && (lastVelocity != Coord::ZERO)) {
        // determine orientation based on direction
        lastOrientation = getOrientOfVelocity(lastVelocity);
    }
}

} // namespace inet

