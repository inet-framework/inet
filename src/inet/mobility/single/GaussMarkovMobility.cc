//
// Copyright (C) 2009 Marcin Kosiba
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author: Marcin Kosiba marcin.kosiba@gmail.com
//

#include "inet/mobility/single/GaussMarkovMobility.h"

namespace inet {

Define_Module(GaussMarkovMobility);

GaussMarkovMobility::GaussMarkovMobility()
{
}

void GaussMarkovMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    EV_TRACE << "initializing GaussMarkovMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        speedMean = par("speed");
        speedStdDev = par("speedStdDev");
        angleMean = deg(fmod(deg(par("angle")).get(), 360.0));
        angleStdDev = deg(par("angleStdDev"));
        alpha = par("alpha");
        if (alpha < 0.0 || alpha > 1.0)
            throw cRuntimeError("The parameter 'alpha' is out of [0;1] interval");
        // constrain alpha to [0.0;1.0]
        margin = par("margin");
        speed = speedMean;
        angle = angleMean;
        stationary = (speed == 0);
    }
}

void GaussMarkovMobility::preventBorderHugging()
{
    bool left = (lastPosition.x < constraintAreaMin.x + margin);
    bool right = (lastPosition.x >= constraintAreaMax.x - margin);
    bool top = (lastPosition.y < constraintAreaMin.y + margin);
    bool bottom = (lastPosition.y >= constraintAreaMax.y - margin);
    if (top || bottom) {
        angleMean = bottom ? deg(270.0) : deg(90.0);
        if (right)
            angleMean -= deg(45.0);
        else if (left)
            angleMean += deg(45.0);
    }
    else if (left)
        angleMean = deg(0.0);
    else if (right)
        angleMean = deg(180.0);
}

void GaussMarkovMobility::move()
{
    preventBorderHugging();
    LineSegmentsMobilityBase::move();
    handleIfOutside(REFLECT, lastPosition, lastVelocity, angle);
}

void GaussMarkovMobility::setTargetPosition()
{
    // calculate new speed and direction based on the model
    speed = alpha * speed
        + (1.0 - alpha) * speedMean
        + sqrt(1.0 - alpha * alpha) * normal(0.0, 1.0) * speedStdDev;

    angle = alpha * angle
        + (1.0 - alpha) * angleMean
        + rad(sqrt(1.0 - alpha * alpha) * normal(0.0, 1.0) * angleStdDev);

    Coord direction(cos(rad(angle).get()), sin(rad(angle).get()));
    nextChange = simTime() + updateInterval;
    targetPosition = lastPosition + direction * (speed * updateInterval.dbl());

    EV_DEBUG << " speed = " << speed << " angle = " << angle << endl;
    EV_DEBUG << " mspeed = " << speedMean << " mangle = " << angleMean << endl;
}

} // namespace inet

