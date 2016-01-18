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

#include "inet/mobility/base/MovingMobilityBase.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"

namespace inet {

using namespace inet::physicalenvironment;

MovingMobilityBase::MovingMobilityBase() :
    moveTimer(nullptr),
    updateInterval(0),
    stationary(false),
    lastSpeed(Coord::ZERO),
    lastUpdate(0),
    nextChange(-1),
    leaveMovementTrail(false),
    movementTrail(nullptr)
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
        if (leaveMovementTrail) {
            movementTrail = new TrailFigure(100, true, "movement trail");
            cCanvas *canvas = visualRepresentation->getParentModule()->getCanvas();
            canvas->addFigureBelow(movementTrail, canvas->getSubmodulesLayer());
        }
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
    if (leaveMovementTrail && visualRepresentation && hasGUI()) {
        cFigure::Point startPosition;
        if (movementTrail->getNumFigures() == 0)
            startPosition = IPhysicalEnvironment::computeCanvasPoint(lastPosition);
        else
            startPosition = static_cast<cLineFigure *>(movementTrail->getFigure(movementTrail->getNumFigures() - 1))->getEnd();
        cFigure::Point endPosition = IPhysicalEnvironment::computeCanvasPoint(lastPosition);
        double dx = startPosition.x - endPosition.x;
        double dy = startPosition.y - endPosition.y;
        if (movementTrail->getNumFigures() == 0 || dx * dx + dy * dy > (lastSpeed * updateInterval.dbl()).squareLength()) {
            cLineFigure *movementLine = new cLineFigure();
            movementLine->setTags("movement_trail recent_history");
            movementLine->setStart(startPosition);
            movementLine->setEnd(endPosition);
            movementLine->setLineWidth(1);
#if OMNETPP_VERSION >= 0x500
            cFigure::Color color = cFigure::GOOD_DARK_COLORS[getId() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))];
            movementLine->setLineColor(color);
            movementLine->setZoomLineWidth(false);
#else
            movementLine->setLineColor(cFigure::BLACK);
#endif
            movementTrail->addFigure(movementLine);
        }
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

