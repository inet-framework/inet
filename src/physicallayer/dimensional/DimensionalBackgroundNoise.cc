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

#include "DimensionalBackgroundNoise.h"
#include "DimensionalNoise.h"
#include "DimensionalUtils.h"
#include "BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(DimensionalBackgroundNoise);

void DimensionalBackgroundNoise::initialize(int stage)
{
    cModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        power = mW(math::dBm2mW(par("power")));
    }
}

const INoise *DimensionalBackgroundNoise::computeNoise(const IListening *listening) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const simtime_t startTime = listening->getStartTime();
    const simtime_t endTime = listening->getEndTime();
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    const ConstMapping *powerMapping = DimensionalUtils::createFlatMapping(startTime, endTime, carrierFrequency, bandwidth, power);
    return new DimensionalNoise(startTime, endTime, carrierFrequency, bandwidth, powerMapping);
}

} // namespace physicallayer

} // namespace inet

