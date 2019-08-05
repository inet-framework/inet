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

#include "inet/physicallayer/analogmodel/packetlevel/DimensionalNoise.h"

namespace inet {

namespace physicallayer {

DimensionalNoise::DimensionalNoise(simtime_t startTime, simtime_t endTime, Hz carrierFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simtime_t, Hz>>>& power) :
    NarrowbandNoiseBase(startTime, endTime, carrierFrequency, bandwidth),
    power(power)
{
}

std::ostream& DimensionalNoise::printToStream(std::ostream& stream, int level) const
{
    stream << "DimensionalNoise";
    if (level <= PRINT_LEVEL_DEBUG)
        stream << ", powerMax = " << power->getMax()
               << ", powerMin = " << power->getMin();
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", power = " << power;
    return NarrowbandNoiseBase::printToStream(stream, level);
}

W DimensionalNoise::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    Point<simtime_t> startPoint(startTime);
    Point<simtime_t> endPoint(endTime);
    W minPower = integrate<WpHz, Domain<simtime_t, Hz>, 0b10, W, Domain<simtime_t>>(power)->getMin(Interval<simtime_t>(startPoint, endPoint, 0b1));
    EV_DEBUG << "Computing minimum noise power: start = " << startPoint << ", end = " << endPoint << " -> " << minPower << endl;
    return minPower;
}

W DimensionalNoise::computeMaxPower(simtime_t startTime, simtime_t endTime) const
{
    Point<simtime_t> startPoint(startTime);
    Point<simtime_t> endPoint(endTime);
    W maxPower = integrate<WpHz, Domain<simtime_t, Hz>, 0b10, W, Domain<simtime_t>>(power)->getMax(Interval<simtime_t>(startPoint, endPoint, 0b1));
    EV_DEBUG << "Computing maximum noise power: start = " << startPoint << ", end = " << endPoint << " -> " << maxPower << endl;
    return maxPower;
}

} // namespace physicallayer

} // namespace inet

