//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/ApskTransmission.h"

namespace inet {

namespace physicallayer {

ApskTransmission::ApskTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, b headerLength, b dataLength, const IModulation *modulation, Hz bandwidth, const simtime_t symbolTime, bps bitrate, double codeRate) :
    TransmissionBase(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation),
    headerLength(headerLength), dataLength(dataLength), modulation(modulation), bandwidth(bandwidth), symbolTime(symbolTime), bitrate(bitrate), codeRate(codeRate)
{
}

std::ostream& ApskTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskTransmission";
    return TransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

