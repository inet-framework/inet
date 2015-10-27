//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmitterBase.h"

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
        setMode(modeSet != nullptr ? modeSet->getMode(bitrate) : nullptr);
        const char *bandName = par("bandName");
        setBand(*bandName != '\0' ? Ieee80211CompliantBands::getBand(bandName) : nullptr);
        int channelNumber = par("channelNumber");
        if (channelNumber != -1)
            setChannelNumber(channelNumber);
    }
}

std::ostream& Ieee80211TransmitterBase::printToStream(std::ostream& stream, int level) const
{
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", modeSet = " << printObjectToString(modeSet, level - 1)
               << ", band = " << printObjectToString(band, level - 1);
    if (level >= PRINT_LEVEL_INFO)
        stream << ", mode = " << printObjectToString(mode, level - 1)
               << ", channel = " << printObjectToString(channel, level - 1);
    return FlatTransmitterBase::printToStream(stream, level);
}

const IIeee80211Mode *Ieee80211TransmitterBase::computeTransmissionMode(const TransmissionRequest *transmissionRequest) const
{
    const Ieee80211TransmissionRequest *ieee80211TransmissionRequest = dynamic_cast<const Ieee80211TransmissionRequest *>(transmissionRequest);
    if (ieee80211TransmissionRequest != nullptr && ieee80211TransmissionRequest->getMode() != nullptr) {
        if (modeSet != nullptr && !modeSet->containsMode(ieee80211TransmissionRequest->getMode()))
            throw cRuntimeError("Unsupported mode requested");
        return ieee80211TransmissionRequest->getMode();
    }
    else if (modeSet != nullptr && transmissionRequest != nullptr && !std::isnan(transmissionRequest->getBitrate().get()))
        return modeSet->getMode(transmissionRequest->getBitrate());
    else
        return mode;
}

const Ieee80211Channel *Ieee80211TransmitterBase::computeTransmissionChannel(const TransmissionRequest *transmissionRequest) const
{
    const Ieee80211TransmissionRequest *ieee80211TransmissionRequest = dynamic_cast<const Ieee80211TransmissionRequest *>(transmissionRequest);
    if (ieee80211TransmissionRequest != nullptr && ieee80211TransmissionRequest->getChannel() != nullptr)
        return ieee80211TransmissionRequest->getChannel();
    else
        return channel;
}

W Ieee80211TransmitterBase::computeTransmissionPower(const TransmissionRequest *transmissionRequest) const
{
    if (transmissionRequest != nullptr && !std::isnan(transmissionRequest->getPower().get()))
        return transmissionRequest->getPower();
    else
        return power;
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
        if (modeSet->findMode(mode->getDataMode()->getNetBitrate()) == nullptr)
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
        setCarrierFrequency(channel->getCenterFrequency());
    }
}

void Ieee80211TransmitterBase::setChannelNumber(int channelNumber)
{
    if (channel == nullptr || channelNumber != channel->getChannelNumber())
        setChannel(new Ieee80211Channel(band, channelNumber));
}

} // namespace physicallayer

} // namespace inet

