//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmitter.h"
#include "inet/linklayer/ieee80211/mac/aggregation/Ieee80211Psdu.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarTransmitterAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211Transmitter);

Ieee80211Transmitter::~Ieee80211Transmitter()
{
    delete channel;
}

void Ieee80211Transmitter::initialize(int stage)
{
    FlatTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *opMode = par("opMode");
        setModeSet(*opMode ? Ieee80211ModeSet::getModeSet(opMode) : nullptr);
        const char *bandName = par("bandName");
        setBand(*bandName != '\0' ? Ieee80211CompliantBands::getBand(bandName) : nullptr);
        setMode(modeSet != nullptr ? (bitrate != bps(-1) ? modeSet->getMode(bitrate, bandwidth) : modeSet->getFastestMode()) : nullptr);
        int channelNumber = par("channelNumber");
        if (channelNumber != -1)
            setChannelNumber(channelNumber);
    }
}

const IIeee80211Mode *Ieee80211Transmitter::computeTransmissionMode(const Packet *packet) const
{
    const IIeee80211Mode *transmissionMode;
    const auto& modeReq = const_cast<Packet *>(packet)->findTag<Ieee80211ModeReq>();
    const auto& bitrateReq = const_cast<Packet *>(packet)->findTag<SignalBitrateReq>();
    if (modeReq != nullptr) {
        if (modeSet != nullptr && !modeSet->containsMode(modeReq->getMode()))
            throw cRuntimeError("Unsupported mode requested");
        transmissionMode = modeReq->getMode();
    }
    else if (modeSet != nullptr && bitrateReq != nullptr)
        transmissionMode = modeSet->getMode(bitrateReq->getDataBitrate());
    else
        transmissionMode = mode;
    if (transmissionMode == nullptr)
        throw cRuntimeError("Transmission mode is undefined");
    return transmissionMode;
}

const Ieee80211Channel *Ieee80211Transmitter::computeTransmissionChannel(const Packet *packet) const
{
    const Ieee80211Channel *transmissionChannel;
    const auto& channelReq = const_cast<Packet *>(packet)->findTag<Ieee80211ChannelReq>();
    transmissionChannel = channelReq != nullptr ? channelReq->getChannel() : channel;
    if (transmissionChannel == nullptr)
        throw cRuntimeError("Transmission channel is undefined");
    if (channel && channel->hasAllocation()) {
        // The fixed allocation belongs to the radio for its entire lifetime.
        // A packet request may select that value, but cannot introduce a
        // different allocation or a shorter-lived pointer into indications.
        if (!transmissionChannel->hasAllocation() || transmissionChannel->getBand() != channel->getBand()
                || transmissionChannel->getChannelNumber() != channel->getChannelNumber()
                || transmissionChannel->getAllocationWidth() != channel->getAllocationWidth())
            throw cRuntimeError("Per-packet channel request differs from the fixed contiguous allocation");
        return channel;
    }
    return transmissionChannel;
}

void Ieee80211Transmitter::setModeSet(const Ieee80211ModeSet *modeSet)
{
    if (this->modeSet != modeSet) {
        this->modeSet = modeSet;
        if (mode != nullptr)
            mode = modeSet != nullptr ? modeSet->getMode(mode->getDataMode()->getNetBitrate()) : nullptr;
    }
}

void Ieee80211Transmitter::setMode(const IIeee80211Mode *mode)
{
    if (this->mode != mode) {
        if (modeSet->findMode(mode->getDataMode()->getNetBitrate(), mode->getDataMode()->getBandwidth()) == nullptr)
            throw cRuntimeError("Invalid mode");
        this->mode = mode;
    }
}

void Ieee80211Transmitter::setBand(const IIeee80211Band *band)
{
    if (this->band != band) {
        this->band = band;
        if (channel != nullptr)
            setChannel(new Ieee80211Channel(band, channel->getChannelNumber()));
    }
}

void Ieee80211Transmitter::setChannel(const Ieee80211Channel *channel)
{
    if (this->channel != channel) {
        delete this->channel;
        this->channel = channel;
        this->band = channel->getBand();
        setCenterFrequency(channel->getCenterFrequency());
    }
}

void Ieee80211Transmitter::setChannelNumber(int channelNumber)
{
    if (channel == nullptr || channelNumber != channel->getChannelNumber())
        setChannel(new Ieee80211Channel(band, channelNumber));
}

bool Ieee80211Transmitter::isHtChannelWidthSupported(Hz channelWidth) const
{
    // The packet-level PHY currently represents only the primary channel.
    // Until a primary/secondary compound channel determines the analog-model
    // center frequency, advertising a 40 MHz HT width would be false.
    return (channelWidth == MHz(20) || (channel && channel->hasAllocation() && channelWidth <= channel->getAllocationWidth()))
        && modeSet != nullptr && modeSet->getHtSupportedChannelWidths().count(channelWidth) != 0;
}

Ieee80211TxVector Ieee80211Transmitter::computeTransmissionVector(const Packet *packet) const
{
    auto request = packet->findTag<Ieee80211ModeReq>();
    auto vector = request ? request->getTxVector() : Ieee80211TxVector(computeTransmissionMode(packet));
    if (!vector.mode || !modeSet->containsMode(vector.mode))
        throw cRuntimeError("Invalid IEEE80211 transmit vector mode");
    if ((vector.singleMpdu && (vector.format != Ieee80211TxVector::Format::VHT || vector.ampdu))
            || (vector.ampdu && vector.format != Ieee80211TxVector::Format::HT_MIXED && vector.format != Ieee80211TxVector::Format::VHT)
            || (vector.format != Ieee80211TxVector::Format::NON_HT_DUPLICATE
                && vector.format != Ieee80211TxVector(vector.mode).format)
            || (channel && channel->hasAllocation() && vector.mode->getVhtMcsIndex() >= 0 && !vector.singleMpdu && !vector.ampdu))
        throw cRuntimeError("IEEE80211 PPDU format/PSDU framing disagrees with the actual PHY mode");
    if (vector.format == Ieee80211TxVector::Format::NON_HT_DUPLICATE) {
        if (!channel || !channel->hasAllocation() || vector.mode->getHtMcsIndex() >= 0 || vector.mode->getVhtMcsIndex() >= 0
                || vector.mode->getDataMode()->getBandwidth() != MHz(20) || vector.bandwidth <= MHz(20))
            throw cRuntimeError("Invalid non-HT duplicate transmit vector");
    }
    else if (vector.bandwidth != vector.mode->getDataMode()->getBandwidth())
        throw cRuntimeError("IEEE80211 occupied width disagrees with the selected mode");
    if (channel && channel->hasAllocation())
        channel->getCenterChannel(vector.bandwidth); // validates the selected nested width
    return vector;
}

std::ostream& Ieee80211Transmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211Transmitter";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(modeSet, printFieldToString(modeSet, level + 1, evFlags))
               << EV_FIELD(band, printFieldToString(band, level + 1, evFlags));
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(mode, printFieldToString(mode, level + 1, evFlags))
               << EV_FIELD(channel, printFieldToString(channel, level + 1, evFlags));
    return FlatTransmitterBase::printToStream(stream, level);
}

const ITransmission *Ieee80211Transmitter::createTransmission(const IRadio *transmitter, const Packet *packet, simtime_t startTime) const
{
    auto phyHeader = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
    const IIeee80211Mode *transmissionMode = computeTransmissionMode(packet);
    const Ieee80211Channel *transmissionChannel = computeTransmissionChannel(packet);
    W transmissionPower = computeTransmissionPower(packet);
    auto vector = computeTransmissionVector(packet);
    Hz transmissionBandwidth = vector.bandwidth;
    Hz transmissionCenter = transmissionChannel->hasAllocation() ? transmissionChannel->getCenterFrequency(transmissionBandwidth) : centerFrequency;
    if (transmissionMode->getDataMode()->getNumberOfSpatialStreams() > transmitter->getAntenna()->getNumAntennas())
        throw cRuntimeError("Number of spatial streams is higher than the number of antennas");
    B durationLength = phyHeader->getLengthField();
    if (vector.ampdu) {
        ieee80211::Ieee80211AmpduLayout layout;
        bool vht = vector.format == Ieee80211TxVector::VHT;
        if (!ieee80211::findAmpdu(packet,phyHeader->getChunkLength(),phyHeader->getLengthField(),vht,layout)
                || (vht && !ieee80211::isVhtPsduWithinCapacity(phyHeader->getLengthField(),layout,transmissionMode)))
            throw cRuntimeError("Cannot transmit an invalid A-MPDU PSDU");
        if (vht)
            durationLength = layout.preEofLength;
    }
    else if (vector.singleMpdu) {
        ieee80211::Ieee80211SingleMpduLayout layout;
        if (!ieee80211::findSingleMpdu(packet,phyHeader->getChunkLength(),phyHeader->getLengthField(),layout))
            throw cRuntimeError("Cannot transmit an invalid VHT single-member PSDU");
        durationLength = layout.preEofLength;
    }
    const simtime_t duration = transmissionMode->getDuration(durationLength);
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord& startPosition = mobility->getCurrentPosition();
    const Coord& endPosition = mobility->getCurrentPosition();
    const Quaternion& startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion& endOrientation = mobility->getCurrentAngularPosition();
    const simtime_t preambleDuration = transmissionMode->getPreambleMode()->getDuration();
    const simtime_t headerDuration = transmissionMode->getHeaderMode()->getDuration();
    const simtime_t dataDuration = duration - headerDuration - preambleDuration;
    auto analogModel = getAnalogModel()->createAnalogModel(preambleDuration, headerDuration, dataDuration, transmissionCenter, transmissionBandwidth, transmissionPower);
    auto transmission = new Ieee80211Transmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, nullptr, nullptr, nullptr, nullptr, analogModel, transmissionMode, transmissionChannel);
    transmission->setTxVector(vector);
    return transmission;
}

} // namespace physicallayer

} // namespace inet

