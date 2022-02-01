//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211DimensionalTransmission.h"

namespace inet {

namespace physicallayer {

Ieee80211DimensionalTransmission::Ieee80211DimensionalTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, const IModulation *modulation, b headerLength, b dataLength, Hz centerFrequency, Hz bandwidth, bps bitrate, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power, const IIeee80211Mode *mode, const Ieee80211Channel *channel) :
    DimensionalTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, modulation, headerLength, dataLength, centerFrequency, bandwidth, bitrate, power),
    Ieee80211TransmissionBase(mode, channel)
{
}

std::ostream& Ieee80211DimensionalTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211DimensionalTransmission";
    Ieee80211TransmissionBase::printToStream(stream, level);
    return DimensionalTransmission::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

