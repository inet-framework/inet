//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/ApskScalarTransmission.h"

namespace inet {

namespace physicallayer {

ApskScalarTransmission::ApskScalarTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, const IModulation *modulation, b headerLength, b payloadLength, Hz centerFrequency, Hz bandwidth, bps bitrate, W power) :
    ScalarTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, modulation, headerLength, payloadLength, centerFrequency, bandwidth, bitrate, power)
{
}

std::ostream& ApskScalarTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskScalarTransmission";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(power);
    return FlatTransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

