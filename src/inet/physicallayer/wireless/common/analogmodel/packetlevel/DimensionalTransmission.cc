//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalTransmission.h"

namespace inet {

namespace physicallayer {

DimensionalTransmission::DimensionalTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, b headerLength, b dataLength, const IModulation *modulation, const simtime_t symbolTime, Hz centerFrequency, Hz bandwidth, bps bitrate, double codeRate, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power) :
    FlatTransmissionBase(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, headerLength, dataLength, bitrate, codeRate, modulation, symbolTime, centerFrequency, bandwidth),
    power(power)
{
}

std::ostream& DimensionalTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "DimensionalTransmission";
//    if (level <= PRINT_LEVEL_DEBUG)
//        stream << EV_FIELD(powerMax, power->getMax())
//               << EV_FIELD(powerMin, power->getMin());
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(power);
    return FlatTransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

