//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/ScalarSignalAnalogModel.h"

namespace inet {

namespace physicallayer {

ScalarSignalAnalogModel::ScalarSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) :
    NarrowbandSignalAnalogModel(duration, centerFrequency, bandwidth),
    power(power)
{
}

std::ostream& ScalarSignalAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ScalarSignalAnalogModel";
    if (level <= PRINT_LEVEL_DETAIL)
       stream << EV_FIELD(power);
    return NarrowbandSignalAnalogModel::printToStream(stream, level);
}

ScalarTransmissionSignalAnalogModel::ScalarTransmissionSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) :
    ScalarSignalAnalogModel(duration, centerFrequency, bandwidth, power)
{
}

ScalarReceptionSignalAnalogModel::ScalarReceptionSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) :
    ScalarSignalAnalogModel(duration, centerFrequency, bandwidth, power)
{
}

} // namespace physicallayer

} // namespace inet

