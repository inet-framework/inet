//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211TransmitterBase.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace physicallayer {

Ieee80211TransmitterBase::Ieee80211TransmitterBase() :
    modeSet(nullptr),
    mode(nullptr),
    band(nullptr),
    channel(nullptr)
{
}

Ieee80211TransmitterBase::~Ieee80211TransmitterBase()
{
    delete channel;
}

void Ieee80211TransmitterBase::initialize(int stage)
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

std::ostream& Ieee80211TransmitterBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(modeSet, printFieldToString(modeSet, level + 1, evFlags))
               << EV_FIELD(band, printFieldToString(band, level + 1, evFlags));
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(mode, printFieldToString(mode, level + 1, evFlags))
               << EV_FIELD(channel, printFieldToString(channel, level + 1, evFlags));
    return FlatTransmitterBase::printToStream(stream, level);
}

const IIeee80211Mode *Ieee80211TransmitterBase::computeTransmissionMode(const Packet *packet) const
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

const Ieee80211Channel *Ieee80211TransmitterBase::computeTransmissionChannel(const Packet *packet) const
{
    const Ieee80211Channel *transmissionChannel;
    const auto& channelReq = const_cast<Packet *>(packet)->findTag<Ieee80211ChannelReq>();
    transmissionChannel = channelReq != nullptr ? channelReq->getChannel() : channel;
    if (transmissionChannel == nullptr)
        throw cRuntimeError("Transmission channel is undefined");
    return transmissionChannel;

}

void Ieee80211TransmitterBase::setModeSet(const Ieee80211ModeSet *modeSet)
{
    if (this->modeSet != modeSet) {
        this->modeSet = modeSet;
        if (mode != nullptr)
            mode = modeSet != nullptr ? modeSet->getMode(mode->getDataMode()->getNetBitrate()) : nullptr;
    }
}

void Ieee80211TransmitterBase::setMode(const IIeee80211Mode *mode)
{
    if (this->mode != mode) {
        if (modeSet->findMode(mode->getDataMode()->getNetBitrate(), mode->getDataMode()->getBandwidth()) == nullptr)
            throw cRuntimeError("Invalid mode");
        this->mode = mode;
    }
}

void Ieee80211TransmitterBase::setBand(const IIeee80211Band *band)
{
    if (this->band != band) {
        this->band = band;
        if (channel != nullptr)
            setChannel(new Ieee80211Channel(band, channel->getChannelNumber()));
    }
}

void Ieee80211TransmitterBase::setChannel(const Ieee80211Channel *channel)
{
    if (this->channel != channel) {
        delete this->channel;
        this->channel = channel;
        setCenterFrequency(channel->getCenterFrequency());
    }
}

void Ieee80211TransmitterBase::setChannelNumber(int channelNumber)
{
    if (channel == nullptr || channelNumber != channel->getChannelNumber())
        setChannel(new Ieee80211Channel(band, channelNumber));
}

} // namespace physicallayer
} // namespace inet

