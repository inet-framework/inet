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

#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarNoise.h"

namespace inet {

namespace physicallayer {

ScalarNoise::ScalarNoise(simtime_t startTime, simtime_t endTime, Hz centerFrequency, Hz bandwidth, const std::map<simtime_t, W> *powerChanges) :
    NarrowbandNoiseBase(startTime, endTime, centerFrequency, bandwidth),
    powerChanges(powerChanges)
{
}

std::ostream& ScalarNoise::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ScalarNoise";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(powerChanges);
    return NarrowbandNoiseBase::printToStream(stream, level);
}

W ScalarNoise::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    W noisePower = W(0);
    W minNoisePower = W(NaN);
    for (const auto& elem : *powerChanges) {
        if(endTime <= elem.first){
            // Power change after or at end time
            if (std::isnan(minNoisePower.get()) || noisePower < minNoisePower)
                // Previous power change was before start time, set to current power if not first change
                minNoisePower = noisePower;
            break;
        }
        noisePower += elem.second;
        if ((std::isnan(minNoisePower.get()) || noisePower < minNoisePower) && startTime <= elem.first)
            minNoisePower = noisePower;
    }
    return minNoisePower;
}

W ScalarNoise::computeMaxPower(simtime_t startTime, simtime_t endTime) const
{
    W noisePower = W(0);
    W maxNoisePower = W(0);
    for (const auto& elem : *powerChanges) {
        if(endTime <= elem.first){
            // Power change after or at end time
            if (std::isnan(maxNoisePower.get()) || noisePower > maxNoisePower)
                // Previous power change was before start time, set to current power if not first change
                maxNoisePower = noisePower;
            break;
        }

        noisePower += elem.second;
        if ((std::isnan(maxNoisePower.get()) || noisePower > maxNoisePower) && startTime <= elem.first)
            maxNoisePower = noisePower;
    }
    return maxNoisePower;
}

} // namespace physicallayer

} // namespace inet

