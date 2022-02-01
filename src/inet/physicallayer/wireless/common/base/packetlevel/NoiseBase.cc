//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/NoiseBase.h"

namespace inet {

namespace physicallayer {

NoiseBase::NoiseBase(simtime_t startTime, simtime_t endTime) :
    startTime(startTime),
    endTime(endTime)
{
}

std::ostream& NoiseBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(startTime)
               << EV_FIELD(endTime);
    return stream;
}

} // namespace physicallayer

} // namespace inet

