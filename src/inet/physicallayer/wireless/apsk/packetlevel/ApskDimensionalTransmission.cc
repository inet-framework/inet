//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/ApskDimensionalTransmission.h"

namespace inet {

namespace physicallayer {

ApskDimensionalTransmission::ApskDimensionalTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, b headerLength, b payloadLength, const IModulation *modulation, const simtime_t symbolTime, Hz centerFrequency, Hz bandwidth, bps bitrate, double codeRate, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power) :
    DimensionalTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, headerLength, payloadLength, modulation, symbolTime, centerFrequency, bandwidth, bitrate, codeRate, power)
{
}

std::ostream& ApskDimensionalTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskDimensionalTransmission";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(power);
    return FlatTransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

