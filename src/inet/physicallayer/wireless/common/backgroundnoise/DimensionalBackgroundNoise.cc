//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/backgroundnoise/DimensionalBackgroundNoise.h"

#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"

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

std::ostream& DimensionalBackgroundNoise::printToStream(std::ostream& stream, int level, int evFlags) const
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

