//
// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
// Generalization: Andras Varga
// Copyright (C) 2005 Emin Ilker Cetinbas, Andras Varga
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

#include "inet/mobility/single/MassMobility.h"
#include "inet/common/INETMath.h"

namespace inet {

Define_Module(MassMobility);

MassMobility::MassMobility()
{
    changeIntervalParameter = NULL;
    changeAngleByParameter = NULL;
    speedParameter = NULL;
    angle = 0;
}

void MassMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    EV_TRACE << "initializing MassMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        angle = par("startAngle").doubleValue();
        changeIntervalParameter = &par("changeInterval");
        changeAngleByParameter = &par("changeAngleBy");
        speedParameter = &par("speed");
    }
}

void MassMobility::setTargetPosition()
{
    angle += changeAngleByParameter->doubleValue();
    EV_DEBUG << "angle: " << angle << endl;
    double rad = PI * angle / 180.0;
    Coord direction(cos(rad), sin(rad));
    simtime_t nextChangeInterval = changeIntervalParameter->doubleValue();
    EV_DEBUG << "interval: " << nextChangeInterval << endl;
    sourcePosition = lastPosition;
    targetPosition = lastPosition + direction * speedParameter->doubleValue() * nextChangeInterval.dbl();
    previousChange = simTime();
    nextChange = previousChange + nextChangeInterval;
}

void MassMobility::move()
{
    simtime_t now = simTime();
    if (now == nextChange) {
        lastPosition = targetPosition;
        handleIfOutside(REFLECT, lastPosition, lastSpeed, angle);
        EV_INFO << "reached current target position = " << lastPosition << endl;
        setTargetPosition();
        EV_INFO << "new target position = " << targetPosition << ", next change = " << nextChange << endl;
        lastSpeed = (targetPosition - lastPosition) / (nextChange - simTime()).dbl();
    }
    else if (now > lastUpdate) {
        ASSERT(nextChange == -1 || now < nextChange);
        double alpha = (now - previousChange) / (nextChange - previousChange);
        lastPosition = sourcePosition * (1 - alpha) + targetPosition * alpha;
        double dummyAngle;
        handleIfOutside(REFLECT, lastPosition, lastSpeed, dummyAngle);
    }
}

double MassMobility::getMaxSpeed() const
{
    return NaN;
}

} // namespace inet

