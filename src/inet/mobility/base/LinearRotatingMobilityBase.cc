//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/base/LinearRotatingMobilityBase.h"

namespace inet {

LinearRotatingMobilityBase::LinearRotatingMobilityBase()
{
}

void LinearRotatingMobilityBase::initializeOrientation()
{
    MobilityBase::initializeOrientation();
    if (!stationary) {
        setTargetOrientation();
        EV_INFO << "current target orientation = " << targetOrientation << ", next change = " << nextChange << endl;
    }
    lastUpdate = simTime();
    scheduleUpdate();
}

void LinearRotatingMobilityBase::rotate()
{
    simtime_t now = simTime();
    if (now == nextChange) {
        lastOrientation = targetOrientation;
        EV_INFO << "reached current target orientation = " << lastOrientation << endl;
        setTargetOrientation();
        EV_INFO << "new target orientation = " << targetOrientation << ", next change = " << nextChange << endl;
    }
    else if (now > lastUpdate) {
        ASSERT(nextChange == -1 || now < nextChange);
        double delta = (simTime() - lastUpdate).dbl() / (nextChange - lastUpdate).dbl();
        lastOrientation = Quaternion::slerp(lastOrientation, targetOrientation, delta);
    }
}

} // namespace inet

