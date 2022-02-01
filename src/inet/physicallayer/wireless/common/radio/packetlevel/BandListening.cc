//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"

namespace inet {
namespace physicallayer {

BandListening::BandListening(const IRadio *radio, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition, Hz centerFrequency, Hz bandwidth) :
    ListeningBase(radio, startTime, endTime, startPosition, endPosition),
    centerFrequency(centerFrequency),
    bandwidth(bandwidth)
{
}

std::ostream& BandListening::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "BandListening";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(centerFrequency)
               << EV_FIELD(bandwidth);
    return ListeningBase::printToStream(stream, level);
}

} // namespace physicallayer
} // namespace inet

