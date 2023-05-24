//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154NarrowbandTransmitter.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskTransmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/RadioControlInfo_m.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee802154NarrowbandTransmitter);

Ieee802154NarrowbandTransmitter::Ieee802154NarrowbandTransmitter() :
    FlatTransmitterBase()
{
}

std::ostream& Ieee802154NarrowbandTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee802154NarrowbandTransmitter";
    return FlatTransmitterBase::printToStream(stream, level);
}

const ITransmission *Ieee802154NarrowbandTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    W transmissionPower = computeTransmissionPower(packet);
    bps transmissionBitrate = computeTransmissionDataBitrate(packet);
    const simtime_t headerDuration = b(headerLength).get() / bps(transmissionBitrate).get();
    const simtime_t dataDuration = b(packet->getDataLength()).get() / bps(transmissionBitrate).get();
    const simtime_t duration = preambleDuration + headerDuration + dataDuration;
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord& startPosition = mobility->getCurrentPosition();
    const Coord& endPosition = mobility->getCurrentPosition();
    const Quaternion& startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion& endOrientation = mobility->getCurrentAngularPosition();
    auto symbolTime = 0;
    auto transmission = new ApskTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, headerLength, packet->getDataLength(), modulation, bandwidth, symbolTime, transmissionBitrate, codeRate);
    transmission->newAnalogModel = getAnalogModel()->createAnalogModel(packet, duration, centerFrequency, bandwidth, transmissionPower);
    return transmission;
}

} // namespace physicallayer
} // namespace inet

