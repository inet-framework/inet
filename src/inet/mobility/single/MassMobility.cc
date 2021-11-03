//
// Copyright (C) 2005 Emin Ilker Cetinbas, Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
// Generalization: Andras Varga
//

#include "inet/mobility/single/MassMobility.h"

#include "inet/common/INETMath.h"

namespace inet {

Define_Module(MassMobility);

MassMobility::MassMobility()
{
    borderPolicy = REFLECT;
}

void MassMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    EV_TRACE << "initializing MassMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        rad heading = deg(par("initialMovementHeading"));
        rad elevation = deg(par("initialMovementElevation"));
        changeIntervalParameter = &par("changeInterval");
        angleDeltaParameter = &par("angleDelta");
        rotationAxisAngleParameter = &par("rotationAxisAngle");
        speedParameter = &par("speed");
        quaternion = Quaternion(EulerAngles(heading, -elevation, rad(0)));
        WATCH(quaternion);
    }
}

void MassMobility::move()
{
    LineSegmentsMobilityBase::move();
    if (faceForward)
        lastOrientation = quaternion;
}

void MassMobility::setTargetPosition()
{
    rad angleDelta = deg(angleDeltaParameter->doubleValue());
    rad rotationAxisAngle = deg(rotationAxisAngleParameter->doubleValue());
    Quaternion dQ = Quaternion(Coord::X_AXIS, rotationAxisAngle.get()) * Quaternion(Coord::Z_AXIS, angleDelta.get());
    quaternion = quaternion * dQ;
    quaternion.normalize();
    Coord direction = quaternion.rotate(Coord::X_AXIS);

    simtime_t nextChangeInterval = *changeIntervalParameter;
    EV_DEBUG << "interval: " << nextChangeInterval << endl;
    targetPosition = lastPosition + direction * (*speedParameter) * nextChangeInterval.dbl();
    nextChange = segmentStartTime + nextChangeInterval;
}

void MassMobility::processBorderPolicy()
{
    Coord dummyCoord;
    rad dummyAngle;
    Quaternion dummyQuaternion;

    if (simTime() == nextChange) {
        handleIfOutside(borderPolicy, targetPosition, lastVelocity, dummyAngle, dummyAngle, quaternion);
        if (faceForward)
            lastOrientation = quaternion;
    }
    else {
        handleIfOutside(borderPolicy, dummyCoord, lastVelocity, dummyAngle, dummyAngle, faceForward ? lastOrientation : dummyQuaternion);
    }
}

double MassMobility::getMaxSpeed() const
{
    return speedParameter->isExpression() ? NaN : speedParameter->doubleValue();
}

} // namespace inet

