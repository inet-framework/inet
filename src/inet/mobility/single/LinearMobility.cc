//
// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
// Copyright (C) 2005 Emin Ilker Cetinbas
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

#include "inet/common/INETMath.h"
#include "inet/mobility/single/LinearMobility.h"

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

        lastVelocity = direction * speed;
    }
}

void LinearMobility::move()
{
    double elapsedTime = (simTime() - lastUpdate).dbl();
    lastPosition += lastVelocity * elapsedTime;

    // do something if we reach the wall
    Coord dummyCoord;
    handleIfOutside(REFLECT, dummyCoord, lastVelocity);
}

} // namespace inet

