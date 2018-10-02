/* -*- mode:c++ -*- ********************************************************
 * file:        MoBanLocal.cc
 *
 * author:      Majid Nabi <m.nabi@tue.nl>
 *
 *              http://www.es.ele.tue.nl/nes
 *
 * copyright:   (C) 2010 Electronic Systems group(ES),
 *              Eindhoven University of Technology (TU/e), the Netherlands.
 *
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:    MoBAN (Mobility Model for wireless Body Area Networks)
 * description:     Implementation of the local module of the MoBAN mobility model
 ***************************************************************************
 * Citation of the following publication is appreciated if you use MoBAN for
 * a publication of your own.
 *
 * M. Nabi, M. Geilen, T. Basten. MoBAN: A Configurable Mobility Model for Wireless Body Area Networks.
 * In Proc. of the 4th Int'l Conf. on Simulation Tools and Techniques, SIMUTools 2011, Barcelona, Spain, 2011.
 *
 * BibTeX:
 *        @inproceedings{MoBAN,
 *         author = "M. Nabi and M. Geilen and T. Basten.",
 *          title = "{MoBAN}: A Configurable Mobility Model for Wireless Body Area Networks.",
 *        booktitle = "Proceedings of the 4th Int'l Conf. on Simulation Tools and Techniques.",
 *        series = {SIMUTools '11},
 *        isbn = {978-963-9799-41-7},
 *        year = {2011},
 *        location = {Barcelona, Spain},
 *        publisher = {ICST} }
 *
 **************************************************************************/

#include "inet/common/INETMath.h"
#include "inet/mobility/group/MoBanLocal.h"

namespace inet {

Define_Module(MoBanLocal);

MoBanLocal::MoBanLocal()
{
    coordinator = nullptr;
    referencePosition = Coord::ZERO;
    radius = 0;
    speed = 0;
    maxSpeed = 0;
}

void MoBanLocal::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    EV_TRACE << "initializing MoBanLocal stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        WATCH_PTR(coordinator);
        WATCH(referencePosition);
        WATCH(radius);
        WATCH(speed);
    }
    else if (stage == INITSTAGE_SINGLE_MOBILITY)
        computeMaxSpeed();
}

void MoBanLocal::setInitialPosition()
{
    lastPosition = referencePosition;
}

void MoBanLocal::setTargetPosition()
{
    if (speed != 0) {
        // find a uniformly random position within a sphere around the reference point
        double x = uniform(-radius, radius);
        double y = uniform(-radius, radius);
        double z = uniform(-radius, radius);
        while (x * x + y * y + z * z > radius * radius) {
            x = uniform(-radius, radius);
            y = uniform(-radius, radius);
            z = uniform(-radius, radius);
        }

        targetPosition = referencePosition + Coord(x, y, z);
        Coord positionDelta = targetPosition - lastPosition;
        double distance = positionDelta.length();
        nextChange = simTime() + distance / speed;
    }
    else {
        targetPosition = lastPosition;
        nextChange = -1;
    }
}

void MoBanLocal::refreshDisplay() const
{
    if (hasGUI() && subjectModule != nullptr) {
        Coord coordinatorPosition = coordinator->getCurrentPosition();
        subjectModule->getDisplayString().setTagArg("p", 0, lastPosition.x + coordinatorPosition.x);
        subjectModule->getDisplayString().setTagArg("p", 1, lastPosition.y + coordinatorPosition.y);
    }
}

void MoBanLocal::computeMaxSpeed()
{
    maxSpeed = coordinator->getMaxSpeed();
}

void MoBanLocal::setMoBANParameters(Coord referencePoint, double radius, double speed)
{
    Enter_Method_Silent();
    this->referencePosition = referencePoint;
    this->radius = radius;
    this->speed = speed;
    setTargetPosition();
    lastVelocity = (targetPosition - lastPosition) / (nextChange - simTime()).dbl();
    scheduleUpdate();
}

Coord MoBanLocal::getCurrentPosition()
{
    return LineSegmentsMobilityBase::getCurrentPosition() + coordinator->getCurrentPosition();
}

Coord MoBanLocal::getCurrentVelocity()
{
    return LineSegmentsMobilityBase::getCurrentVelocity() + coordinator->getCurrentVelocity();
}


} // namespace inet

