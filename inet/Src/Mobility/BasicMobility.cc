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


#define coreEV (ev.disabled()||!coreDebug) ? (std::ostream&)ev : EV << logName() << "::BasicMobility: "

static int parseInt(const char *s, int defaultValue)
{
    if (!s || !*s)
        return defaultValue;
    char *endptr;
    int value = strtol(s, &endptr, 10);
    return *endptr=='\0' ? value : defaultValue;
}


void BasicMobility::initialize(int stage)
{
    BasicModule::initialize(stage);

    EV << "initializing BasicMobility stage " << stage << endl;

    if (stage == 0)
    {
        cc = dynamic_cast<ChannelControl *>(simulation.moduleByPath("channelcontrol"));
        if (cc == 0)
            error("Could not find channelcontrol module");

        // get a pointer to the host
        hostPtr = findHost();
        myHostRef = cc->registerHost(hostPtr, Coord());
    }
    else if (stage == 1)
    {
        // playground size gets set up by ChannelControl in stage==0 (Andras)
        // read the playgroundsize from ChannelControl
        Coord pgs = cc->getPgs();

        // reading the coordinates from omnetpp.ini makes predefined scenarios a lot easier
        // -1 indicates start at display string position, or random position if it's not present
        pos.x = pos.y = -1;
        if (hasPar("x"))   // not all mobility models have an "x" parameter
            pos.x = par("x");
        if (pos.x == -1)
            pos.x = parseInt(hostPtr->displayString().getTagArg("p",0), -1);
        if (pos.x == -1)
            pos.x = genk_uniform(0, 0, pgs.x);

        if (hasPar("y")) // not all mobility models have an "y" parameter
            pos.y = par("y");
        if (pos.y == -1)
            pos.y = parseInt(hostPtr->displayString().getTagArg("p",1), -1);
        if (pos.y == -1)
            pos.y = genk_uniform(0, 0, pgs.y);

        // check validity of position
        if (pos.x < 0 || pos.y < 0 || pos.x >= pgs.x || pos.y >= pgs.y)
            error("node position (%d,%d) is outside the playground", pos.x, pos.y);

        // adjusting the display string is no longer needed (Andras)

        // print new host position on the screen and update bb info
        updatePosition();
    }
}

void BasicMobility::handleMessage(cMessage * msg)
{
    if (!msg->isSelfMessage())
        error("mobility modules can only receive self messages");

    handleSelfMsg(msg);
}


void BasicMobility::updatePosition()
{
    cc->updateHostPosition(myHostRef, pos);

    double r = cc->getCommunicationRange(myHostRef);
    hostPtr->displayString().setTagArg("p", 0, (long) pos.x);
    hostPtr->displayString().setTagArg("p", 1, (long) pos.y);
    hostPtr->displayString().setTagArg("r", 0, (long) r);

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
