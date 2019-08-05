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

#include "inet/physicallayer/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/backgroundnoise/IsotropicDimensionalBackgroundNoise.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(IsotropicDimensionalBackgroundNoise);

IsotropicDimensionalBackgroundNoise::IsotropicDimensionalBackgroundNoise() :
    power(W(NaN))
{
}

void IsotropicDimensionalBackgroundNoise::initialize(int stage)
{
    cModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        power = mW(dBmW2mW(par("power")));
    }
}

std::ostream& IsotropicDimensionalBackgroundNoise::printToStream(std::ostream& stream, int level) const
{
    stream << "IsotropicDimensionalBackgroundNoise";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", power = " << power;
    return stream;
}

const INoise *IsotropicDimensionalBackgroundNoise::computeNoise(const IListening *listening) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const simtime_t startTime = listening->getStartTime();
    const simtime_t endTime = listening->getEndTime();
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    // NOTE: dividing by the bandwidth here makes sure the total background noise power in the listening band is the given power
    const Ptr<const IFunction<WpHz, Domain<simtime_t, Hz>>>& powerFunction = makeShared<ConstantFunction<WpHz, Domain<simtime_t, Hz>>>(power / bandwidth);
    return new DimensionalNoise(startTime, endTime, carrierFrequency, bandwidth, powerFunction);
}

} // namespace physicallayer

} // namespace inet

