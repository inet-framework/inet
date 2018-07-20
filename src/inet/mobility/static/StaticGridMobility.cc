/*
 * Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
 * Copyright (C) 2013 OpenSim Ltd
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

#include "inet/mobility/static/StaticGridMobility.h"

namespace inet {

Define_Module(StaticGridMobility);

void StaticGridMobility::setInitialPosition()
{
    int numHosts = par("numHosts");
    double marginX = par("marginX");
    double marginY = par("marginY");
    double separationX = par("separationX");
    double separationY = par("separationY");
    int columns = par("columns");
    int rows = par("rows");
    if (numHosts > rows * columns)
        throw cRuntimeError("parameter error: numHosts > rows * columns");

    int index = subjectModule->getIndex();

    int row = index / columns;
    int col = index % columns;
    lastPosition.x = constraintAreaMin.x + marginX + (col + 0.5) * separationX;
    lastPosition.y = constraintAreaMin.y + marginY + (row + 0.5) * separationY;
    lastPosition.z = par("initialZ");
    recordScalar("x", lastPosition.x);
    recordScalar("y", lastPosition.y);
    recordScalar("z", lastPosition.z);
}

} // namespace inet

