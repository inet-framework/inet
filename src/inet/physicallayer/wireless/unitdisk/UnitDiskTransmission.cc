//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskTransmission.h"

namespace inet {

namespace physicallayer {

UnitDiskTransmission::UnitDiskTransmission(const IRadio *transmitter, const Packet *macFrame, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, m communicationRange, m interferenceRange, m detectionRange) :
    TransmissionBase(transmitter, macFrame, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation),
    communicationRange(communicationRange),
    interferenceRange(interferenceRange),
    detectionRange(detectionRange)
{
}

std::ostream& UnitDiskTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskTransmission";
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(communicationRange);
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(interferenceRange)
               << EV_FIELD(detectionRange);
    return TransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

