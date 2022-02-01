//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarReception.h"

namespace inet {

namespace physicallayer {

ScalarReception::ScalarReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, Hz centerFrequency, Hz bandwidth, W power) :
    FlatReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, centerFrequency, bandwidth),
    power(power)
{
}

std::ostream& ScalarReception::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ScalarReception";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(power);
    return FlatReceptionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

