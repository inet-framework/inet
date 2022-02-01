//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/ListeningBase.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

ListeningBase::ListeningBase(const IRadio *receiver, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) :
    receiver(receiver),
    startTime(startTime),
    endTime(endTime),
    startPosition(startPosition),
    endPosition(endPosition)
{
}

std::ostream& ListeningBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(receiverId, receiver->getId())
               << EV_FIELD(startTime)
               << EV_FIELD(endTime)
               << EV_FIELD(startPosition)
               << EV_FIELD(endPosition);
    return stream;
}

} // namespace physicallayer

} // namespace inet

