/***************************************************************************
 * file:        ConstSpeedMobility.cc
 *
 * author:      Steffen Sroka
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/


#include "RestrictedConstSpeedMobility.h"

#include "FWMath.h"


Define_Module(RestrictedConstSpeedMobility);


/**
 * Reads the updateInterval and the velocity
 *
 * If the host is not stationary it calculates a random position and
 * schedules a timer to trigger the first movement
 */
void RestrictedConstSpeedMobility::initialize(int stage)
{

    ConstSpeedMobility::initialize(stage);
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
         	setTargetPosition();
        }
        if (targetPos.x<x1 || targetPos.x>x2 || targetPos.y<y1 || targetPos.y>y2)
        {
         	setTargetPosition();
        }
    }
    EV << "initializing RestrictedConstSpeed stage " << stage << endl;


}


Coord RestrictedConstSpeedMobility::getRandomPosition()
{
    Coord p;
    p.x = uniform(x1,x2);
    p.y = uniform(y1,y2);
    return p;
}

