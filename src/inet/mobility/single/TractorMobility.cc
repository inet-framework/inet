//
// Copyright (C) 2007 Peterpaul Klein Haneveld
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#include "inet/mobility/single/TractorMobility.h"

#include "inet/common/INETMath.h"

namespace inet {

Define_Module(TractorMobility);

TractorMobility::TractorMobility()
{
    speed = 0;
    x1 = 0;
    y1 = 0;
    x2 = 0;
    y2 = 0;
    rowCount = 0;
    step = 0;
}

void TractorMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    EV_TRACE << "initializing TractorMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        speed = par("speed");
        x1 = par("x1");
        y1 = par("y1");
        x2 = par("x2");
        y2 = par("y2");
        rowCount = par("rowCount");
        step = 0;
    }
}

void TractorMobility::setInitialPosition()
{
    lastPosition.x = x1;
    lastPosition.y = y1;
}

void TractorMobility::setTargetPosition()
{
    int sign;
    Coord positionDelta = Coord::ZERO;
    switch (step % 4) {
        case 0:
            positionDelta.x = x2 - x1;
            break;

        case 1:
        case 3:
            sign = (step / (2 * rowCount)) % 2 ? -1 : 1;
            positionDelta.y = (y2 - y1) / rowCount * sign;
            break;

        case 2:
            positionDelta.x = x1 - x2;
            break;
    }
    step++;
    targetPosition = lastPosition + positionDelta;
    nextChange = simTime() + positionDelta.length() / speed;
}

} // namespace inet

