//
// Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

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

