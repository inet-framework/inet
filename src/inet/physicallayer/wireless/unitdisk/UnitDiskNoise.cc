//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskNoise.h"

namespace inet {

namespace physicallayer {

UnitDiskNoise::UnitDiskNoise(simtime_t startTime, simtime_t endTime, bool isInterfering) :
    NoiseBase(startTime, endTime),
    isInterfering_(isInterfering)
{
}

std::ostream& UnitDiskNoise::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskNoise";
    return NoiseBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

