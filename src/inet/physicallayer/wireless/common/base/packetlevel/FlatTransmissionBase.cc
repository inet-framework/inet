//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmissionBase.h"

namespace inet {

namespace physicallayer {

FlatTransmissionBase::FlatTransmissionBase(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, b headerLength, b dataLength, bps bitrate, double codeRate, const IModulation *modulation, const simtime_t symbolTime, Hz centerFrequency, Hz bandwidth) :
    NarrowbandTransmissionBase(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, modulation, symbolTime, centerFrequency, bandwidth),
    headerLength(headerLength),
    dataLength(dataLength),
    bitrate(bitrate),
    codeRate(codeRate)
{
}

std::ostream& FlatTransmissionBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(bitrate);
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(headerLength)
               << EV_FIELD(dataLength);
    return NarrowbandTransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

