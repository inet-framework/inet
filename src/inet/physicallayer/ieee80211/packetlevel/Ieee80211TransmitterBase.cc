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

#include "inet/physicallayer/common/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmitterBase.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211VhtMode.h"

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
        const char *bandName = par("bandName");
        setBand(*bandName != '\0' ? Ieee80211CompliantBands::getBand(bandName) : nullptr);
        setModeSet(*opMode ? Ieee80211ModeSet::getModeSet(opMode) : nullptr);

        if (strcmp(opMode, "ac") == 0) {
            if (band == nullptr)
                throw cRuntimeError("AC mode required a valid bandwidth");
            if (strcmp(band->getName(), "5 GHz&20 MHz") == 0) {
                setMode(modeSet != nullptr ? (bitrate != bps(-1) ? modeSet->getMode(bitrate, Hz(20e6)) : nullptr) : nullptr);
            }
            else if (strcmp(band->getName(), "5 GHz&40 MHz") == 0) {
                setMode(modeSet != nullptr ? (bitrate != bps(-1) ? modeSet->getMode(bitrate, Hz(40e6)) : nullptr) : nullptr);
            }
            else if (strcmp(band->getName(), "5 GHz&80 MHz") == 0) {
                setMode(modeSet != nullptr ? (bitrate != bps(-1) ? modeSet->getMode(bitrate, Hz(80e6)) : nullptr) : nullptr);
            }
            else if (strcmp(band->getName(), "5 GHz&160 MHz") == 0) {
                setMode(modeSet != nullptr ? (bitrate != bps(-1) ? modeSet->getMode(bitrate, Hz(160e6)) : nullptr) : nullptr);
            }
            else
                throw cRuntimeError("Invalid bandwidth for AC mode");
        }
        else
            setMode(modeSet != nullptr ? (bitrate != bps(-1) ? modeSet->getMode(bitrate) : nullptr) : nullptr);

        int channelNumber = par("channelNumber");
        if (channelNumber != -1)
            setChannelNumber(channelNumber);
    }
}

std::ostream& Ieee80211TransmitterBase::printToStream(std::ostream& stream, int level) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", modeSet = " << printObjectToString(modeSet, level + 1)
               << ", band = " << printObjectToString(band, level + 1);
    if (level <= PRINT_LEVEL_INFO)
        stream << ", mode = " << printObjectToString(mode, level + 1)
               << ", channel = " << printObjectToString(channel, level + 1);
    return FlatTransmitterBase::printToStream(stream, level);
}

const IIeee80211Mode *Ieee80211TransmitterBase::computeTransmissionMode(const Packet *packet) const
{
    const IIeee80211Mode *transmissionMode;
    auto modeReq = const_cast<Packet *>(packet)->findTag<Ieee80211ModeReq>();
    auto bitrateReq = const_cast<Packet *>(packet)->findTag<SignalBitrateReq>();
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
    auto channelReq = const_cast<Packet *>(packet)->findTag<Ieee80211ChannelReq>();
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
        const Ieee80211VhtMode *modVht = dynamic_cast<const Ieee80211VhtMode *>(mode);
        if (modVht) {
            double bitRate = modVht->getDataMode()->getNetBitrate().get();
            double normBitRate = (std::round(bitRate / 1e5) * 1e5);
            if (modeSet->findMode(bps(normBitRate), modVht->getDataMode()->getBandwidth()) == nullptr)
                throw cRuntimeError("Invalid mode");
        }
        else {
            if (modeSet->findMode(mode->getDataMode()->getNetBitrate()) == nullptr)
                throw cRuntimeError("Invalid mode");
        }
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

