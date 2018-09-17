//
// Copyright (C) 2013 OpenSim Ltd.
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
#include "inet/physicallayer/base/packetlevel/AnalogModelBase.h"

namespace inet {

namespace physicallayer {

double AnalogModelBase::computeAntennaGain(const IAntennaGain* antennaGain, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation) const
{
    if (antennaGain->getMinGain() == antennaGain->getMaxGain())
        return antennaGain->getMinGain();
    else {
        auto direction = Quaternion::rotationFromTo(Coord::X_AXIS, endPosition - startPosition);
        auto antennaLocalDirection = Quaternion(startOrientation).inverse() * direction;
        return antennaGain->computeGain(antennaLocalDirection);
    }
}

} // namespace physicallayer

} // namespace inet

