/*
 * Copyright (C) 2012 Alfonso Ariza, Universidad de Malaga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

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

