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
#include "inet/physicallayer/backgroundnoise/IsotropicScalarBackgroundNoise.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(IsotropicScalarBackgroundNoise);

void IsotropicScalarBackgroundNoise::initialize(int stage)
{
    cModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        power = mW(math::dBmW2mW(par("power")));
        bandwidth = Hz(par("bandwidth"));
    }
}

std::ostream& IsotropicScalarBackgroundNoise::printToStream(std::ostream& stream, int level) const
{
    stream << "IsotropicScalarBackgroundNoise";
    if (level <= PRINT_LEVEL_DETAIL) {
        stream << ", power = " << power;
        stream << ", bandwidth = " << bandwidth;
    }
    return stream;
}

const INoise *IsotropicScalarBackgroundNoise::computeNoise(const IListening *listening) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    simtime_t startTime = listening->getStartTime();
    simtime_t endTime = listening->getEndTime();
    Hz centerFrequency = bandListening->getCenterFrequency();
    Hz listeningBandwidth = bandListening->getBandwidth();
    if (std::isnan(bandwidth.get()))
        bandwidth = listeningBandwidth;
    else if (bandwidth != listeningBandwidth)
        throw cRuntimeError("Background noise bandwidth doesn't match listening bandwidth");
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
    powerChanges->insert(std::pair<simtime_t, W>(startTime, power));
    powerChanges->insert(std::pair<simtime_t, W>(endTime, -power));
    return new ScalarNoise(startTime, endTime, centerFrequency, bandwidth, powerChanges);
}

} // namespace physicallayer

} // namespace inet

