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


#include "MobilityBase.h"
#include "FWMath.h"


simsignal_t MobilityBase::mobilityStateChangedSignal = SIMSIGNAL_NULL;

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

MobilityBase::MobilityBase()
{
    visualRepresentation = NULL;
    constraintAreaMin = Coord::ZERO;
    constraintAreaMax = Coord::ZERO;
    lastPosition = Coord::ZERO;
}

void MobilityBase::initialize(int stage)
{
    BasicModule::initialize(stage);
    EV << "initializing MobilityBase stage " << stage << endl;
    if (stage == 0)
    {
        mobilityStateChangedSignal = registerSignal("mobilityStateChanged");
        Coord contraintAreaSize;
        contraintAreaSize.x = par("constraintAreaSizeX");
        contraintAreaSize.y = par("constraintAreaSizeY");
        contraintAreaSize.z = par("constraintAreaSizeZ");
        if (contraintAreaSize.x == -1)
            contraintAreaSize.x = INFINITY;
        if (contraintAreaSize.y == -1)
            contraintAreaSize.y = INFINITY;
        if (contraintAreaSize.z == -1)
            contraintAreaSize.z = INFINITY;
        constraintAreaMin.x = par("constraintAreaX");
        constraintAreaMin.y = par("constraintAreaY");
        constraintAreaMin.z = par("constraintAreaZ");
        constraintAreaMax = constraintAreaMin + contraintAreaSize;
        visualRepresentation = findVisualRepresentation();
        if (visualRepresentation) {
            const char *s = visualRepresentation->getDisplayString().getTagArg("p", 2);
            if (s && *s)
                error("The coordinates of '%s' are invalid. Please remove automatic arrangement"
                      " (3rd argument of 'p' tag) from '@display' attribute.", visualRepresentation->getFullPath().c_str());
        }
    }
    else if (stage == 1)
    {
        initializePosition();
        if (isOutside())
            throw cRuntimeError("node position (%g,%g,%g) is outside the constraint area", lastPosition.x, lastPosition.y, lastPosition.z);
    }
    else if (stage == 3) {
        emitMobilityStateChangedSignal();
        updateVisualRepresentation();
    }
}

void MobilityBase::initializePosition()
{
    // reading the coordinates from omnetpp.ini makes predefined scenarios a lot easier
    bool filled = false;
    if (hasPar("initFromDisplayString") && par("initFromDisplayString").boolValue() && visualRepresentation)
    {
        filled = parseIntTo(visualRepresentation->getDisplayString().getTagArg("p", 0), lastPosition.x)
              && parseIntTo(visualRepresentation->getDisplayString().getTagArg("p", 1), lastPosition.y);
        if (filled)
            lastPosition.z = 0;

    }
    // not all mobility models have "initialX", "initialY" and "initialZ" parameters
    else if (hasPar("initialX") && hasPar("initialY") && hasPar("initialZ"))
    {
        lastPosition.x = par("initialX");
        lastPosition.y = par("initialY");
        lastPosition.z = par("initialZ");
        filled = true;
    }
    if (!filled)
        lastPosition = getRandomPosition();
}

void MobilityBase::handleMessage(cMessage * message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else
        throw cRuntimeError("mobility modules can only receive self messages");
}

void MobilityBase::updateVisualRepresentation()
{
    if (ev.isGUI() && visualRepresentation)
    {
        visualRepresentation->getDisplayString().setTagArg("p", 0, (long)lastPosition.x);
        visualRepresentation->getDisplayString().setTagArg("p", 1, (long)lastPosition.y);
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

static int reflect(double min, double max, double &coordinate, double &speed)
{
    double size = max - min;
    double value = coordinate - min;
    int sign = 1 - FWMath::modulo(floor(value / size), 2) * 2;
    ASSERT(sign == 1 || sign == -1);
    coordinate = FWMath::modulo(sign * value, size) + min;
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

static void wrap(double min, double max, double &coordinate)
{
    coordinate = FWMath::modulo(coordinate - min, max - min) + min;
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
    if (isOutside())
    {
        Coord newPosition = getRandomPosition();
        targetPosition += newPosition - lastPosition;
        lastPosition = newPosition;
    }
}

void MobilityBase::raiseErrorIfOutside()
{
    if (isOutside())
    {
        throw cRuntimeError("node moved outside the area %g,%g,%g - %g,%g,%g (x=%g,y=%g,z=%g)",
              constraintAreaMin.x, constraintAreaMin.y, constraintAreaMin.z,
              constraintAreaMax.x, constraintAreaMax.y, constraintAreaMax.z,
              lastPosition.x, lastPosition.y, lastPosition.z);
    }
}

void MobilityBase::handleIfOutside(BorderPolicy policy, Coord& targetPosition, Coord& speed, double& angle)
{
    switch (policy)
    {
        case REFLECT:       reflectIfOutside(targetPosition, speed, angle); break;
        case WRAP:          wrapIfOutside(targetPosition); break;
        case PLACERANDOMLY: placeRandomlyIfOutside(targetPosition); break;
        case RAISEERROR:    raiseErrorIfOutside(); break;
        default:            throw cRuntimeError("Invalid outside policy=%d in module", policy, getFullPath().c_str());
    }
}
