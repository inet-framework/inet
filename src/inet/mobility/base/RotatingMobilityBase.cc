//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/base/RotatingMobilityBase.h"

namespace inet {

RotatingMobilityBase::RotatingMobilityBase()
{
    rotateTimer = nullptr;
    updateInterval = 0;
    stationary = false;
    lastUpdate = 0;
    nextChange = -1;
}

RotatingMobilityBase::~RotatingMobilityBase()
{
    cancelAndDelete(rotateTimer);
}

void RotatingMobilityBase::initialize(int stage)
{
    MobilityBase::initialize(stage);

    EV_TRACE << "initializing RotatingMobilityBase stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        rotateTimer = new cMessage("rotate");
        updateInterval = par("updateInterval");
    }
}

void RotatingMobilityBase::initializeOrientation()
{
    MobilityBase::initializeOrientation();
    lastUpdate = simTime();
    scheduleUpdate();
}

void RotatingMobilityBase::rotateAndUpdate()
{
    simtime_t now = simTime();
    if (nextChange == now || lastUpdate != now) {
        rotate();
        lastUpdate = simTime();
        emitMobilityStateChangedSignal();
    }
}

void RotatingMobilityBase::handleSelfMessage(cMessage *message)
{
    rotateAndUpdate();
    scheduleUpdate();
}

void RotatingMobilityBase::scheduleUpdate()
{
    cancelEvent(rotateTimer);
    if (!stationary && updateInterval != 0) {
        // periodic update is needed
        simtime_t nextUpdate = simTime() + updateInterval;
        if (nextChange != -1 && nextChange < nextUpdate)
            // next change happens earlier than next update
            scheduleAt(nextChange, rotateTimer);
        else
            // next update happens earlier than next change or there is no change at all
            scheduleAt(nextUpdate, rotateTimer);
    }
    else if (nextChange != -1)
        // no periodic update is needed
        scheduleAt(nextChange, rotateTimer);
}

const Quaternion& RotatingMobilityBase::getCurrentAngularPosition()
{
    rotateAndUpdate();
    return lastOrientation;
}

const Quaternion& RotatingMobilityBase::getCurrentAngularVelocity()
{
    rotateAndUpdate();
    return lastAngularVelocity;
}

} // namespace inet

