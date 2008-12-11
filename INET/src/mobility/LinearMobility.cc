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

#include "LinearMobility.h"
#include "FWMath.h"


Define_Module(LinearMobility);


void LinearMobility::initialize(int stage)
{
    BasicMobility::initialize(stage);

    EV << "initializing LinearMobility stage " << stage << endl;

    if (stage == 0)
    {
        updateInterval = par("updateInterval");
        speed = par("speed");
        angle = par("angle");
        acceleration = par("acceleration");
        angle = fmod(angle,360);

        // if the initial speed is lower than 0, the node is stationary
        stationary = (speed == 0);

        // host moves the first time after some random delay to avoid synchronized movements
        if (!stationary)
            scheduleAt(simTime() + uniform(0, updateInterval), new cMessage("move"));
    }
}


/**
 * The only self message possible is to indicate a new movement. If
 * host is stationary this function is never called.
 */
void LinearMobility::handleSelfMsg(cMessage * msg)
{
    move();
    updatePosition();
    if (!stationary)
        scheduleAt(simTime() + updateInterval, msg);
}

/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position
 */
void LinearMobility::move()
{
    pos.x += speed * cos(PI * angle / 180) * updateInterval;
    pos.y += speed * sin(PI * angle / 180) * updateInterval;

    // do something if we reach the wall
    Coord dummy;
    handleIfOutside(REFLECT, dummy, dummy, angle);

    // accelerate
    speed += acceleration * updateInterval;
    if (speed <= 0)
    {
        speed = 0;
        stationary = true;
    }

    EV << " xpos= " << pos.x << " ypos=" << pos.y << " speed=" << speed << endl;
}
