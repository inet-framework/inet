//
// Author: Marcin Kosiba marcin.kosiba@gmail.com
// Copyright (C) 2009 Marcin Kosiba
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

#include "inet/mobility/single/GaussMarkovMobility.h"

namespace inet {

Define_Module(GaussMarkovMobility);

GaussMarkovMobility::GaussMarkovMobility()
{
    speed = 0;
    angle = 0;
    alpha = 0;
    margin = 0;
    speedMean = 0;
    angleMean = 0;
    variance = 0;
}

void GaussMarkovMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    EV_TRACE << "initializing GaussMarkovMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        speedMean = par("speed");
        angleMean = par("angle");
        alpha = par("alpha");
        margin = par("margin");
        variance = par("variance");
        angle = fmod(angle, 360);
        //constrain alpha to [0.0;1.0]
        alpha = fmax(0.0, alpha);
        alpha = fmin(1.0, alpha);

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
        angleMean = bottom ? 270.0 : 90.0;
        if (right)
            angleMean -= 45.0;
        else if (left)
            angleMean += 45.0;
    }
    else if (left)
        angleMean = 0.0;
    else if (right)
        angleMean = 180.0;
}

void GaussMarkovMobility::move()
{
    preventBorderHugging();
    LineSegmentsMobilityBase::move();
    Coord dummy;
    handleIfOutside(REFLECT, dummy, dummy, angle);
}

void GaussMarkovMobility::setTargetPosition()
{
    // calculate new speed and direction based on the model
    speed = alpha * speed
        + (1.0 - alpha) * speedMean
        + sqrt(1.0 - alpha * alpha)
        * normal(0.0, 1.0)
        * variance;

    angle = alpha * angle
        + (1.0 - alpha) * angleMean
        + sqrt(1.0 - alpha * alpha)
        * normal(0.0, 1.0)
        * variance;

    double rad = M_PI * angle / 180.0;
    Coord direction(cos(rad), sin(rad));
    nextChange = simTime() + updateInterval;
    targetPosition = lastPosition + direction * speed * updateInterval.dbl();

    EV_DEBUG << " speed = " << speed << " angle = " << angle << endl;
    EV_DEBUG << " mspeed = " << speedMean << " mangle = " << angleMean << endl;
}

} // namespace inet

