//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandNoiseBase.h"

namespace inet {

namespace physicallayer {

NarrowbandNoiseBase::NarrowbandNoiseBase(simtime_t startTime, simtime_t endTime, Hz centerFrequency, Hz bandwidth) :
    NoiseBase(startTime, endTime),
    centerFrequency(centerFrequency),
    bandwidth(bandwidth)
{
}

std::ostream& NarrowbandNoiseBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(centerFrequency)
               << EV_FIELD(bandwidth);
    return NoiseBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

