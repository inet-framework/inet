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

DimensionalReception::DimensionalReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, Hz carrierFrequency, Hz bandwidth, const ConstMapping *power) :
    FlatReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, carrierFrequency, bandwidth),
    power(power)
{
}

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
        startArgument.setArgValue(Dimension::frequency, (carrierFrequency - bandwidth / 2).get());
        endArgument.setArgValue(Dimension::frequency, nexttoward((carrierFrequency + bandwidth / 2).get(), 0));
    }
    W minPower = W(MappingUtils::findMin(*power, startArgument, endArgument));
    EV_DEBUG << "Computing minimum reception power: start = " << startArgument << ", end = " << endArgument << " -> minimum reception power = " << minPower << endl;
    return minPower;
}

} // namespace physicallayer

} // namespace inet

