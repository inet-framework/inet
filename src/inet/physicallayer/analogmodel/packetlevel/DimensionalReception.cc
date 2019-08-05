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

#include "inet/physicallayer/analogmodel/packetlevel/DimensionalReception.h"

namespace inet {

namespace physicallayer {

DimensionalReception::DimensionalReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, Hz carrierFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simtime_t, Hz>>>& power) :
    FlatReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, carrierFrequency, bandwidth),
    power(power)
{
}

W DimensionalReception::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    Point<simtime_t> startPoint(startTime);
    Point<simtime_t> endPoint(endTime);
    W minPower = integrate<WpHz, Domain<simtime_t, Hz>, 0b10, W, Domain<simtime_t>>(power)->getMin(Interval<simtime_t>(startPoint, endPoint, 0b1));
    EV_DEBUG << "Computing minimum reception power: start = " << startPoint << ", end = " << endPoint << " -> minimum reception power = " << minPower << endl;
    return minPower;
}

} // namespace physicallayer

} // namespace inet

