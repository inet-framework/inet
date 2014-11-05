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

#include "inet/physicallayer/analogmodel/ScalarNoise.h"

namespace inet {

namespace physicallayer {

ScalarNoise::ScalarNoise(simtime_t startTime, simtime_t endTime, Hz carrierFrequency, Hz bandwidth, const std::map<simtime_t, W> *powerChanges) :
    NarrowbandNoiseBase(startTime, endTime, carrierFrequency, bandwidth),
    powerChanges(powerChanges)
{
}

void ScalarNoise::printToStream(std::ostream& stream) const
{
    stream << "ScalarNoise, "
           << "powerChanges = { " << powerChanges << " }, ";
    NarrowbandNoiseBase::printToStream(stream);
}

W ScalarNoise::computeMaxPower(simtime_t startTime, simtime_t endTime) const
{
    W noisePower = W(0);
    W maxNoisePower = W(0);
    for (std::map<simtime_t, W>::const_iterator it = powerChanges->begin(); it != powerChanges->end(); it++) {
        noisePower += it->second;
        if (noisePower > maxNoisePower && startTime <= it->first && it->first <= endTime)
            maxNoisePower = noisePower;
    }
    return maxNoisePower;
}

} // namespace physicallayer

} // namespace inet

