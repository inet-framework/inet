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

#include "inet/mobility/single/FacingMobility.h"

namespace inet {

Define_Module(FacingMobility);

void FacingMobility::initialize(int stage)
{
    MobilityBase::initialize(stage);
    EV_TRACE << "initializing FacingMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        sourceMobility = getModuleFromPar<IMobility>(par("sourceMobility"), this);
        targetMobility = getModuleFromPar<IMobility>(par("targetMobility"), this);
    }
}

Quaternion FacingMobility::getCurrentAngularPosition()
{
    Coord direction = targetMobility->getCurrentPosition() - sourceMobility->getCurrentPosition();
    auto alpha = rad(atan2(direction.y, direction.x));
    auto beta = rad(-asin(direction.z));
    auto gamma = rad(0.0);
    lastOrientation = Quaternion(EulerAngles(alpha, beta, gamma));
    return lastOrientation;
}

} // namespace inet

