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
#include "PhysicalEnvironment.h"

namespace inet {

MovingMobilityBase::MovingMobilityBase() :
    moveTimer(NULL),
    updateInterval(0),
    stationary(false),
    lastSpeed(Coord::ZERO),
    lastUpdate(0),
    nextChange(-1),
    leaveMovementTrail(false),
    movementTrail(NULL)
{
}

MovingMobilityBase::~MovingMobilityBase()
{
    cancelAndDelete(moveTimer);
}

void MovingMobilityBase::initialize(int stage)
{
    MobilityBase::initialize(stage);
    EV_TRACE << "initializing MovingMobilityBase stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        moveTimer = new cMessage("move");
        updateInterval = par("updateInterval");
        leaveMovementTrail = par("leaveMovementTrail");
#ifdef __CCANVAS_H
        if (leaveMovementTrail) {
            movementTrail = new TrailLayer(100, "movement trail");
            visualRepresentation->getParentModule()->getCanvas()->addToplevelLayer(movementTrail);
        }
#endif
    }
}

void MovingMobilityBase::initializePosition()
{
    MobilityBase::initializePosition();
    lastUpdate = simTime();
    scheduleUpdate();
}

void MovingMobilityBase::moveAndUpdate()
{
    simtime_t now = simTime();
    if (nextChange == now || lastUpdate != now) {
        move();
        lastUpdate = simTime();
        emitMobilityStateChangedSignal();
        updateVisualRepresentation();
    }
}

void MovingMobilityBase::updateVisualRepresentation()
{
    MobilityBase::updateVisualRepresentation();
    if (leaveMovementTrail && visualRepresentation && ev.isGUI()) {
#ifdef __CCANVAS_H
        Coord endPosition = lastPosition;
        Coord startPosition;
        if (movementTrail->getNumChildren() == 0)
            startPosition = endPosition;
        else {
            cFigure::Point previousEnd = static_cast<cLineFigure *>(movementTrail->getChild(movementTrail->getNumChildren() - 1))->getEnd();
            startPosition.x = previousEnd.x;
            startPosition.y = previousEnd.y;
        }
        if (movementTrail->getNumChildren() == 0 || startPosition.distance(endPosition) > 10) {
            cLineFigure *movementLine = new cLineFigure();
            movementLine->setStart(PhysicalEnvironment::computeCanvasPoint(startPosition));
            movementLine->setEnd(PhysicalEnvironment::computeCanvasPoint(endPosition));
            movementLine->setLineColor(cFigure::BLACK);
            movementTrail->addChild(movementLine);
        }
#endif
    }
}

void MovingMobilityBase::handleSelfMessage(cMessage *message)
{
    moveAndUpdate();
    scheduleUpdate();
}

void MovingMobilityBase::scheduleUpdate()
{
    cancelEvent(moveTimer);
    if (!stationary && updateInterval != 0) {
        // periodic update is needed
        simtime_t nextUpdate = simTime() + updateInterval;
        if (nextChange != -1 && nextChange < nextUpdate)
            // next change happens earlier than next update
            scheduleAt(nextChange, moveTimer);
        else
            // next update happens earlier than next change or there is no change at all
            scheduleAt(nextUpdate, moveTimer);
    }
    else if (nextChange != -1)
        // no periodic update is needed
        scheduleAt(nextChange, moveTimer);
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

} // namespace inet

