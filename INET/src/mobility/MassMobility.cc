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


#include "MassMobility.h"
#include "FWMath.h"


#define MK_UPDATE_POS 100
#define MK_CHANGE_DIR 101

Define_Module(MassMobility);


/**
 * Reads the updateInterval and the velocity
 *
 * If the host is not stationary it calculates a random position and
 * schedules a timer to trigger the first movement
 */
void MassMobility::initialize(int stage)
{
    BasicMobility::initialize(stage);

    EV << "initializing MassMobility stage " << stage << endl;

    if (stage == 0)
    {
        updateInterval = par("updateInterval");

        changeInterval = &par("changeInterval");
        changeAngleBy = &par("changeAngleBy");
        speed = &par("speed");

        // initial speed and angle
        currentSpeed = speed->doubleValue();
        currentAngle = uniform(0, 360);
        step.x = currentSpeed * cos(PI * currentAngle / 180) * updateInterval;
        step.y = currentSpeed * sin(PI * currentAngle / 180) * updateInterval;

        scheduleAt(simTime() + uniform(0, updateInterval), new cMessage("move", MK_UPDATE_POS));
        scheduleAt(simTime() + uniform(0, changeInterval->doubleValue()), new cMessage("turn", MK_CHANGE_DIR));
    }
}


/**
 * The only self message possible is to indicate a new movement.
 */
void MassMobility::handleSelfMsg(cMessage * msg)
{
    switch (msg->getKind())
    {
    case MK_UPDATE_POS:
        move();
        updatePosition();
        scheduleAt(simTime() + updateInterval, msg);
        break;
    case MK_CHANGE_DIR:
        currentAngle += changeAngleBy->doubleValue();
        currentSpeed = speed->doubleValue();
        step.x = currentSpeed * cos(PI * currentAngle / 180) * updateInterval;
        step.y = currentSpeed * sin(PI * currentAngle / 180) * updateInterval;
        scheduleAt(simTime() + changeInterval->doubleValue(), msg);
        break;
    default:
        opp_error("Unknown self message kind in MassMobility class");
        break;
    }

}

/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position
 */
void MassMobility::move()
{
    pos += step;

    // do something if we reach the wall
    Coord dummy;
    handleIfOutside(REFLECT, dummy, step, currentAngle);

    EV << " xpos= " << pos.x << " ypos=" << pos.y << " speed=" << currentSpeed << endl;
}

