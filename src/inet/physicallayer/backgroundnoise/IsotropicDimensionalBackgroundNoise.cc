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

void IsotropicDimensionalBackgroundNoise::initialize(int stage)
{
    cModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        powerSpectralDensity = WpHz(dBmWpMHz2WpHz(par("powerSpectralDensity")));
        power = mW(dBmW2mW(par("power")));
        bandwidth = Hz(par("bandwidth"));
        if (std::isnan(powerSpectralDensity.get()) && std::isnan(power.get()))
            throw cRuntimeError("One of powerSpectralDensity or power parameters must be specified");
        if (!std::isnan(powerSpectralDensity.get()) && !std::isnan(power.get()))
            throw cRuntimeError("Both of powerSpectralDensity and power parameters cannot be specified");
    }
}

std::ostream& IsotropicDimensionalBackgroundNoise::printToStream(std::ostream& stream, int level) const
{
    stream << "IsotropicDimensionalBackgroundNoise";
    if (level <= PRINT_LEVEL_DETAIL) {
        if (!std::isnan(powerSpectralDensity.get()))
            stream << ", powerSpectralDensity = " << powerSpectralDensity;
        else {
            stream << ", power = " << power;
            stream << ", bandwidth = " << bandwidth;
        }
    }
    return stream;
}

const INoise *IsotropicDimensionalBackgroundNoise::computeNoise(const IListening *listening) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz centerFrequency = bandListening->getCenterFrequency();
    Hz listeningBandwidth = bandListening->getBandwidth();
    WpHz noisePowerSpectralDensity;
    if (!std::isnan(powerSpectralDensity.get()))
        noisePowerSpectralDensity = powerSpectralDensity;
    else {
        if (std::isnan(bandwidth.get()))
            bandwidth = listeningBandwidth;
        else if (bandwidth != listeningBandwidth)
            throw cRuntimeError("The powerSpectralDensity parameter is not specified and the power parameter cannot be used, because background noise bandwidth doesn't match listening bandwidth");
        // NOTE: dividing by the bandwidth here makes sure the total background noise power in the listening band is the given power
        noisePowerSpectralDensity = power / bandwidth;
    }
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& powerFunction = makeShared<ConstantFunction<WpHz, Domain<simsec, Hz>>>(noisePowerSpectralDensity);
    const simtime_t startTime = listening->getStartTime();
    const simtime_t endTime = listening->getEndTime();
    return new DimensionalNoise(startTime, endTime, centerFrequency, bandwidth, makeFirstQuadrantLimitedFunction(powerFunction));
}

} // namespace physicallayer

} // namespace inet

