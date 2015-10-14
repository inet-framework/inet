//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/mobility/single/CircleMobility.h"
#include "inet/common/INETMath.h"

namespace inet {

Define_Module(CircleMobility);

CircleMobility::CircleMobility()
{
    cx = 0;
    cy = 0;
    cz = 0;
    r = -1;
    startAngle = 0;
    speed = 0;
    omega = 0;
    angle = 0;
}

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
        startAngle = par("startAngle").doubleValue() / 180.0 * M_PI;
        speed = par("speed");
        omega = speed / r;
        stationary = (omega == 0);
    }
}

void CircleMobility::setInitialPosition()
{
    move();
}

void CircleMobility::move()
{
    angle = startAngle + omega * simTime().dbl();
    lastPosition.x = cx + r * cos(angle);
    lastPosition.y = cy + r * sin(angle);
    lastPosition.z = cz;
    lastSpeed.x = -sin(angle) * speed;
    lastSpeed.y = cos(angle) * speed;
    lastSpeed.z = 0;
    // do something if we reach the wall
    Coord dummyCoord;
    double dummyAngle;
    handleIfOutside(REFLECT, dummyCoord, dummyCoord, dummyAngle);
}

} // namespace inet

