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

#include "inet/physicallayer/base/packetlevel/NarrowbandNoiseBase.h"

namespace inet {

namespace physicallayer {

NarrowbandNoiseBase::NarrowbandNoiseBase(simtime_t startTime, simtime_t endTime, Hz centerFrequency, Hz bandwidth) :
    NoiseBase(startTime, endTime),
    centerFrequency(centerFrequency),
    bandwidth(bandwidth)
{
}

std::ostream& NarrowbandNoiseBase::printToStream(std::ostream& stream, int level) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", centerFrequency = " << centerFrequency
               << ", bandwidth = " << bandwidth;
    return NoiseBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

