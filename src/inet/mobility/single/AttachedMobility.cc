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

#include "AttachedMobility.h"

namespace inet {

Define_Module(AttachedMobility);

void AttachedMobility::initialize(int stage)
{
    MobilityBase::initialize(stage);
    EV_TRACE << "initializing AttachedMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        mobility = getModuleFromPar<IMobility>(par("mobilityModule"), this, true);
        offset.x = par("offsetX");
        offset.y = par("offsetY");
        offset.z = par("offsetZ");
        isZeroOffset = offset == Coord::ZERO;
        check_and_cast<cModule *>(mobility)->subscribe(IMobility::mobilityStateChangedSignal, this);
    }
}

void AttachedMobility::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (IMobility::mobilityStateChangedSignal == signal) {
        emitMobilityStateChangedSignal();
        updateVisualRepresentation();
    }
}

Coord AttachedMobility::getCurrentPosition()
{
    if (isZeroOffset)
        return mobility->getCurrentPosition();
    else {
        EulerAngles a = mobility->getCurrentAngularPosition();
        std::swap(a.alpha, a.gamma);
        Rotation rotation(a);
        return mobility->getCurrentPosition() + rotation.rotateVectorClockwise(offset);
    }
}

Coord AttachedMobility::getCurrentSpeed()
{
    if (isZeroOffset)
        return mobility->getCurrentSpeed();
    else {
        // TODO:
        return Coord::NIL;
    }
}

Coord AttachedMobility::getCurrentAcceleration()
{
    if (isZeroOffset)
        return mobility->getCurrentAcceleration();
    else {
        // TODO:
        return Coord::NIL;
    }
}

EulerAngles AttachedMobility::getCurrentAngularPosition()
{
    return mobility->getCurrentAngularPosition();
}

EulerAngles AttachedMobility::getCurrentAngularSpeed()
{
    return mobility->getCurrentAngularSpeed();
}

EulerAngles AttachedMobility::getCurrentAngularAcceleration()
{
    return mobility->getCurrentAngularAcceleration();
}

} // namespace inet

