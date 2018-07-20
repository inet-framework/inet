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

#include "inet/physicallayer/analogmodel/packetlevel/ScalarNoise.h"

namespace inet {

namespace physicallayer {

ScalarNoise::ScalarNoise(simtime_t startTime, simtime_t endTime, Hz carrierFrequency, Hz bandwidth, const std::map<simtime_t, W> *powerChanges) :
    NarrowbandNoiseBase(startTime, endTime, carrierFrequency, bandwidth),
    powerChanges(powerChanges)
{
}

std::ostream& ScalarNoise::printToStream(std::ostream& stream, int level) const
{
    stream << "ScalarNoise";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", powerChanges = " << powerChanges ;
    return NarrowbandNoiseBase::printToStream(stream, level);
}

W ScalarNoise::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    W noisePower = W(0);
    W minNoisePower = W(NaN);
    for (const auto & elem : *powerChanges) {
        noisePower += elem.second;
        if ((std::isnan(minNoisePower.get()) || noisePower < minNoisePower) && startTime <= elem.first && elem.first < endTime)
            minNoisePower = noisePower;
    }
    return minNoisePower;
}

W ScalarNoise::computeMaxPower(simtime_t startTime, simtime_t endTime) const
{
    W noisePower = W(0);
    W maxNoisePower = W(0);
    for (const auto & elem : *powerChanges) {
        noisePower += elem.second;
        if ((std::isnan(maxNoisePower.get()) || noisePower > maxNoisePower) && startTime <= elem.first && elem.first < endTime)
            maxNoisePower = noisePower;
    }
    return maxNoisePower;
}

} // namespace physicallayer

} // namespace inet

