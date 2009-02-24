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

#include "RectangleMobility.h"
#include "FWMath.h"


Define_Module(RectangleMobility);


/**
 * Reads the parameters.
 * If the host is not stationary it calculates a random position and
 * schedules a timer to trigger the first movement
 */
void RectangleMobility::initialize(int stage)
{
    BasicMobility::initialize(stage);

    EV << "initializing RectangleMobility stage " << stage << endl;

    if (stage == 1)
    {
        x1 = par("x1");
        y1 = par("y1");
        x2 = par("x2");
        y2 = par("y2");
        updateInterval = par("updateInterval");
        speed = par("speed");

        // if the initial speed is lower than 0, the node is stationary
        stationary = (speed == 0);

        // calculate helper vars
        double dx = x2-x1;
        double dy = y2-y1;
        corner1 = dx;
        corner2 = corner1 + dy;
        corner3 = corner2 + dx;
        corner4 = corner3 + dy;

        // determine start position
        double startPos = par("startPos");
        startPos = fmod(startPos,4);
        if (startPos<1)
            d = startPos*dx; // top side
        else if (startPos<2)
            d = corner1 + (startPos-1)*dy; // right side
        else if (startPos<3)
            d = corner2 + (startPos-2)*dx; // bottom side
        else
            d = corner3 + (startPos-3)*dy; // left side
        calculateXY();
        WATCH(d);
        updatePosition();

        // host moves the first time after some random delay to avoid synchronized movements
        if (!stationary)
            scheduleAt(simTime() + uniform(0, updateInterval), new cMessage("move"));
    }
}


/**
 * The only self message possible is to indicate a new movement. If
 * host is stationary this function is never called.
 */
void RectangleMobility::handleSelfMsg(cMessage * msg)
{
    move();
    updatePosition();
    scheduleAt(simTime() + updateInterval, msg);
}

void RectangleMobility::move()
{
    d += speed * updateInterval;
    while (d<0) d+=corner4;
    while (d>=corner4) d-=corner4;

    calculateXY();
    EV << " xpos= " << pos.x << " ypos=" << pos.y << " speed=" << speed << endl;
}

void RectangleMobility::calculateXY()
{
    if (d < corner1)
    {
        // top side
        pos.x = x1 + d;
        pos.y = y1;
    }
    else if (d < corner2)
    {
        // right side
        pos.x = x2;
        pos.y = y1 + d - corner1;
    }
    else if (d < corner3)
    {
        // bottom side
        pos.x = x2 - d + corner2;
        pos.y = y2;
    }
    else
    {
        // left side
        pos.x = x1;
        pos.y = y2 - d + corner3;
    }
}
