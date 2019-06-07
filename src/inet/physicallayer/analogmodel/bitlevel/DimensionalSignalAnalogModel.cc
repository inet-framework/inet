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

DimensionalSignalAnalogModel::DimensionalSignalAnalogModel(const simtime_t duration, Hz carrierFrequency, Hz bandwidth, const Ptr<const math::IFunction<W, simtime_t, Hz>>& power) :
    NarrowbandSignalAnalogModel(duration, carrierFrequency, bandwidth),
    power(power)
{
}

std::ostream& DimensionalSignalAnalogModel::printToStream(std::ostream& stream, int level) const
{
    stream << "DimensionalSignalAnalogModel";
    if (level <= PRINT_LEVEL_DEBUG)
        stream << ", powerMax = " << power->getMax()
               << ", powerMin = " << power->getMin();
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", power = " << power;
    return NarrowbandSignalAnalogModel::printToStream(stream, level);
}

W DimensionalSignalAnalogModel::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    math::Point<simtime_t, Hz> startPoint(startTime, carrierFrequency - bandwidth / 2);
    math::Point<simtime_t, Hz> endPoint(endTime, carrierFrequency + bandwidth / 2);
    W minPower = power->getMin(math::Interval<simtime_t, Hz>(startPoint, endPoint));
    EV_DEBUG << "Computing minimum reception power: start = " << startPoint << ", end = " << endPoint << " -> minimum reception power = " << minPower << endl;
    return minPower;
}

DimensionalTransmissionSignalAnalogModel::DimensionalTransmissionSignalAnalogModel(const simtime_t duration, Hz carrierFrequency, Hz bandwidth, const Ptr<const math::IFunction<W, simtime_t, Hz>>& power) :
    DimensionalSignalAnalogModel(duration, carrierFrequency, bandwidth, power)
{
}

DimensionalReceptionSignalAnalogModel::DimensionalReceptionSignalAnalogModel(const simtime_t duration, Hz carrierFrequency, Hz bandwidth, const Ptr<const math::IFunction<W, simtime_t, Hz>>& power) :
    DimensionalSignalAnalogModel(duration, carrierFrequency, bandwidth, power)
{
}

} // namespace physicallayer

} // namespace inet

