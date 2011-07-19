/* -*- mode:c++ -*- ********************************************************
 * file:        MovingMobilityBase.cc
 *
 * author:      Daniel Willkomm, Andras Varga, Zoltan Bojthe
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              (C) 2005 Andras Varga
 *              (C) 2011 Zoltan Bojthe
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


#include "MovingMobilityBase.h"


MovingMobilityBase::MovingMobilityBase()
{
    updateInterval = 0;
    stationary = false;
    lastSpeed = Coord::ZERO;
    lastUpdate = 0;
    nextChange = -1;
}

void MovingMobilityBase::initialize(int stage)
{
    MobilityBase::initialize(stage);
    EV << "initializing MovingMobilityBase stage " << stage << endl;
    if (stage == 0)
        updateInterval = par("updateInterval");
    else if (stage == 3) {
        if (!stationary && updateInterval != 0)
            scheduleUpdate(simTime(), new cMessage("move"));
    }
}

void MovingMobilityBase::positionUpdated()
{
    lastUpdate = simTime();
    MobilityBase::positionUpdated();
}

void MovingMobilityBase::moveAndUpdate()
{
    if (lastUpdate != simTime()) {
        move();
        positionUpdated();
    }
}

void MovingMobilityBase::handleSelfMessage(cMessage *msg)
{
    moveAndUpdate();
    if (!stationary && updateInterval != 0)
        scheduleUpdate(simTime() + updateInterval, msg);
}

void MovingMobilityBase::scheduleUpdate(simtime_t nextUpdate, cMessage *message)
{
    scheduleAt(nextChange != -1 && nextUpdate > nextChange ? nextChange : nextUpdate, message);
}

Coord MovingMobilityBase::getCurrentPosition()
{
    moveAndUpdate();
    return lastPosition;
}

Coord MovingMobilityBase::getCurrentSpeed()
{
    moveAndUpdate();
    return lastSpeed;
}
