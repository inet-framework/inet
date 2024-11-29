//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/ApskTransmitter.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskTransmission.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskPhyHeader_m.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskTransmitter);

std::ostream& ApskTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskTransmitter";
    return FlatTransmitterBase::printToStream(stream, level);
}

const ITransmission *ApskTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    auto phyHeader = packet->peekAtFront<ApskPhyHeader>();
    ASSERT(phyHeader->getChunkLength() == headerLength);
    auto dataLength = packet->getDataLength() - phyHeader->getChunkLength();
    // TODO move these to analog model
    W transmissionPower = computeTransmissionPower(packet);
    Hz transmissionCenterFrequency = computeCenterFrequency(packet);
    Hz transmissionBandwidth = computeBandwidth(packet);
    bps transmissionBitrate = computeTransmissionDataBitrate(packet);
    const simtime_t headerDuration = headerLength.get<b>() / transmissionBitrate.get<bps>();
    const simtime_t dataDuration = dataLength.get<b>() / transmissionBitrate.get<bps>();
    const simtime_t duration = preambleDuration + headerDuration + dataDuration;
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord& startPosition = mobility->getCurrentPosition();
    const Coord& endPosition = mobility->getCurrentPosition();
    const Quaternion& startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion& endOrientation = mobility->getCurrentAngularPosition();
    auto analogModel = getAnalogModel()->createAnalogModel(preambleDuration, headerDuration, dataDuration, transmissionCenterFrequency, transmissionBandwidth, transmissionPower);
    return new ApskTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, nullptr, nullptr, nullptr, nullptr, analogModel, headerLength, dataLength, modulation, bandwidth, -1, transmissionBitrate, codeRate);
}

} // namespace physicallayer

} // namespace inet

