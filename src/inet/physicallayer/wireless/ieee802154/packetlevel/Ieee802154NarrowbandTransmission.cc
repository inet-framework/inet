//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154NarrowbandTransmission.h"

namespace inet {

namespace physicallayer {

Ieee802154NarrowbandTransmission::Ieee802154NarrowbandTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation) :
    TransmissionBase(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation)
{
}

std::ostream& Ieee802154NarrowbandTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee802154NarrowbandTransmission";
    return TransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

