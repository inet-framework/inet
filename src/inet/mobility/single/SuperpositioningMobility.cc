//
// Copyright (C) OpenSim Ltd.
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

#include "inet/common/geometry/common/Quaternion.h"
#include "inet/mobility/single/SuperpositioningMobility.h"

namespace inet {

Define_Module(SuperpositioningMobility);

void SuperpositioningMobility::initialize(int stage)
{
    MobilityBase::initialize(stage);
    EV_TRACE << "initializing SuperpositioningMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        int numElements = par("numElements");
        for (int i = 0; i < numElements; i++) {
            auto element = getSubmodule("element", i);
            element->subscribe(IMobility::mobilityStateChangedSignal, this);
            elements.push_back(check_and_cast<IMobility *>(element));
        }
    }
    else if (stage == INITSTAGE_LAST)
        initializePosition();
}

void SuperpositioningMobility::setInitialPosition()
{
    lastPosition = getCurrentPosition();
}

Coord SuperpositioningMobility::getCurrentPosition()
{
    Coord position;
    for (auto element : elements)
        position += element->getCurrentPosition();
    lastPosition = position;
    return position;
}

Coord SuperpositioningMobility::getCurrentVelocity()
{
    Coord velocity;
    for (auto element : elements)
        velocity += element->getCurrentVelocity();
    return velocity;
}

Coord SuperpositioningMobility::getCurrentAcceleration()
{
    Coord acceleration;
    for (auto element : elements)
        acceleration += element->getCurrentAcceleration();
    return acceleration;
}

EulerAngles SuperpositioningMobility::getCurrentAngularPosition()
{
    Quaternion angularPosition;
    for (auto element : elements)
        angularPosition *= Quaternion(element->getCurrentAngularPosition());
    lastOrientation = angularPosition.toEulerAngles();
    return lastOrientation;
}

EulerAngles SuperpositioningMobility::getCurrentAngularVelocity()
{
    Quaternion angularVelocity;
    for (auto element : elements)
        angularVelocity *= Quaternion(element->getCurrentAngularVelocity());
    return angularVelocity.toEulerAngles();
}

EulerAngles SuperpositioningMobility::getCurrentAngularAcceleration()
{
    Quaternion angularAcceleration;
    for (auto element : elements)
        angularAcceleration *= Quaternion(element->getCurrentAngularAcceleration());
    return angularAcceleration.toEulerAngles();
}

void SuperpositioningMobility::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (IMobility::mobilityStateChangedSignal == signal)
        emitMobilityStateChangedSignal();
}

} // namespace inet

