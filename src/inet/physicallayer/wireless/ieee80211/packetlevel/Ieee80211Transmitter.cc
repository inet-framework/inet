//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmitter.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211Transmitter);

Ieee80211Transmitter::Ieee80211Transmitter() :
    Ieee80211TransmitterBase()
{
}

std::ostream& Ieee80211Transmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211Transmitter";
    return Ieee80211TransmitterBase::printToStream(stream, level);
}

const ITransmission *Ieee80211Transmitter::createTransmission(const IRadio *transmitter, const Packet *packet, simtime_t startTime) const
{
    auto phyHeader = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
    const IIeee80211Mode *transmissionMode = computeTransmissionMode(packet);
    const Ieee80211Channel *transmissionChannel = computeTransmissionChannel(packet);
    W transmissionPower = computeTransmissionPower(packet);
    Hz transmissionBandwidth = transmissionMode->getDataMode()->getBandwidth();
    bps transmissionBitrate = transmissionMode->getDataMode()->getNetBitrate();
    if (transmissionMode->getDataMode()->getNumberOfSpatialStreams() > transmitter->getAntenna()->getNumAntennas())
        throw cRuntimeError("Number of spatial streams is higher than the number of antennas");
    const simtime_t duration = transmissionMode->getDuration(B(phyHeader->getLengthField()));
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord& startPosition = mobility->getCurrentPosition();
    const Coord& endPosition = mobility->getCurrentPosition();
    const Quaternion& startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion& endOrientation = mobility->getCurrentAngularPosition();
    auto headerLength = b(transmissionMode->getHeaderMode()->getLength());
    auto dataLength = b(transmissionMode->getDataMode()->getCompleteLength(B(phyHeader->getLengthField())));
    const simtime_t preambleDuration = transmissionMode->getPreambleMode()->getDuration();
    const simtime_t headerDuration = transmissionMode->getHeaderMode()->getDuration();
    const simtime_t dataDuration = duration - headerDuration - preambleDuration;
    auto dataModulation = transmissionMode->getDataMode()->getModulation();
    auto dataSymbolTime = transmissionMode->getDataMode()->getSymbolInterval();
    auto transmission = new Ieee80211Transmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, headerLength, dataLength, dataModulation, dataSymbolTime, centerFrequency, transmissionBandwidth, transmissionBitrate, codeRate, transmissionPower, transmissionMode, transmissionChannel);
    transmission->analogModel = getAnalogModel()->createAnalogModel(packet, duration, centerFrequency, transmissionBandwidth, transmissionPower);
    return transmission;
}

} // namespace physicallayer

} // namespace inet

