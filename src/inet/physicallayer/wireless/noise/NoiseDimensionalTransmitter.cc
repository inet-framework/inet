//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/noise/NoiseDimensionalTransmitter.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmissionBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

Define_Module(NoiseDimensionalTransmitter);

void NoiseDimensionalTransmitter::initialize(int stage)
{
    TransmitterBase::initialize(stage);
    DimensionalTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        durationParameter = &par("duration");
        centerFrequencyParameter = &par("centerFrequency");
        bandwidthParameter = &par("bandwidth");
        powerParameter = &par("power");
    }
}

std::ostream& NoiseDimensionalTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "NoiseDimensionalTransmitter";
    TransmitterBase::printToStream(stream, level);
    return DimensionalTransmitterBase::printToStream(stream, level);
}

const ITransmission *NoiseDimensionalTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, simtime_t startTime) const
{
    Hz centerFrequency = Hz(centerFrequencyParameter->doubleValue());
    Hz bandwidth = Hz(bandwidthParameter->doubleValue());
    W power = W(powerParameter->doubleValue());
    const simtime_t duration = durationParameter->doubleValue();
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord& startPosition = mobility->getCurrentPosition();
    const Coord& endPosition = mobility->getCurrentPosition();
    const Quaternion& startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion& endOrientation = mobility->getCurrentAngularPosition();
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& powerFunction = createPowerFunction(startTime, endTime, centerFrequency, bandwidth, power);
    auto analogModel = getAnalogModel()->createAnalogModel(packet, 0, 0, duration, centerFrequency, bandwidth, power);
    return new FlatTransmissionBase(transmitter, nullptr, startTime, endTime, 0, 0, duration, startPosition, endPosition, startOrientation, endOrientation, nullptr, nullptr, nullptr, nullptr, analogModel, b(-1), b(-1), bps(NaN), -1, nullptr, -1, centerFrequency, bandwidth);
}

} // namespace physicallayer

} // namespace inet

