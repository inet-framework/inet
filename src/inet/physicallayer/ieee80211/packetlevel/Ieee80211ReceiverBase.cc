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

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ReceiverBase.h"

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmissionBase.h"

namespace inet {

namespace physicallayer {

Ieee80211ReceiverBase::Ieee80211ReceiverBase() :
    modeSet(nullptr),
    band(nullptr),
    channel(nullptr)
{
}

Ieee80211ReceiverBase::~Ieee80211ReceiverBase()
{
    delete channel;
}

void Ieee80211ReceiverBase::initialize(int stage)
{
    FlatReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *opMode = par("opMode");
        setModeSet(*opMode ? Ieee80211ModeSet::getModeSet(opMode) : nullptr);
        const char *bandName = par("bandName");
        setBand(*bandName != '\0' ? Ieee80211CompliantBands::getBand(bandName) : nullptr);
        int channelNumber = par("channelNumber");
        if (channelNumber != -1)
            setChannelNumber(channelNumber);
    }
}

std::ostream& Ieee80211ReceiverBase::printToStream(std::ostream& stream, int level) const
{
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", modeSet = " << printObjectToString(modeSet, level - 1)
               << ", band = " << printObjectToString(band, level - 1);
    if (level >= PRINT_LEVEL_INFO)
        stream << ", channel = " << printObjectToString(channel, level - 1);
    return FlatReceiverBase::printToStream(stream, level);
}

void Ieee80211ReceiverBase::setModeSet(const Ieee80211ModeSet *modeSet)
{
    this->modeSet = modeSet;
}

void Ieee80211ReceiverBase::setBand(const IIeee80211Band *band)
{
    if (this->band != band) {
        this->band = band;
        if (channel != nullptr)
            setChannel(new Ieee80211Channel(band, channel->getChannelNumber()));
    }
}

void Ieee80211ReceiverBase::setChannel(const Ieee80211Channel *channel)
{
    if (this->channel != channel) {
        delete this->channel;
        this->channel = channel;
        setCarrierFrequency(channel->getCenterFrequency());
    }
}

void Ieee80211ReceiverBase::setChannelNumber(int channelNumber)
{
    if (channel == nullptr || channelNumber != channel->getChannelNumber())
        setChannel(new Ieee80211Channel(band, channelNumber));
}

ReceptionIndication *Ieee80211ReceiverBase::createReceptionIndication() const
{
    return new Ieee80211ReceptionIndication();
}

const ReceptionIndication *Ieee80211ReceiverBase::computeReceptionIndication(const ISNIR *snir) const
{
    Ieee80211ReceptionIndication *indication = check_and_cast<Ieee80211ReceptionIndication *>(const_cast<ReceptionIndication *>(FlatReceiverBase::computeReceptionIndication(snir)));

    const Ieee80211TransmissionBase *transmission = check_and_cast<const Ieee80211TransmissionBase *>(snir->getReception()->getTransmission());
    //FIXME fill indication
    indication->setMode(transmission->getMode());
    indication->setChannel(const_cast<Ieee80211Channel *>(transmission->getChannel()));
    //indication->setSnr();
    //indication->setLossRate();
    //indication->setRecPow();
    //indication->setAirtimeMetric();
    //indication->setTestFrameDuration();
    //indication->setTestFrameError());
    //indication->setTestFrameSize();

    return indication;
}


} // namespace physicallayer

} // namespace inet

