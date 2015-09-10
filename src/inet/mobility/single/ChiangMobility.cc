//
// Author: Marcin Kosiba marcin.kosiba@gmail.com
// Copyright (C) 2009 Marcin Kosiba
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

#include "inet/mobility/single/ChiangMobility.h"

namespace inet {

Define_Module(ChiangMobility);

static const double stateMatrix[3][3] = {
    { 0.7, 0.3, 0.0 },
    { 0.5, 0.0, 0.5 },
    { 0.0, 0.3, 0.7 }
};

ChiangMobility::ChiangMobility()
{
    speed = 0;
    stateTransitionUpdateInterval = 0;
    xState = 0;
    yState = 0;
}

void ChiangMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    EV_TRACE << "initializing ChiangMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        stateTransitionUpdateInterval = par("stateTransitionUpdateInterval");
        speed = par("speed");
        stationary = (speed == 0);
        xState = 1;
        yState = 1;
    }
}

int ChiangMobility::getNextStateIndex(int currentState)
{
    //we assume that the sum in each row is 1
    double sum = 0;
    double randomValue = uniform(0.0, 1.0);
    for (int i = 0; i < 3; i++) {
        sum += stateMatrix[currentState][i];
        if (sum >= randomValue)
            return i;
    }
    EV_DEBUG << " getNextStateIndex error! currentState= " << currentState << " value= " << randomValue << endl;
    return currentState;
}

void ChiangMobility::setTargetPosition()
{
    xState = getNextStateIndex(xState);
    yState = getNextStateIndex(yState);
    nextChange = simTime() + stateTransitionUpdateInterval;
    Coord direction(xState - 1, yState - 1);
    double length = direction.length();
    if (length)
        lastSpeed = direction / length * speed;
    else
        lastSpeed = Coord::ZERO;
    targetPosition = lastPosition - lastSpeed * stateTransitionUpdateInterval;
}

void ChiangMobility::move()
{
    LineSegmentsMobilityBase::move();
    double dummyAngle;
    Coord dummyPosition;
    handleIfOutside(REFLECT, dummyPosition, lastSpeed, dummyAngle);
}

} // namespace inet

