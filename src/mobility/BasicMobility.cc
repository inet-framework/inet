/* -*- mode:c++ -*- ********************************************************
 * file:        BasicMobility.cc
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


#include "BasicMobility.h"

#include "FWMath.h"


#define coreEV (ev.isDisabled()||!coreDebug) ? (std::ostream&)ev : EV << logName() << "::BasicMobility: "

static bool parseIntTo(const char *s, double& destValue)
{
    if (!s || !*s)
        return false;

    char *endptr;
    int value = strtol(s, &endptr, 10);

    if (*endptr)
        return false;

    destValue = value;
    return true;
}

void BasicMobility::initPos()
{
    // reading the coordinates from omnetpp.ini makes predefined scenarios a lot easier

    bool filled = false;

    if (hasPar("initFromDisplayString") && par("initFromDisplayString").boolValue())
    {
        filled = parseIntTo(hostPtr->getDisplayString().getTagArg("p", 0), pos.x)
              && parseIntTo(hostPtr->getDisplayString().getTagArg("p", 1), pos.y);

    }
    else
    {
        // not all mobility models have "initialX" and "initialY" parameter
        if (hasPar("initialX") && hasPar("initialY"))
        {
            pos.x = par("initialX");
            pos.y = par("initialY");
            filled = true;
        }
    }

    if (!filled)
        pos = getRandomPosition();
}

void BasicMobility::initialize(int stage)
{
    BasicModule::initialize(stage);

    EV << "initializing BasicMobility stage " << stage << endl;

    if (stage == 0)
    {
        // get a pointer to the host
        hostPtr = findHost();

        const char *s = hostPtr->getDisplayString().getTagArg("p", 2);

        if (s && *s)
            error("The coordinates of '%s' host are invalid. Please remove automatic arrangement"
                    " (3rd argument of 'p' tag) from '@display' attribute.",
                    hostPtr->getFullPath().c_str());
    }
    else if (stage == 1)
    {
        areaTopLeft.y = par("constraintAreaY");
        areaBottomRight.x = par("constraintAreaWidth");
        areaTopLeft.x = par("constraintAreaX");
        areaBottomRight.y = par("constraintAreaHeight");
        areaBottomRight += areaTopLeft;

        initPos();

        // check validity of position
        if (isOutside())
            throw cRuntimeError("node position (%d,%d) is outside the playground", pos.x, pos.y);

        // adjusting the display string is no longer needed (Andras)

        // print new host position on the screen and update bb info
        positionUpdated();
    }
    else if (stage == 3)
    {
        positionUpdated();
    }
}

void BasicMobility::handleMessage(cMessage * msg)
{
    if (!msg->isSelfMessage())
        throw cRuntimeError("mobility modules can only receive self messages");

    handleSelfMsg(msg);
}


void BasicMobility::positionUpdated()
{
//    cc->updateHostPosition(myHostRef, pos);

    if (ev.isGUI())
    {
        hostPtr->getDisplayString().setTagArg("p", 0, (long)pos.x);
        hostPtr->getDisplayString().setTagArg("p", 1, (long)pos.y);
//        double r = cc->getCommunicationRange(myHostRef);
//        hostPtr->getDisplayString().setTagArg("r", 0, (long) r);
    }

    nb->fireChangeNotification(NF_HOSTPOSITION_UPDATED, &pos);
}

/**
 * You can redefine this function if you want to use another
 * calculation
 */
Coord BasicMobility::getRandomPosition()
{
    Coord p;
    p.x = uniform(areaTopLeft.x, areaBottomRight.x);
    p.y = uniform(areaTopLeft.y, areaBottomRight.y);
    return p;
}

void BasicMobility::reflectIfOutside(Coord& targetPos, Coord& step, double& angle)
{
    if (pos.x < areaTopLeft.x)
    {
        pos.x = 2 * areaTopLeft.x - pos.x;
        targetPos.x = 2 * areaTopLeft.x - targetPos.x;
        step.x = -step.x;
        angle = 180 - angle;
    }
    else if (pos.x >= areaBottomRight.x)
    {
        pos.x = 2 * areaBottomRight.x - pos.x;
        targetPos.x = 2 * areaBottomRight.x - targetPos.x;
        step.x = -step.x;
        angle = 180 - angle;
    }

    if (pos.y < areaTopLeft.y)
    {
        pos.y = 2 * areaTopLeft.y - pos.y;
        targetPos.y = 2 * areaTopLeft.y - targetPos.y;
        step.y = -step.y;
        angle = -angle;
    }
    else if (pos.y >= areaBottomRight.y)
    {
        pos.y = 2 * areaBottomRight.y - pos.y;
        targetPos.y = 2 * areaBottomRight.y - targetPos.y;
        step.y = -step.y;
        angle = -angle;
    }
}

void BasicMobility::wrapIfOutside(Coord& targetPos)
{
    if (pos.x < areaTopLeft.x)
    {
        pos.x += areaBottomRight.x - areaTopLeft.x;
        targetPos.x += areaBottomRight.x - areaTopLeft.x;
    }
    else if (pos.x >= areaBottomRight.x)
    {
        pos.x -= areaBottomRight.x - areaTopLeft.x;
        targetPos.x -= areaBottomRight.x - areaTopLeft.x;
    }
    if (pos.y < areaTopLeft.y)
    {
        pos.y += areaBottomRight.y - areaTopLeft.y;
        targetPos.y += areaBottomRight.y - areaTopLeft.y;
    }
    else if (pos.y >= areaBottomRight.y)
    {
        pos.y -= areaBottomRight.y - areaTopLeft.y;
        targetPos.y -= areaBottomRight.y - areaTopLeft.y;
    }
}

bool BasicMobility::isOutside()
{
    return pos.x < areaTopLeft.x || pos.x >= areaBottomRight.x
        || pos.y < areaTopLeft.y || pos.y >= areaBottomRight.y;
}

void BasicMobility::placeRandomlyIfOutside(Coord& targetPos)
{
    if (isOutside())
    {
        Coord newPos = getRandomPosition();
        targetPos += newPos - pos;
        pos = newPos;
    }
}

void BasicMobility::raiseErrorIfOutside()
{
    if (isOutside())
    {
        throw cRuntimeError("node moved outside the area %g,%g - %g,%g (x=%g,y=%g)",
              areaTopLeft.x, areaTopLeft.y, areaBottomRight.x, areaBottomRight.y, pos.x, pos.y);
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
        default:            throw cRuntimeError("Invalid outside policy=%d in module",
                                                policy, getFullPath().c_str());
    }
}

