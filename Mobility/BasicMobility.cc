/* -*- mode:c++ -*- ********************************************************
 * file:        BasicMobility.cc
 *
 * author:      Daniel Willkomm, Andras Varga
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              (C) 2005 Andras Varga
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


#include "BasicMobility.h"
#include "FWMath.h"


#define coreEV (ev.disabled()||!coreDebug) ? (std::ostream&)ev : ev << logName() << "::BasicMobility: "

Define_Module(BasicMobility);

/**
 * Assigns a pointer to ChannelControl and gets a pointer to its host.
 *
 * Creates a random position for a host if the position is not given
 * as a parameter in "omnetpp.ini".
 *
 * Additionally the registration with ChannelControl is done and it is
 * assured that the position display string tag (p) exists and contains
 * the exact (x) tag.
 */
void BasicMobility::initialize(int stage)
{
    BasicModule::initialize(stage);

    coreEV << "initializing BasicMobility stage " << stage << endl;

    if (stage == 0)
    {
        cc = dynamic_cast<ChannelControl *>(simulation.moduleByPath("channelcontrol"));
        if (cc == 0)
            error("Could not find channelcontrol module");

        //get a pointer to the host
        hostPtr = findHost();
        myHostRef = cc->registerHost(hostPtr, Coord());
    }
    else if (stage == 1)
    {
        // playground size gets set up by ChannelControl in stage==0 (Andras)
        // read the playgroundsize from ChannelControl
        Coord pgs = cc->getPgs();

        // reading the out from omnetpp.ini makes predefined scenarios a lot easier
        if (hasPar("x") && hasPar("y"))
        {
            pos.x = par("x");
            pos.y = par("y");
            // -1 indicates start at random position
            if (pos.x == -1 || pos.y == -1)
                pos = getRandomPosition();
            //we do not have negative positions
            //also checks whether position is within the playground
            else if (pos.x < 0 || pos.y < 0 || pos.x > pgs.x || pos.y > pgs.y)
                error("node position specified in omnetpp.ini exceeds playgroundsize");
        }
        else
        {
            // Start at a random position
            pos = getRandomPosition();
        }

        // adjusting the display string is no longer needed (Andras)

        // print new host position on the screen and update bb info
        updatePosition();
    }
}

/**
 * Dispatches self-messages to handleSelfMsg()
 */
void BasicMobility::handleMessage(cMessage * msg)
{
    if (!msg->isSelfMessage())
        error("mobility modules can only receive self messages");

    handleSelfMsg(msg);
}


/**
 * This function tells NotificationBoard that the position has changed, and
 * it also moves the host's icon to the new position on the screen.
 *
 * This function has to be called every time the position of the host
 * changes!
 */
void BasicMobility::updatePosition()
{
    cc->updateHostPosition(myHostRef, pos);

    char xStr[32], yStr[32], rStr[32];
    sprintf(xStr, "%d", FWMath::round(pos.x));
    sprintf(yStr, "%d", FWMath::round(pos.y));
    sprintf(rStr, "%d", FWMath::round(cc->getCommunicationRange(myHostRef)));
    hostPtr->displayString().setTagArg("p", 0, xStr);
    hostPtr->displayString().setTagArg("p", 1, yStr);
    hostPtr->displayString().setTagArg("r", 0, rStr);

    nb->fireChangeNotification(NF_HOSTPOSITION_UPDATED, &pos);
}


/**
 * You can redefine this function if you want to use another
 * calculation
 */
Coord BasicMobility::getRandomPosition()
{
    Coord p;
    p.x = genk_uniform(0, 0, cc->getPgs()->x);
    p.y = genk_uniform(0, 0, cc->getPgs()->y);
    return p;
}

void BasicMobility::reflectIfOutside(Coord& targetPos, Coord& step, double& angle)
{
    if (pos.x < 0)
    {
        pos.x = -pos.x;
        targetPos.x = -targetPos.x;
        step.x = -step.x;
        angle = 180 - angle;
    }
    else if (pos.x >= playgroundSizeX())
    {
        pos.x = 2*playgroundSizeX() - pos.x;
        targetPos.x = 2*playgroundSizeX() - targetPos.x;
        step.x = -step.x;
        angle = 180 - angle;
    }
    if (pos.y < 0)
    {
        pos.y = -pos.y;
        targetPos.y = -targetPos.y;
        step.y = -step.y;
        angle = -angle;
    }
    else if (pos.y >= playgroundSizeY())
    {
        pos.y = 2*playgroundSizeY() - pos.y;
        targetPos.y = 2*playgroundSizeY() - targetPos.y;
        step.y = -step.y;
        angle = -angle;
    }
}

void BasicMobility::wrapIfOutside(Coord& targetPos)
{
    if (pos.x < 0)
    {
        pos.x += playgroundSizeX();
        targetPos.x += playgroundSizeX();
    }
    else if (pos.x >= playgroundSizeX())
    {
        pos.x -= playgroundSizeX();
        targetPos.x -= playgroundSizeX();
    }
    if (pos.y < 0)
    {
        pos.y += playgroundSizeY();
        targetPos.y += playgroundSizeY();
    }
    else if (pos.y >= playgroundSizeY())
    {
        pos.y -= playgroundSizeY();
        targetPos.y -= playgroundSizeY();
    }
}

void BasicMobility::placeRandomlyIfOutside(Coord& targetPos)
{
    if (pos.x<0 || pos.x>=playgroundSizeX() || pos.y<0 || pos.y>=playgroundSizeY())
    {
        Coord newPos = getRandomPosition();
        targetPos += newPos - pos;
        pos = newPos;
    }
}

void BasicMobility::raiseErrorIfOutside()
{
    if (pos.x<0 || pos.x>=playgroundSizeX() || pos.y<0 || pos.y>=playgroundSizeY())
    {
        error("node moved outside the playground of size %gx%g (x=%g,y=%g)",
              playgroundSizeX(), playgroundSizeY(), pos.x, pos.y);
    }
}

void BasicMobility::handleIfOutside(BorderPolicy policy, Coord& targetPos, Coord& step, double& angle)
{
    switch (policy)
    {
        case REFLECT:       reflectIfOutside(targetPos, step, angle); break;
        case WRAP:          wrapIfOutside(targetPos); break;
        case PLACERANDOMLY: placeRandomlyIfOutside(targetPos); break;
        case RAISEERROR:    raiseErrorIfOutside(); break;
    }
}
