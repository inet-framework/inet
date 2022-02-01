//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/single/SuperpositioningMobility.h"

#include "inet/common/geometry/common/Quaternion.h"

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
        else {
            positionComposition = PositionComposition::PC_ELEMENT;
            positionElementIndex = std::stoi(positionCompositionAsString);
        }
        const char *orientationCompositionAsString = par("orientationComposition");
        if (!strcmp(orientationCompositionAsString, "zero"))
            orientationComposition = OrientationComposition::OC_ZERO;
        else if (!strcmp(orientationCompositionAsString, "sum"))
            orientationComposition = OrientationComposition::OC_SUM;
        else if (!strcmp(orientationCompositionAsString, "average"))
            orientationComposition = OrientationComposition::OC_AVERAGE;
        else if (!strcmp(orientationCompositionAsString, "faceForward"))
            orientationComposition = OrientationComposition::OC_FACE_FORWARD;
        else {
            orientationComposition = OrientationComposition::OC_ELEMENT;
            orientationElementIndex = std::stoi(orientationCompositionAsString);
        }
        int numElements = par("numElements");
        for (int i = 0; i < numElements; i++) {
            auto element = getSubmodule("element", i);
            element->subscribe(IMobility::mobilityStateChangedSignal, this);
            elements.push_back(check_and_cast<IMobility *>(element));
        }
        WATCH(lastVelocity);
        WATCH(lastAcceleration);
    }
    else if (stage == INITSTAGE_LAST)
        initializePosition();
}

void SuperpositioningMobility::setInitialPosition()
{
    lastPosition = getCurrentPosition();
}

const Coord& SuperpositioningMobility::getCurrentPosition()
{
    switch (positionComposition) {
        case PositionComposition::PC_ZERO:
            lastPosition = Coord::ZERO;
            break;
        case PositionComposition::PC_AVERAGE:
        case PositionComposition::PC_SUM:
            lastPosition = Coord::ZERO;
            for (auto element : elements)
                lastPosition += element->getCurrentPosition();
            if (positionComposition == PositionComposition::PC_AVERAGE)
                lastPosition /= elements.size();
            break;
        case PositionComposition::PC_ELEMENT:
            lastPosition = elements[positionElementIndex]->getCurrentPosition();
            break;
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
    return lastPosition;
}

const Coord& SuperpositioningMobility::getCurrentVelocity()
{
    switch (positionComposition) {
        case PositionComposition::PC_ZERO:
            lastVelocity = Coord::ZERO;
            break;
        case PositionComposition::PC_AVERAGE:
        case PositionComposition::PC_SUM:
            lastVelocity = Coord::ZERO;
            for (auto element : elements)
                lastVelocity += element->getCurrentVelocity();
            if (positionComposition == PositionComposition::PC_AVERAGE)
                lastVelocity /= elements.size();
            break;
        case PositionComposition::PC_ELEMENT:
            lastVelocity = elements[positionElementIndex]->getCurrentVelocity();
            break;
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
    return lastVelocity;
}

const Coord& SuperpositioningMobility::getCurrentAcceleration()
{
    switch (positionComposition) {
        case PositionComposition::PC_ZERO:
            lastAcceleration = Coord::ZERO;
            break;
        case PositionComposition::PC_AVERAGE:
        case PositionComposition::PC_SUM:
            lastAcceleration = Coord::ZERO;
            for (auto element : elements)
                lastAcceleration += element->getCurrentAcceleration();
            if (positionComposition == PositionComposition::PC_AVERAGE)
                lastAcceleration /= elements.size();
            break;
        case PositionComposition::PC_ELEMENT:
            lastAcceleration = elements[positionElementIndex]->getCurrentAcceleration();
            break;
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
    return lastAcceleration;
}

const Quaternion& SuperpositioningMobility::getCurrentAngularPosition()
{
    switch (orientationComposition) {
        case OrientationComposition::OC_ZERO:
            lastOrientation = Quaternion::IDENTITY;
            break;
        case OrientationComposition::OC_SUM:
            lastOrientation = Quaternion::IDENTITY;
            for (auto element : elements)
                lastOrientation *= Quaternion(element->getCurrentAngularPosition());
            break;
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
        case OrientationComposition::OC_ELEMENT: {
            lastOrientation = elements[orientationElementIndex]->getCurrentAngularPosition();
            break;
        }
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
    return lastOrientation;
}

const Quaternion& SuperpositioningMobility::getCurrentAngularVelocity()
{
    switch (orientationComposition) {
        case OrientationComposition::OC_ZERO:
            lastAngularVelocity = Quaternion::IDENTITY;
            break;
        case OrientationComposition::OC_SUM:
            lastAngularVelocity = Quaternion::IDENTITY;
            for (auto element : elements)
                lastAngularVelocity *= Quaternion(element->getCurrentAngularVelocity());
            break;
        case OrientationComposition::OC_AVERAGE:
            lastAngularVelocity = Quaternion::NIL;
            break;
        case OrientationComposition::OC_FACE_FORWARD:
            lastAngularVelocity = Quaternion::NIL;
            break;
        case OrientationComposition::OC_ELEMENT:
            lastAngularVelocity = Quaternion::NIL;
            break;
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
    return lastAngularVelocity;
}

const Quaternion& SuperpositioningMobility::getCurrentAngularAcceleration()
{
    switch (orientationComposition) {
        case OrientationComposition::OC_ZERO:
            lastAngularAcceleration = Quaternion::IDENTITY;
            break;
        case OrientationComposition::OC_SUM:
            lastAngularAcceleration = Quaternion::IDENTITY;
            for (auto element : elements)
                lastAngularAcceleration *= Quaternion(element->getCurrentAngularAcceleration());
            break;
        case OrientationComposition::OC_AVERAGE:
            lastAngularAcceleration = Quaternion::NIL;
            break;
        case OrientationComposition::OC_FACE_FORWARD:
            lastAngularAcceleration = Quaternion::NIL;
            break;
        case OrientationComposition::OC_ELEMENT:
            lastAngularAcceleration = Quaternion::NIL;
            break;
        default:
            throw cRuntimeError("Unknown orientation composition");
    }
    return lastAngularAcceleration;
}

void SuperpositioningMobility::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (IMobility::mobilityStateChangedSignal == signal)
        emitMobilityStateChangedSignal();
}

} // namespace inet

