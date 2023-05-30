//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskNoise.h"

namespace inet {

namespace physicallayer {

UnitDiskNoise::UnitDiskNoise(simtime_t startTime, simtime_t endTime, Power minPower, Power maxPower) :
    NoiseBase(startTime, endTime),
    minPower(minPower),
    maxPower(maxPower)
{
    ASSERT(minPower <= maxPower);
}


W UnitDiskNoise::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    return minPower >= UnitDiskReceptionAnalogModel::POWER_DETECTABLE ? W(INFINITY) : W(0);
}

W UnitDiskNoise::computeMaxPower(simtime_t startTime, simtime_t endTime) const
{
    return maxPower >= UnitDiskReceptionAnalogModel::POWER_DETECTABLE ? W(INFINITY) : W(0);
}

std::ostream& UnitDiskNoise::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskNoise";
    if (level <= PRINT_LEVEL_DEBUG)
        stream << EV_FIELD(minPower)
               << EV_FIELD(maxPower);
    return NoiseBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

