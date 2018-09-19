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
        const char *positionCompositionAsString = par("positionComposition");
        if (!strcmp(positionCompositionAsString, "zero"))
            positionComposition = PositionComposition::PC_ZERO;
        else if (!strcmp(positionCompositionAsString, "sum"))
            positionComposition = PositionComposition::PC_SUM;
        else if (!strcmp(positionCompositionAsString, "average"))
            positionComposition = PositionComposition::PC_AVERAGE;
        else
            throw cRuntimeError("Unknown position Composition");
        const char *orientationCompositionAsString = par("orientationComposition");
        if (!strcmp(orientationCompositionAsString, "zero"))
            orientationComposition = OrientationComposition::OC_ZERO;
        else if (!strcmp(orientationCompositionAsString, "sum"))
            orientationComposition = OrientationComposition::OC_SUM;
        else if (!strcmp(orientationCompositionAsString, "average"))
            orientationComposition = OrientationComposition::OC_AVERAGE;
        else if (!strcmp(orientationCompositionAsString, "faceForward"))
            orientationComposition = OrientationComposition::OC_FACE_FORWARD;
        else
            throw cRuntimeError("Unknown orientation composition");
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
    switch (positionComposition) {
        case PositionComposition::PC_ZERO:
            lastPosition = Coord::ZERO;
        case PositionComposition::PC_AVERAGE:
        case PositionComposition::PC_SUM: {
            Coord position;
            for (auto element : elements)
                position += element->getCurrentPosition();
            lastPosition = position;
            if (positionComposition == PositionComposition::PC_AVERAGE)
                lastPosition /= elements.size();
            break;
        }
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
    return lastPosition;
}

Coord SuperpositioningMobility::getCurrentVelocity()
{
    switch (positionComposition) {
        case PositionComposition::PC_ZERO:
            return Coord::ZERO;
        case PositionComposition::PC_AVERAGE:
        case PositionComposition::PC_SUM: {
            Coord velocity;
            for (auto element : elements)
                velocity += element->getCurrentVelocity();
            if (positionComposition == PositionComposition::PC_AVERAGE)
                velocity /= elements.size();
            return velocity;
        }
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
}

Coord SuperpositioningMobility::getCurrentAcceleration()
{
    switch (positionComposition) {
        case PositionComposition::PC_ZERO:
            return Coord::ZERO;
        case PositionComposition::PC_AVERAGE:
        case PositionComposition::PC_SUM: {
            Coord acceleration;
            for (auto element : elements)
                acceleration += element->getCurrentAcceleration();
            if (positionComposition == PositionComposition::PC_AVERAGE)
                acceleration /= elements.size();
            return acceleration;
        }
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
}

Quaternion SuperpositioningMobility::getCurrentAngularPosition()
{
    switch (orientationComposition) {
        case OrientationComposition::OC_ZERO: {
            lastOrientation = Quaternion::IDENTITY;
            break;
        }
        case OrientationComposition::OC_SUM: {
            Quaternion angularPosition;
            for (auto element : elements)
                angularPosition *= Quaternion(element->getCurrentAngularPosition());
            lastOrientation = angularPosition;
            break;
        }
        case OrientationComposition::OC_AVERAGE: {
            Coord rotatedX;
            Coord rotatedY;
            Coord rotatedZ;
            for (auto element : elements) {
                Quaternion quaternion(element->getCurrentAngularPosition());
                rotatedX += quaternion.rotate(Coord::X_AXIS);
                rotatedY += quaternion.rotate(Coord::Y_AXIS);
                rotatedZ += quaternion.rotate(Coord::Z_AXIS);
            }
            rotatedX.normalize();
            rotatedY.normalize();
            rotatedZ.normalize();
            double matrix[3][3];
            matrix[0][0] = rotatedX.x;
            matrix[1][0] = rotatedX.y;
            matrix[2][0] = rotatedX.z;
            matrix[0][1] = rotatedY.x;
            matrix[1][1] = rotatedY.y;
            matrix[2][1] = rotatedY.z;
            matrix[0][2] = rotatedZ.x;
            matrix[1][2] = rotatedZ.y;
            matrix[2][2] = rotatedZ.z;
            if (!rotatedX.isUnspecified() && !rotatedY.isUnspecified() && !rotatedZ.isUnspecified())
                lastOrientation = RotationMatrix(matrix).toQuaternion();
            break;
        }
        case OrientationComposition::OC_FACE_FORWARD: {
            // determine orientation based on direction
            Coord lastVelocity = getCurrentVelocity();
            if (lastVelocity != Coord::ZERO) {
                Coord direction = lastVelocity;
                direction.normalize();
                auto alpha = rad(atan2(direction.y, direction.x));
                auto beta = rad(-asin(direction.z));
                auto gamma = rad(0.0);
                lastOrientation = Quaternion(EulerAngles(alpha, beta, gamma));
            }
            break;
        }
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
    return lastOrientation;
}

Quaternion SuperpositioningMobility::getCurrentAngularVelocity()
{
    switch (orientationComposition) {
        case OrientationComposition::OC_ZERO:
            return Quaternion::IDENTITY;
        case OrientationComposition::OC_SUM: {
            Quaternion angularVelocity;
            for (auto element : elements)
                angularVelocity *= Quaternion(element->getCurrentAngularVelocity());
            return angularVelocity;
        }
        case OrientationComposition::OC_AVERAGE:
            return Quaternion::NIL;
        case OrientationComposition::OC_FACE_FORWARD:
            return Quaternion::NIL;
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
}

Quaternion SuperpositioningMobility::getCurrentAngularAcceleration()
{
    switch (orientationComposition) {
        case OrientationComposition::OC_ZERO:
            return Quaternion::IDENTITY;
        case OrientationComposition::OC_SUM: {
            Quaternion angularAcceleration;
            for (auto element : elements)
                angularAcceleration *= Quaternion(element->getCurrentAngularAcceleration());
            return angularAcceleration;
        }
        case OrientationComposition::OC_AVERAGE:
            return Quaternion::NIL;
        case OrientationComposition::OC_FACE_FORWARD:
            return Quaternion::NIL;
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
}

void SuperpositioningMobility::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (IMobility::mobilityStateChangedSignal == signal)
        emitMobilityStateChangedSignal();
}

} // namespace inet

