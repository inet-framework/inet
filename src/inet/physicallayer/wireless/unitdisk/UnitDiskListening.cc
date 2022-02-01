//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskListening.h"

namespace inet {

namespace physicallayer {

UnitDiskListening::UnitDiskListening(const IRadio *radio, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) :
    ListeningBase(radio, startTime, endTime, startPosition, endPosition)
{
}

std::ostream& UnitDiskListening::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskListening";
    return ListeningBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

