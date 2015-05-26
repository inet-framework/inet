/* -*- mode:c++ -*- ********************************************************
 * file:        MobilityBase.cc
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

#include "inet/common/INETMath.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/mobility/base/MobilityBase.h"

namespace inet {

using namespace inet::physicalenvironment;

Register_Abstract_Class(MobilityBase);

static bool parseIntTo(const char *s, double& destValue)
{
    if (!s || !*s)
        return false;

    /* This method is only used to convert positions from the display strings,
     * which can contain floating point values.
     */
    if(sscanf(s, "%lf", &destValue) != 1)
        return false;

    return true;
}

static bool isFiniteNumber(double value)
{
    return value <= DBL_MAX && value >= -DBL_MAX;
}

MobilityBase::MobilityBase() :
    visualRepresentation(nullptr),
    constraintAreaMin(Coord::ZERO),
    constraintAreaMax(Coord::ZERO),
    lastPosition(Coord::ZERO),
    lastOrientation(EulerAngles::ZERO)
{
}

void MobilityBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    EV_TRACE << "initializing MobilityBase stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        constraintAreaMin.x = par("constraintAreaMinX");
        constraintAreaMin.y = par("constraintAreaMinY");
        constraintAreaMin.z = par("constraintAreaMinZ");
        constraintAreaMax.x = par("constraintAreaMaxX");
        constraintAreaMax.y = par("constraintAreaMaxY");
        constraintAreaMax.z = par("constraintAreaMaxZ");
        visualRepresentation = findVisualRepresentation();
        if (visualRepresentation) {
            const char *s = visualRepresentation->getDisplayString().getTagArg("p", 2);
            if (s && *s)
                throw cRuntimeError("The coordinates of '%s' are invalid. Please remove automatic arrangement"
                                    " (3rd argument of 'p' tag) from '@display' attribute.", visualRepresentation->getFullPath().c_str());
        }
        WATCH(constraintAreaMin);
        WATCH(constraintAreaMax);
        WATCH(lastPosition);
    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT_2) {
        initializeOrientation();
        initializePosition();
    }
}

void MobilityBase::initializePosition()
{
    setInitialPosition();
    checkPosition();
    emitMobilityStateChangedSignal();
    updateVisualRepresentation();
}

void MobilityBase::setInitialPosition()
{
    // reading the coordinates from omnetpp.ini makes predefined scenarios a lot easier
    bool filled = false;
    if (hasPar("initFromDisplayString") && par("initFromDisplayString").boolValue() && visualRepresentation) {
        filled = parseIntTo(visualRepresentation->getDisplayString().getTagArg("p", 0), lastPosition.x)
            && parseIntTo(visualRepresentation->getDisplayString().getTagArg("p", 1), lastPosition.y);
        if (filled)
            lastPosition.z = 0;
    }
    // not all mobility models have "initialX", "initialY" and "initialZ" parameters
    else if (hasPar("initialX") && hasPar("initialY") && hasPar("initialZ")) {
        lastPosition.x = par("initialX");
        lastPosition.y = par("initialY");
        lastPosition.z = par("initialZ");
        filled = true;
    }
    if (!filled)
        lastPosition = getRandomPosition();
}

void MobilityBase::checkPosition()
{
    if (!isFiniteNumber(lastPosition.x) || !isFiniteNumber(lastPosition.y) || !isFiniteNumber(lastPosition.z))
        throw cRuntimeError("Mobility position is not a finite number after initialize (x=%g,y=%g,z=%g)", lastPosition.x, lastPosition.y, lastPosition.z);
    if (isOutside())
        throw cRuntimeError("Mobility position (x=%g,y=%g,z=%g) is outside the constraint area (%g,%g,%g - %g,%g,%g)",
                lastPosition.x, lastPosition.y, lastPosition.z,
                constraintAreaMin.x, constraintAreaMin.y, constraintAreaMin.z,
                constraintAreaMax.x, constraintAreaMax.y, constraintAreaMax.z);
}

void MobilityBase::initializeOrientation()
{
    if (hasPar("initialAlpha") && hasPar("initialBeta") && hasPar("initialGamma")) {
        lastOrientation.alpha = par("initialAlpha");
        lastOrientation.beta = par("initialBeta");
        lastOrientation.gamma = par("initialGamma");
    }
}

void MobilityBase::handleMessage(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else
        throw cRuntimeError("Mobility modules can only receive self messages");
}

void MobilityBase::updateVisualRepresentation()
{
    EV_DEBUG << "current position = " << lastPosition << endl;
    if (hasGUI()) {
        char buf[80];
        sprintf(buf, "%.3g, %.3g, %.3g", lastPosition.x, lastPosition.y, lastPosition.z);
        getDisplayString().setTagArg("t", 0, buf);
        if (visualRepresentation != nullptr) {
            cFigure::Point point = IPhysicalEnvironment::computeCanvasPoint(lastPosition);
            char buf[32];
            snprintf(buf, sizeof(buf), "%lf", point.x);
            buf[sizeof(buf) - 1] = 0;
            visualRepresentation->getDisplayString().setTagArg("p", 0, buf);
            snprintf(buf, sizeof(buf), "%lf", point.y);
            buf[sizeof(buf) - 1] = 0;
            visualRepresentation->getDisplayString().setTagArg("p", 1, buf);
        }
    }
}

void MobilityBase::emitMobilityStateChangedSignal()
{
    emit(mobilityStateChangedSignal, this);
}

Coord MobilityBase::getRandomPosition()
{
    Coord p;
    p.x = uniform(constraintAreaMin.x, constraintAreaMax.x);
    p.y = uniform(constraintAreaMin.y, constraintAreaMax.y);
    p.z = uniform(constraintAreaMin.z, constraintAreaMax.z);
    return p;
}

bool MobilityBase::isOutside()
{
    return lastPosition.x < constraintAreaMin.x || lastPosition.x > constraintAreaMax.x
           || lastPosition.y < constraintAreaMin.y || lastPosition.y > constraintAreaMax.y
           || lastPosition.z < constraintAreaMin.z || lastPosition.z > constraintAreaMax.z;
}

static int reflect(double min, double max, double& coordinate, double& speed)
{
    double size = max - min;
    double value = coordinate - min;
    int sign = 1 - math::modulo(floor(value / size), 2) * 2;
    ASSERT(sign == 1 || sign == -1);
    coordinate = math::modulo(sign * value, size) + min;
    speed = sign * speed;
    return sign;
}

void MobilityBase::reflectIfOutside(Coord& targetPosition, Coord& speed, double& angle)
{
    int sign;
    double dummy;
    if (lastPosition.x < constraintAreaMin.x || constraintAreaMax.x < lastPosition.x) {
        sign = reflect(constraintAreaMin.x, constraintAreaMax.x, lastPosition.x, speed.x);
        reflect(constraintAreaMin.x, constraintAreaMax.x, targetPosition.x, dummy);
        angle = 90 + sign * (angle - 90);
    }
    if (lastPosition.y < constraintAreaMin.y || constraintAreaMax.y < lastPosition.y) {
        sign = reflect(constraintAreaMin.y, constraintAreaMax.y, lastPosition.y, speed.y);
        reflect(constraintAreaMin.y, constraintAreaMax.y, targetPosition.y, dummy);
        angle = sign * angle;
    }
    if (lastPosition.z < constraintAreaMin.z || constraintAreaMax.z < lastPosition.z) {
        sign = reflect(constraintAreaMin.z, constraintAreaMax.z, lastPosition.z, speed.z);
        reflect(constraintAreaMin.z, constraintAreaMax.z, targetPosition.z, dummy);
        // NOTE: angle is not affected
    }
}

static void wrap(double min, double max, double& coordinate)
{
    coordinate = math::modulo(coordinate - min, max - min) + min;
}

void MobilityBase::wrapIfOutside(Coord& targetPosition)
{
    if (lastPosition.x < constraintAreaMin.x || constraintAreaMax.x < lastPosition.x) {
        wrap(constraintAreaMin.x, constraintAreaMax.x, lastPosition.x);
        wrap(constraintAreaMin.x, constraintAreaMax.x, targetPosition.x);
    }
    if (lastPosition.y < constraintAreaMin.y || constraintAreaMax.y < lastPosition.y) {
        wrap(constraintAreaMin.y, constraintAreaMax.y, lastPosition.y);
        wrap(constraintAreaMin.y, constraintAreaMax.y, targetPosition.y);
    }
    if (lastPosition.z < constraintAreaMin.z || constraintAreaMax.z < lastPosition.z) {
        wrap(constraintAreaMin.z, constraintAreaMax.z, lastPosition.z);
        wrap(constraintAreaMin.z, constraintAreaMax.z, targetPosition.z);
    }
}

void MobilityBase::placeRandomlyIfOutside(Coord& targetPosition)
{
    if (isOutside()) {
        Coord newPosition = getRandomPosition();
        targetPosition += newPosition - lastPosition;
        lastPosition = newPosition;
    }
}

void MobilityBase::raiseErrorIfOutside()
{
    if (isOutside()) {
        throw cRuntimeError("Mobility moved outside the area %g,%g,%g - %g,%g,%g (x=%g,y=%g,z=%g)",
                constraintAreaMin.x, constraintAreaMin.y, constraintAreaMin.z,
                constraintAreaMax.x, constraintAreaMax.y, constraintAreaMax.z,
                lastPosition.x, lastPosition.y, lastPosition.z);
    }
}

void MobilityBase::handleIfOutside(BorderPolicy policy, Coord& targetPosition, Coord& speed, double& angle)
{
    switch (policy) {
        case REFLECT:
            reflectIfOutside(targetPosition, speed, angle);
            break;

        case WRAP:
            wrapIfOutside(targetPosition);
            break;

        case PLACERANDOMLY:
            placeRandomlyIfOutside(targetPosition);
            break;

        case RAISEERROR:
            raiseErrorIfOutside();
            break;

        default:
            throw cRuntimeError("Invalid outside policy=%d in module", policy, getFullPath().c_str());
    }
}

} // namespace inet

