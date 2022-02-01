//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandReceptionBase.h"

namespace inet {

namespace physicallayer {

NarrowbandReceptionBase::NarrowbandReceptionBase(const IRadio *receiver, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, Hz centerFrequency, Hz bandwidth) :
    ReceptionBase(receiver, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation),
    centerFrequency(centerFrequency),
    bandwidth(bandwidth)
{
}

std::ostream& NarrowbandReceptionBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(centerFrequency);
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(bandwidth);
    return ReceptionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

