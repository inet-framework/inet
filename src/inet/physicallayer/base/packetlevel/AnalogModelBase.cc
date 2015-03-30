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

#include "inet/physicallayer/base/packetlevel/AnalogModelBase.h"

namespace inet {

namespace physicallayer {

EulerAngles AnalogModelBase::computeTransmissionDirection(const ITransmission *transmission, const IArrival *arrival) const
{
    const Coord transmissionStartPosition = transmission->getStartPosition();
    const Coord arrivalStartPosition = arrival->getStartPosition();
    Coord transmissionStartDirection = arrivalStartPosition - transmissionStartPosition;
    double z = transmissionStartDirection.z;
    transmissionStartDirection.z = 0;
    double heading = atan2(transmissionStartDirection.y, transmissionStartDirection.x);
    double elevation = atan2(z, transmissionStartDirection.length());
    return EulerAngles(heading, elevation, 0);
}

} // namespace physicallayer

} // namespace inet

