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


/**
 * Reads the updateInterval and the velocity
 *
 * If the host is not stationary it calculates a random position and
 * schedules a timer to trigger the first movement
 */
void ConstSpeedMobility::initialize(int stage)
{
    BasicMobility::initialize(stage);

    EV << "initializing ConstSpeedMobility stage " << stage << endl;

    if (stage == 0)
    {
        updateInterval = par("updateInterval");
        vHost = par("vHost");

        // if the initial speed is lower than 0, the node is stationary
        stationary = (vHost <= 0);

        //calculate the target position of the host if the host moves
        if (!stationary)
        {
            setTargetPosition();
            //host moves the first time after some random delay to avoid synchronized movements
            scheduleAt(simTime() + uniform(0, updateInterval), new cMessage("move"));
        }
    }
}


/**
 * The only self message possible is to indicate a new movement. If
 * host is stationary this function is never called.
 */
void ConstSpeedMobility::handleSelfMsg(cMessage * msg)
{
    move();
    updatePosition();
    scheduleAt(simTime() + updateInterval, msg);
}


/**
 * Calculate a new random position and the number of steps the host
 * needs to reach this position
 */
void ConstSpeedMobility::setTargetPosition()
{
    targetPos = getRandomPosition();
    double distance = pos.distance(targetPos);
    double totalTime = distance / vHost;
    numSteps = FWMath::round(totalTime / updateInterval);

    stepSize = (targetPos - pos) / numSteps;

    step = 0;

    EV << "distance=" << distance << " xpos= " << targetPos.x << " ypos=" << targetPos.y
        << "totalTime=" << totalTime << " numSteps=" << numSteps << " vHost=" << vHost << endl;
}


/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position
 */
void ConstSpeedMobility::move()
{
    step++;

    if (step <= numSteps)
    {
        EV << "stepping forward. step #=" << step << " xpos= " << pos.x << " ypos=" << pos.
            y << endl;

        pos += stepSize;
    }
    else
    {
        EV << "destination reached.\n xpos= " << pos.x << " ypos=" << pos.y << endl;

        setTargetPosition();
    }
}
