//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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
        getDisplayString().setTagArg("t", 0, text);
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

