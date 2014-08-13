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

#include "DimensionalUtils.h"

namespace inet {

namespace physicallayer {

ConstMapping *DimensionalUtils::createFlatMapping(const DimensionSet& dimensions, const simtime_t startTime, const simtime_t endTime, Hz carrierFrequency, Hz bandwidth, W power)
{
    Mapping *powerMapping = MappingUtils::createMapping(Argument::MappedZero, dimensions, Mapping::LINEAR);
    Argument position(dimensions);
    position.setTime(startTime);
    if (dimensions.hasDimension(Dimension::frequency))
        position.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
    powerMapping->setValue(position, power.get());
    position.setTime(endTime);
    powerMapping->setValue(position, power.get());
    if (dimensions.hasDimension(Dimension::frequency))
    {
        position.setTime(startTime);
        position.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
        powerMapping->setValue(position, power.get());
        position.setTime(endTime);
        powerMapping->setValue(position, power.get());
    }
    return powerMapping;
}

} // namespace physicallayer

} // namespace inet

