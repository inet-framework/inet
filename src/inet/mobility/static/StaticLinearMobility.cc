//
// Copyright (C) 2012 Alfonso Ariza, Universidad de Malaga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/mobility/static/StaticLinearMobility.h"

namespace inet {

Define_Module(StaticLinearMobility);

StaticLinearMobility::StaticLinearMobility()
{
    initialX = 0;
    initialY = 0;
    separation = 0;
    orientation = deg(0);
}

void StaticLinearMobility::initialize(int stage)
{
    StationaryMobilityBase::initialize(stage);
    EV_TRACE << "initializing StaticLinearMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        initialX = par("initialX");
        initialY = par("initialY");
        separation = par("separation");
        orientation = deg(par("orientation"));
    }
}

void StaticLinearMobility::setInitialPosition()
{
    int index = subjectModule->getIndex();
    Coord initialPos;
    initialPos.x = initialX;
    initialPos.y = initialY;
    initialPos.z = 0;
    Coord direction(cos(rad(orientation).get()), sin(rad(orientation).get()));
    lastPosition = initialPos + (direction * (index * separation));
    if (lastPosition.x >= constraintAreaMax.x)
        lastPosition.x -= 1;
    if (lastPosition.y >= constraintAreaMax.y)
        lastPosition.y -= 1;
    lastPosition.z = 0;
}

void StaticLinearMobility::finish()
{
    StationaryMobilityBase::finish();
    recordScalar("x", lastPosition.x);
    recordScalar("y", lastPosition.y);
}

} // namespace inet

