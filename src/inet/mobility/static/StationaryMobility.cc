//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/static/StationaryMobility.h"

namespace inet {

Define_Module(StationaryMobility);

void StationaryMobility::initialize(int stage)
{
    StationaryMobilityBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        updateFromDisplayString = par("updateFromDisplayString");
}

void StationaryMobility::refreshDisplay() const
{
    if (updateFromDisplayString) {
        const_cast<StationaryMobility *>(this)->updateMobilityStateFromDisplayString();
        DirectiveResolver directiveResolver(const_cast<StationaryMobility *>(this));
        auto text = format.formatString(&directiveResolver);
        getDisplayString().setTagArg("t", 0, text.c_str());
    }
    else
        StationaryMobilityBase::refreshDisplay();
}

void StationaryMobility::updateMobilityStateFromDisplayString()
{
    char *end;
    double depth;
    cDisplayString& displayString = subjectModule->getDisplayString();
    canvasProjection->computeCanvasPoint(lastPosition, depth);
    double x = strtod(displayString.getTagArg("p", 0), &end);
    double y = strtod(displayString.getTagArg("p", 1), &end);
    auto newPosition = canvasProjection->computeCanvasPointInverse(cFigure::Point(x, y), depth);
    if (lastPosition != newPosition) {
        lastPosition = newPosition;
        emit(mobilityStateChangedSignal, const_cast<StationaryMobility *>(this));
    }
    Quaternion swing;
    Quaternion twist;
    Coord vector = canvasProjection->computeCanvasPointInverse(cFigure::Point(0, 0), 1);
    vector.normalize();
    lastOrientation.getSwingAndTwist(vector, swing, twist);
    double oldAngle;
    Coord axis;
    twist.getRotationAxisAndAngle(axis, oldAngle);
    double newAngle = math::deg2rad(strtod(displayString.getTagArg("a", 0), &end));
    if (oldAngle != newAngle) {
        lastOrientation *= Quaternion(vector, newAngle - oldAngle);
        emit(mobilityStateChangedSignal, const_cast<StationaryMobility *>(this));
    }
}

} // namespace inet

