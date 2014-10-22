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

#include "inet/physicallayer/dimensional/DimensionalReception.h"

namespace inet {

namespace physicallayer {

W DimensionalReception::computeMinPower(simtime_t startTime, simtime_t endTime) const
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
        startArgument.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
        endArgument.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
    }
    W minPower = W(MappingUtils::findMin(*power, startArgument, endArgument));
    EV_DEBUG << "Computing minimum reception power: start = " << startArgument << ", end = " << endArgument << " -> minimum reception power = " << minPower << endl;
    return minPower;
}

} // namespace physicallayer

} // namespace inet

