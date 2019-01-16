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

#include "inet/common/INETMath.h"
#include "inet/mobility/single/CircleMobility.h"

namespace inet {

Define_Module(CircleMobility);

CircleMobility::CircleMobility()
{
    cx = 0;
    cy = 0;
    cz = 0;
    r = -1;
    startAngle = deg(0);
    speed = 0;
    omega = 0;
    angle = deg(0);
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
        startAngle = deg(par("startAngle"));
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
    angle = startAngle + rad(omega * simTime().dbl());
    double cosAngle = cos(rad(angle).get());
    double sinAngle = sin(rad(angle).get());
    lastPosition.x = cx + r * cosAngle;
    lastPosition.y = cy + r * sinAngle;
    lastPosition.z = cz;
    lastVelocity.x = -sinAngle * speed;
    lastVelocity.y = cosAngle * speed;
    lastVelocity.z = 0;
    // do something if we reach the wall
    Coord dummyCoord;
    handleIfOutside(REFLECT, dummyCoord, dummyCoord);
}

} // namespace inet

