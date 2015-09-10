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

#include "inet/physicallayer/analogmodel/bitlevel/DimensionalSignalAnalogModel.h"

namespace inet {

namespace physicallayer {

DimensionalSignalAnalogModel::DimensionalSignalAnalogModel(const simtime_t duration, Hz carrierFrequency, Hz bandwidth, const ConstMapping *power) :
    NarrowbandSignalAnalogModel(duration, carrierFrequency, bandwidth),
    power(power)
{
}

std::ostream& DimensionalSignalAnalogModel::printToStream(std::ostream& stream, int level) const
{
    stream << "DimensionalSignalAnalogModel";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", powerDimensionSet = " << power->getDimensionSet();
    if (level >= PRINT_LEVEL_DEBUG)
        stream << ", powerMax = " << MappingUtils::findMax(*power)
               << ", powerMin = " << MappingUtils::findMin(*power);
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", power = " << power;
    return NarrowbandSignalAnalogModel::printToStream(stream, level);
}

W DimensionalSignalAnalogModel::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    const DimensionSet& dimensions = power->getDimensionSet();
    Argument startArgument(dimensions);
    Argument endArgument(dimensions);
    if (dimensions.hasDimension(Dimension::time)) {
        startArgument.setTime(startTime);
        // NOTE: to exclude the moment where the reception power starts to be 0 again
        endArgument.setTime(MappingUtils::pre(endTime));
    }
    if (dimensions.hasDimension(Dimension::frequency)) {
        startArgument.setArgValue(Dimension::frequency, (carrierFrequency - bandwidth / 2).get());
        endArgument.setArgValue(Dimension::frequency, nexttoward((carrierFrequency + bandwidth / 2).get(), 0));
    }
    W minPower = W(MappingUtils::findMin(*power, startArgument, endArgument));
    EV_DEBUG << "Computing minimum reception power: start = " << startArgument << ", end = " << endArgument << " -> minimum reception power = " << minPower << endl;
    return minPower;
}

DimensionalTransmissionSignalAnalogModel::DimensionalTransmissionSignalAnalogModel(const simtime_t duration, Hz carrierFrequency, Hz bandwidth, const ConstMapping *power) :
    DimensionalSignalAnalogModel(duration, carrierFrequency, bandwidth, power)
{
}

DimensionalReceptionSignalAnalogModel::DimensionalReceptionSignalAnalogModel(const simtime_t duration, Hz carrierFrequency, Hz bandwidth, const ConstMapping *power) :
    DimensionalSignalAnalogModel(duration, carrierFrequency, bandwidth, power)
{
}

} // namespace physicallayer

} // namespace inet

