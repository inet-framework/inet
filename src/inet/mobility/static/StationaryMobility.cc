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
    else if (stage == INITSTAGE_SINGLE_MOBILITY && updateFromDisplayString)
        rememberDisplayStringPosition();
}

void StationaryMobility::refreshDisplay() const
{
    if (updateFromDisplayString) {
        const_cast<StationaryMobility *>(this)->updateMobilityStateFromDisplayString();
    }
    StationaryMobilityBase::refreshDisplay();
    if (updateFromDisplayString)
        rememberDisplayStringPosition();
}

void StationaryMobility::rememberDisplayStringPosition() const
{
    if (subjectModule != nullptr) {
        cDisplayString& displayString = subjectModule->getDisplayString();
        displayStringPositionX = displayString.getTagArg("p", 0);
        displayStringPositionY = displayString.getTagArg("p", 1);
    }
}

void StationaryMobility::updateMobilityStateFromDisplayString()
{
    char *end;
    double depth;
    cDisplayString& displayString = subjectModule->getDisplayString();
    const char *xText = displayString.getTagArg("p", 0);
    const char *yText = displayString.getTagArg("p", 1);
    // an unchanged p tag is not a drag, and it may have been written through an earlier canvas
    // projection, e.g. before GeoMapCanvasVisualizer installed its map projection in INITSTAGE_LAST
    if (displayStringPositionX != xText || displayStringPositionY != yText) {
        canvasProjection->computeCanvasPoint(lastPosition, depth);
        double x = strtod(xText, &end);
        double y = strtod(yText, &end);
        auto newPosition = canvasProjection->computeCanvasPointInverse(cFigure::Point(x, y), depth);
        if (lastPosition != newPosition) {
            lastPosition = newPosition;
            emit(mobilityStateChangedSignal, const_cast<StationaryMobility *>(this));
        }
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

