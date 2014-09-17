//
// Copyright (C) 2007 Peterpaul Klein Haneveld
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "inet/common/INETMath.h"
#include "inet/mobility/single/TractorMobility.h"

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

