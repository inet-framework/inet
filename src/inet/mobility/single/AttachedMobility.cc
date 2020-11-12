//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/mobility/single/AttachedMobility.h"

#include "inet/common/geometry/common/Quaternion.h"

namespace inet {

Define_Module(AttachedMobility);

void AttachedMobility::initialize(int stage)
{
    MobilityBase::initialize(stage);
    EV_TRACE << "initializing AttachedMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        mobility = getModuleFromPar<IMobility>(par("mobilityModule"), this);
        positionOffset.x = par("offsetX");
        positionOffset.y = par("offsetY");
        positionOffset.z = par("offsetZ");
        auto alpha = deg(par("offsetHeading"));
        auto offsetElevation = deg(par("offsetElevation"));
        // NOTE: negation is needed, see IMobility comments on orientation
        auto beta = -offsetElevation;
        auto gamma = deg(par("offsetBank"));
        orientationOffset = Quaternion(EulerAngles(alpha, beta, gamma));
        isZeroOffset = positionOffset == Coord::ZERO;
        check_and_cast<cModule *>(mobility)->subscribe(IMobility::mobilityStateChangedSignal, this);
        WATCH(lastVelocity);
        WATCH(lastAngularPosition);
    }
}

void AttachedMobility::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (IMobility::mobilityStateChangedSignal == signal)
        emitMobilityStateChangedSignal();
}

const Coord& AttachedMobility::getCurrentPosition()
{
    if (isZeroOffset)
        lastPosition = mobility->getCurrentPosition();
    else {
        RotationMatrix rotation(mobility->getCurrentAngularPosition().toEulerAngles());
        lastPosition = mobility->getCurrentPosition() + rotation.rotateVector(positionOffset);
    }
    return lastPosition;
}

const Coord& AttachedMobility::getCurrentVelocity()
{
    if (isZeroOffset)
        lastVelocity = mobility->getCurrentVelocity();
    else {
        RotationMatrix rotation(mobility->getCurrentAngularPosition().toEulerAngles());
        Coord rotatedOffset = rotation.rotateVector(positionOffset);
        Quaternion quaternion(mobility->getCurrentAngularVelocity());
        Coord rotationAxis;
        double rotationAngle;
        quaternion.getRotationAxisAndAngle(rotationAxis, rotationAngle);
        auto additionalVelocity = rotationAngle == 0 ? Coord::ZERO : rotationAxis % rotatedOffset * rotationAngle;
        lastVelocity = mobility->getCurrentVelocity() + additionalVelocity;
    }
    return lastVelocity;
}

const Coord& AttachedMobility::getCurrentAcceleration()
{
    if (isZeroOffset)
        return mobility->getCurrentAcceleration();
    else {
        // TODO:
        return Coord::NIL;
    }
}

const Quaternion& AttachedMobility::getCurrentAngularPosition()
{
    lastAngularPosition = mobility->getCurrentAngularPosition();
    lastAngularPosition *= Quaternion(orientationOffset);
    return lastAngularPosition;
}

const Quaternion& AttachedMobility::getCurrentAngularVelocity()
{
    return mobility->getCurrentAngularVelocity();
}

const Quaternion& AttachedMobility::getCurrentAngularAcceleration()
{
    return mobility->getCurrentAngularAcceleration();
}

} // namespace inet

