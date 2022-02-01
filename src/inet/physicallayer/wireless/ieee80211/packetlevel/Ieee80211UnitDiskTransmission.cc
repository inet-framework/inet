//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211UnitDiskTransmission.h"

namespace inet {

namespace physicallayer {

Ieee80211UnitDiskTransmission::Ieee80211UnitDiskTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, m communicationRange, m interferenceRange, m detectionRange, const IIeee80211Mode *mode, const Ieee80211Channel *channel) :
    UnitDiskTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, communicationRange, interferenceRange, detectionRange),
    Ieee80211TransmissionBase(mode, channel)
{
}

std::ostream& Ieee80211UnitDiskTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211UnitDiskTransmission";
    Ieee80211TransmissionBase::printToStream(stream, level);
    return UnitDiskTransmission::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

