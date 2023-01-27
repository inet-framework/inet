//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211UnitDiskTransmitter.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211UnitDiskTransmission.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211UnitDiskTransmitter);

Ieee80211UnitDiskTransmitter::Ieee80211UnitDiskTransmitter() :
    Ieee80211TransmitterBase()
{
}

std::ostream& Ieee80211UnitDiskTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211UnitDiskTransmitter";
    return Ieee80211TransmitterBase::printToStream(stream, level);
}

const ITransmission *Ieee80211UnitDiskTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, simtime_t startTime) const
{
    const IIeee80211Mode *transmissionMode = computeTransmissionMode(packet);
    if (transmissionMode->getDataMode()->getNumberOfSpatialStreams() > transmitter->getAntenna()->getNumAntennas())
        throw cRuntimeError("Number of spatial streams is higher than the number of antennas");
    const simtime_t duration = transmissionMode->getDuration(packet->getDataLength() - b(48));
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord& startPosition = mobility->getCurrentPosition();
    const Coord& endPosition = mobility->getCurrentPosition();
    const Quaternion& startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion& endOrientation = mobility->getCurrentAngularPosition();
    const simtime_t preambleDuration = transmissionMode->getPreambleMode()->getDuration();
    const simtime_t headerDuration = transmissionMode->getHeaderMode()->getDuration();
    const simtime_t dataDuration = duration - headerDuration - preambleDuration;
    auto transmission = new Ieee80211UnitDiskTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, transmissionMode, channel);
    transmission->analogModel = getAnalogModel()->createAnalogModel(packet);
    return transmission;
}

} // namespace physicallayer
} // namespace inet

