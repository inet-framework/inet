//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/backgroundnoise/IsotropicScalarBackgroundNoise.h"

#include "inet/common/math/Functions.h"
#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarNoise.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"

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

std::ostream& IsotropicScalarBackgroundNoise::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "IsotropicScalarBackgroundNoise";
    if (level <= PRINT_LEVEL_DETAIL) {
        stream << EV_FIELD(power);
        stream << EV_FIELD(bandwidth);
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
    Hz noiseBandwidth = std::isnan(bandwidth.get()) ? listeningBandwidth : bandwidth;
    // A scalar background-noise power is the integrated power over its
    // configured bandwidth. Per-channel HT40 CCA listens to 20 MHz slices,
    // so preserve the equivalent flat noise density instead of rejecting a
    // narrower listening band.
    W listeningPower = power * (listeningBandwidth / noiseBandwidth);
    const auto& powerFunction = makeShared<math::Boxcar1DFunction<W, simtime_t>>(startTime, endTime, listeningPower);
    return new ScalarNoise(startTime, endTime, centerFrequency, listeningBandwidth, powerFunction);
}

} // namespace physicallayer

} // namespace inet

