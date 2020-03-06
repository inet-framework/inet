//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154NarrowbandDimensionalTransmitter.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/RadioControlInfo_m.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee802154NarrowbandDimensionalTransmitter);

Ieee802154NarrowbandDimensionalTransmitter::Ieee802154NarrowbandDimensionalTransmitter() :
    FlatTransmitterBase(),
    DimensionalTransmitterBase()
{
}

void Ieee802154NarrowbandDimensionalTransmitter::initialize(int stage)
{
    FlatTransmitterBase::initialize(stage);
    DimensionalTransmitterBase::initialize(stage);
}

std::ostream& Ieee802154NarrowbandDimensionalTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee802154NarrowbandDimensionalTransmitter";
    DimensionalTransmitterBase::printToStream(stream, level);
    return DimensionalTransmitterBase::printToStream(stream, level);
}

const ITransmission *Ieee802154NarrowbandDimensionalTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    W transmissionPower = computeTransmissionPower(packet);
    bps transmissionBitrate = computeTransmissionDataBitrate(packet);
    const simtime_t headerDuration = b(headerLength).get() / bps(transmissionBitrate).get();
    const simtime_t dataDuration = b(packet->getTotalLength()).get() / bps(transmissionBitrate).get();
    const simtime_t duration = preambleDuration + headerDuration + dataDuration;
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& powerFunction = createPowerFunction(startTime, endTime, centerFrequency, bandwidth, transmissionPower);
    const Coord& startPosition = mobility->getCurrentPosition();
    const Coord& endPosition = mobility->getCurrentPosition();
    const Quaternion& startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion& endOrientation = mobility->getCurrentAngularPosition();
    auto symbolTime = 0;
    return new DimensionalTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, headerLength, packet->getTotalLength(), modulation, symbolTime, centerFrequency, bandwidth, transmissionBitrate, codeRate, powerFunction);
}

} // namespace physicallayer

} // namespace inet

