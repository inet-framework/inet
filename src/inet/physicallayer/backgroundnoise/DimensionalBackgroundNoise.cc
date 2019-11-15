//
// Copyright (C) OpenSim Ltd.
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

#include "inet/physicallayer/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/backgroundnoise/DimensionalBackgroundNoise.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(DimensionalBackgroundNoise);

DimensionalBackgroundNoise::DimensionalBackgroundNoise() :
    power(W(NaN))
{
}

void DimensionalBackgroundNoise::initialize(int stage)
{
    DimensionalTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        power = mW(dBmW2mW(par("power")));
    }
}

std::ostream& DimensionalBackgroundNoise::printToStream(std::ostream& stream, int level) const
{
    stream << "DimensionalBackgroundNoise";
    return DimensionalTransmitterBase::printToStream(stream, level);
}

const INoise *DimensionalBackgroundNoise::computeNoise(const IListening *listening) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const simtime_t startTime = listening->getStartTime();
    const simtime_t endTime = listening->getEndTime();
    Hz centerFrequency = bandListening->getCenterFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    const auto& powerFunction = createPowerFunction(startTime, endTime, centerFrequency, bandwidth, power);
    return new DimensionalNoise(startTime, endTime, centerFrequency, bandwidth, powerFunction);
}

} // namespace physicallayer

} // namespace inet

