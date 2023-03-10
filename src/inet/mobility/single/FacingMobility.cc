//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/single/FacingMobility.h"

namespace inet {

Define_Module(FacingMobility);

void FacingMobility::initialize(int stage)
{
    MobilityBase::initialize(stage);
    EV_TRACE << "initializing FacingMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        format.parseFormat(par("displayStringTextFormat"));
        sourceMobility.reference(this, "sourceMobility", true);
        targetMobility.reference(this, "targetMobility", true);
    }
}

const Quaternion& FacingMobility::getCurrentAngularPosition()
{
    Coord direction = targetMobility->getCurrentPosition() - sourceMobility->getCurrentPosition();
    direction.normalize();
    auto alpha = rad(atan2(direction.y, direction.x));
    auto beta = rad(-asin(direction.z));
    auto gamma = rad(0.0);
    ASSERT(!std::isnan(alpha.get()) && !std::isnan(beta.get()));
    lastOrientation = Quaternion(EulerAngles(alpha, beta, gamma));
    return lastOrientation;
}

void FacingMobility::handleParameterChange(const char *name)
{
    if (!strcmp(name, "displayStringTextFormat"))
        format.parseFormat(par("displayStringTextFormat"));
    else if (!strcmp(name, "targetMobility")) {
        targetMobility.reference(this, "targetMobility", true);
        emitMobilityStateChangedSignal();
    }
}

} // namespace inet

