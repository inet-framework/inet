//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    simtime_t previousTime;
    for (const auto& elem : *powerChanges) {
        if ((std::isnan(minNoisePower.get()) || noisePower < minNoisePower) && previousTime < startTime && elem.first > endTime)
            minNoisePower = noisePower;
        noisePower += elem.second;
        if ((std::isnan(minNoisePower.get()) || noisePower < minNoisePower) && startTime <= elem.first && elem.first < endTime)
            minNoisePower = noisePower;
        previousTime = elem.first;
    }
    if (std::isnan(minNoisePower.get()))
        minNoisePower = previousTime < startTime ? W(0) : noisePower;
    return minNoisePower;
}

W ScalarNoise::computeMaxPower(simtime_t startTime, simtime_t endTime) const
{
    W noisePower = W(0);
    W maxNoisePower = W(NaN);
    simtime_t previousTime;
    for (const auto& elem : *powerChanges) {
        if ((std::isnan(maxNoisePower.get()) || noisePower > maxNoisePower) && previousTime < startTime && elem.first > endTime)
            maxNoisePower = noisePower;
        noisePower += elem.second;
        if ((std::isnan(maxNoisePower.get()) || noisePower > maxNoisePower) && startTime <= elem.first && elem.first < endTime)
            maxNoisePower = noisePower;
        previousTime = elem.first;
    }
    if (std::isnan(maxNoisePower.get()))
        maxNoisePower = previousTime < startTime ? W(0) : noisePower;
    return maxNoisePower;
}

} // namespace physicallayer

} // namespace inet

