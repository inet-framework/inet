//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/single/CircleMobility.h"

#include "inet/common/INETMath.h"

namespace inet {

Define_Module(CircleMobility);

void CircleMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);

    EV_TRACE << "initializing CircleMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        cx = par("cx");
        cy = par("cy");
        cz = par("cz");
        r = par("r");
        ASSERT(r > 0);
        startAngle = deg(par("startAngle"));
        speed = par("speed");
        omega = speed / r;
        stationary = (omega == 0);
        startTime = simTime();
        startAngularVelocity = Quaternion(EulerAngles(rad(omega), rad(0), rad(0)));
        WATCH(startAngularVelocity);
        lastAngularVelocity = startAngularVelocity;
    }
}

void CircleMobility::setInitialMobilityData()
{
    move();
    orient();
}

void CircleMobility::move()
{
    angle = startAngle + rad(omega * (simTime() - startTime).dbl());
    double cosAngle = cos(rad(angle).get());
    double sinAngle = sin(rad(angle).get());
    lastPosition.x = cx + r * cosAngle;
    lastPosition.y = cy + r * sinAngle;
    lastPosition.z = cz;
    lastVelocity.x = -sinAngle * speed;
    lastVelocity.y = cosAngle * speed;
    lastVelocity.z = 0;
    lastAngularVelocity = startAngularVelocity;
    // do something if we reach the wall
    Coord dummyCoord;
    rad a;
    handleIfOutside(REFLECT, dummyCoord, lastVelocity, a, a, lastAngularVelocity);
}

} // namespace inet

