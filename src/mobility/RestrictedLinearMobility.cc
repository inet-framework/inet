//
// Author: Alfonso Ariza
// Copyright (C) 2009 Alfonso Ariza
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

#include "RestrictedLinearMobility.h"
#include "FWMath.h"


Define_Module(RestrictedLinearMobility);


void RestrictedLinearMobility::initialize(int stage)
{
    LinearMobility::initialize(stage);
    if (stage == 1)
    {
        x1 = par("x1");
        y1 = par("y1");
        x2 = par("x2");
        y2 = par("y2");

        if (x1<0)
            x1=0;
        if (y1<0)
            y1=0;
        if (x1>=x2)
        {
            x1=0;
            x2=getPlaygroundSizeX();
        }
        if (y1>=y2)
        {
            y1=0;
            y2=getPlaygroundSizeY();
        }
        if (x2>getPlaygroundSizeX())
            x2=getPlaygroundSizeX();
        if (y2>getPlaygroundSizeY())
            y2=getPlaygroundSizeY();
        if (pos.x<x1 || pos.x>x2 || pos.y<y1 || pos.y>y2)
        {
            pos = getRandomPosition();
            updatePosition();
        }
    }
    EV << "initializing RestrictedLinearMobility stage " << stage << endl;
}


Coord RestrictedLinearMobility::getRandomPosition()
{
    Coord p;
    p.x = uniform(x1,x2);
    p.y = uniform(y1,y2);
    return p;
}

void RestrictedLinearMobility::reflectIfOutside(Coord& targetPos, Coord& step, double& angle)
{
    if (pos.x < x1)
    {
        pos.x = 2*x1 - pos.x;
        targetPos.x = 2*x1 - targetPos.x;
        step.x = -step.x;
        angle = 180 - angle;
    }
    else if (pos.x >= x2)
    {
        pos.x = 2*x2 - pos.x;
        targetPos.x = 2*x2 - targetPos.x;
        step.x = -step.x;
        angle = 180 - angle;
    }
    if (pos.y < y1)
    {
        pos.y = 2*y1 - pos.y;
        targetPos.y = 2*y1-targetPos.y;
        step.y = -step.y;
        angle = -angle;
    }
    else if (pos.y >= y2)
    {
        pos.y = 2*y2 - pos.y;
        targetPos.y = 2*y2 - targetPos.y;
        step.y = -step.y;
        angle = -angle;
    }
}
