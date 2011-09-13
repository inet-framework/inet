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


#include "ConstSpeedMobility.h"
#include "FWMath.h"


Define_Module(ConstSpeedMobility);


ConstSpeedMobility::ConstSpeedMobility()
{
    speed = 0;
}

void ConstSpeedMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);
    EV << "initializing ConstSpeedMobility stage " << stage << endl;
    if (stage == 0)
    {
        speed = par("speed");
        stationary = speed == 0;
    }
}

void ConstSpeedMobility::setTargetPosition()
{
    targetPosition = getRandomPosition();
    Coord positionDelta = targetPosition - lastPosition;
    double distance = positionDelta.length();
    nextChange = simTime() + distance / speed;
    EV << "new target set. distance=" << distance << " xpos= " << targetPosition.x << " ypos=" << targetPosition.y << " nextChange=" << nextChange;
}
