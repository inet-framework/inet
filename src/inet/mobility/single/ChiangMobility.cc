//
// Copyright (C) 2009 Marcin Kosiba
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author: Marcin Kosiba marcin.kosiba@gmail.com
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
    // we assume that the sum in each row is 1
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
        lastVelocity = direction / length * speed;
    else
        lastVelocity = Coord::ZERO;
    targetPosition = lastPosition - lastVelocity * stateTransitionUpdateInterval;
}

void ChiangMobility::move()
{
    LineSegmentsMobilityBase::move();
    Coord dummyCoord;
    handleIfOutside(REFLECT, dummyCoord, lastVelocity);
}

} // namespace inet

