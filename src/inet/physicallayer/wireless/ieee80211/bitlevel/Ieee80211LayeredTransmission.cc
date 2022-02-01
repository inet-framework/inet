//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211LayeredTransmission.h"

namespace inet {

namespace physicallayer {

Ieee80211LayeredTransmission::Ieee80211LayeredTransmission(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel, const ITransmissionAnalogModel *analogModel, const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, const IIeee80211Mode *mode, const Ieee80211Channel *channel) :
    LayeredTransmission(packetModel, bitModel, symbolModel, sampleModel, analogModel, transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation),
    Ieee80211TransmissionBase(mode, channel)
{
}

std::ostream& Ieee80211LayeredTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211LayeredTransmission";
    Ieee80211TransmissionBase::printToStream(stream, level);
    return LayeredTransmission::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

