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

DimensionalNoise::DimensionalNoise(simtime_t startTime, simtime_t endTime, Hz carrierFrequency, Hz bandwidth, const ConstMapping *power) :
    NarrowbandNoiseBase(startTime, endTime, carrierFrequency, bandwidth),
    power(power)
{
}

std::ostream& DimensionalNoise::printToStream(std::ostream& stream, int level) const
{
    stream << "DimensionalNoise";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", powerDimensionSet = " << power->getDimensionSet();
    if (level <= PRINT_LEVEL_DEBUG)
        stream << ", powerMax = " << MappingUtils::findMax(*power)
               << ", powerMin = " << MappingUtils::findMin(*power);
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", power = " << power;
    return NarrowbandNoiseBase::printToStream(stream, level);
}

W DimensionalNoise::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    const DimensionSet& dimensions = power->getDimensionSet();
    Argument start(dimensions);
    Argument end(dimensions);
    if (dimensions.hasDimension(Dimension::time)) {
        start.setTime(startTime);
        end.setTime(endTime);
    }
    if (dimensions.hasDimension(Dimension::frequency)) {
        start.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
        end.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
    }
    W minPower = W(MappingUtils::findMin(*power, start, end));
    EV_DEBUG << "Computing minimum noise power: start = " << start << ", end = " << end << " -> " << minPower << endl;
    return minPower;
}

W DimensionalNoise::computeMaxPower(simtime_t startTime, simtime_t endTime) const
{
    const DimensionSet& dimensions = power->getDimensionSet();
    Argument start(dimensions);
    Argument end(dimensions);
    if (dimensions.hasDimension(Dimension::time)) {
        start.setTime(startTime);
        end.setTime(endTime);
    }
    if (dimensions.hasDimension(Dimension::frequency)) {
        start.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
        end.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
    }
    W maxPower = W(MappingUtils::findMax(*power, start, end));
    EV_DEBUG << "Computing maximum noise power: start = " << start << ", end = " << end << " -> " << maxPower << endl;
    return maxPower;
}

} // namespace physicallayer

} // namespace inet

