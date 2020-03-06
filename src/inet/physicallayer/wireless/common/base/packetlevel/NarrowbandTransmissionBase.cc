//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandTransmissionBase.h"

namespace inet {

namespace physicallayer {

NarrowbandTransmissionBase::NarrowbandTransmissionBase(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, const IModulation *modulation, const simtime_t symbolTime, Hz centerFrequency, Hz bandwidth) :
    TransmissionBase(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation),
    modulation(modulation),
    symbolTime(symbolTime),
    centerFrequency(centerFrequency),
    bandwidth(bandwidth)
{
}

std::ostream& NarrowbandTransmissionBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(centerFrequency);
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(bandwidth)
               << EV_FIELD(modulation, printFieldToString(modulation, level + 1, evFlags));
    return TransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

