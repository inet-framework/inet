//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
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

#include "inet/mobility/group/MoBanLocal.h"

#include "inet/common/INETMath.h"

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
        WATCH(lastCompositePosition);
        WATCH(lastCompositeVelocity);
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
    Enter_Method("setMoBANParameters");
    this->referencePosition = referencePoint;
    this->radius = radius;
    this->speed = speed;
    setTargetPosition();
    lastVelocity = (targetPosition - lastPosition) / (nextChange - simTime()).dbl();
    scheduleUpdate();
}

const Coord& MoBanLocal::getCurrentPosition()
{
    lastCompositePosition = LineSegmentsMobilityBase::getCurrentPosition() + coordinator->getCurrentPosition();
    return lastCompositePosition;
}

const Coord& MoBanLocal::getCurrentVelocity()
{
    lastCompositeVelocity = LineSegmentsMobilityBase::getCurrentVelocity() + coordinator->getCurrentVelocity();
    return lastCompositeVelocity;
}

} // namespace inet

