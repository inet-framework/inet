//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Receiver.h"
#include "inet/common/math/Functions.h"
#include "inet/physicallayer/wireless/common/analogmodel/dimensional/DimensionalReceptionAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/dimensional/DimensionalSnir.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/Reception.h"
#include <memory>

#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211Receiver);

namespace {

std::unique_ptr<Reception> primaryCopy(const IReception *reception)
{
    auto transmission = dynamic_cast<const Ieee80211Transmission *>(reception->getTransmission());
    if (!transmission || transmission->getTxVector().format != Ieee80211TxVector::Format::NON_HT_DUPLICATE)
        return nullptr;
    auto analog = check_and_cast<const DimensionalReceptionAnalogModel *>(reception->getAnalogModel());
    auto center = transmission->getChannel()->getCenterFrequency(MHz(20));
    auto filter = makeShared<Boxcar2DFunction<double, simsec, Hz>>(simsec(reception->getStartTime()), simsec(reception->getEndTime()),
        center - MHz(10), center + MHz(10), 1);
    auto filtered = new DimensionalReceptionAnalogModel(reception->getPreambleDuration(), reception->getHeaderDuration(), reception->getDataDuration(),
        center, MHz(20), analog->getPower()->multiply(filter));
    return std::make_unique<Reception>(reception->getReceiverRadio(), transmission, reception->getStartTime(), reception->getEndTime(),
        reception->getStartPosition(), reception->getEndPosition(), reception->getStartOrientation(), reception->getEndOrientation(), filtered);
}

BandListening matchingListening(const IListening *listening, const IReception *reception)
{
    auto analog = check_and_cast<const INarrowbandSignalAnalogModel *>(reception->getAnalogModel());
    return BandListening(listening->getReceiverRadio(), listening->getStartTime(), listening->getEndTime(), listening->getStartPosition(),
        listening->getEndPosition(), analog->getCenterFrequency(), analog->getBandwidth());
}

} // namespace

Ieee80211Receiver::~Ieee80211Receiver()
{
    delete channel;
}

void Ieee80211Receiver::initialize(int stage)
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

std::ostream& Ieee80211Receiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211Receiver";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(modeSet, printFieldToString(modeSet, level + 1, evFlags))
               << EV_FIELD(band, printFieldToString(band, level + 1, evFlags));
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(channel, printFieldToString(channel, level + 1, evFlags));
    return FlatReceiverBase::printToStream(stream, level);
}

bool Ieee80211Receiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto ieee80211Transmission = dynamic_cast<const Ieee80211Transmission *>(transmission);
    if (channel && channel->hasAllocation()) {
        if (!ieee80211Transmission || !modeSet->containsMode(ieee80211Transmission->getMode()))
            return false;
        auto analog = dynamic_cast<const INarrowbandSignalAnalogModel *>(transmission->getAnalogModel());
        auto sendingChannel = ieee80211Transmission->getChannel();
        return analog && sendingChannel && sendingChannel->hasAllocation()
            && sendingChannel->getPrimaryChannel() == channel->getPrimaryChannel()
            && channel->containsPrimaryAllocation(analog->getCenterFrequency(), analog->getBandwidth())
            && ieee80211Transmission->getMode()->getDataMode()->getNumberOfSpatialStreams() <= listening->getReceiverRadio()->getAntenna()->getNumAntennas();
    }
    return ieee80211Transmission && modeSet->containsMode(ieee80211Transmission->getMode()) && NarrowbandReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool Ieee80211Receiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto ieee80211Transmission = dynamic_cast<const Ieee80211Transmission *>(reception->getTransmission());
    if (channel && channel->hasAllocation()) {
        if (!computeIsReceptionPossible(listening, reception->getTransmission()))
            return false;
        auto copy = primaryCopy(reception);
        auto decoded = copy ? copy.get() : reception;
        auto matching = matchingListening(listening, decoded);
        return getAnalogModel()->computeIsReceptionPossible(&matching, decoded, sensitivity);
    }
    return ieee80211Transmission && modeSet->containsMode(ieee80211Transmission->getMode()) && getAnalogModel()->computeIsReceptionPossible(listening, reception, sensitivity);
}

const IReceptionResult *Ieee80211Receiver::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    auto transmission = check_and_cast<const Ieee80211Transmission *>(reception->getTransmission());
    auto copy = primaryCopy(reception);
    std::unique_ptr<DimensionalSnir> decodedSnir;
    if (copy)
        decodedSnir = std::make_unique<DimensionalSnir>(copy.get(), snir->getNoise());
    auto receptionResult = FlatReceiverBase::computeReceptionResult(listening, reception, interference, decodedSnir ? decodedSnir.get() : snir, decisions);
    auto packet = const_cast<Packet *>(receptionResult->getPacket());
    packet->addTagIfAbsent<Ieee80211ModeInd>()->setTxVector(transmission->getTxVector());
    packet->addTagIfAbsent<Ieee80211ChannelInd>()->setChannel(transmission->getChannel());
    return receptionResult;
}

void Ieee80211Receiver::setModeSet(const Ieee80211ModeSet *modeSet)
{
    this->modeSet = modeSet;
}

bool Ieee80211Receiver::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part,
        const IInterference *interference, const ISnir *snir) const
{
    auto copy = primaryCopy(reception);
    if (!copy)
        return FlatReceiverBase::computeIsReceptionSuccessful(listening, reception, part, interference, snir);
    DimensionalSnir decoded(copy.get(), snir->getNoise());
    auto matching = matchingListening(listening, copy.get());
    return FlatReceiverBase::computeIsReceptionSuccessful(&matching, copy.get(), part, interference, &decoded);
}

W Ieee80211Receiver::computeSignalPower(const IListening *listening, const ISnir *snir, const IInterference *interference) const
{
    auto transmission = dynamic_cast<const Ieee80211Transmission *>(snir->getReception()->getTransmission());
    if (transmission && transmission->getTxVector().format == Ieee80211TxVector::Format::NON_HT_DUPLICATE)
        return check_and_cast<const DimensionalReceptionAnalogModel *>(snir->getReception()->getAnalogModel())->computeMinPower(listening->getStartTime(), listening->getEndTime());
    return FlatReceiverBase::computeSignalPower(listening, snir, interference);
}

void Ieee80211Receiver::setBand(const IIeee80211Band *band)
{
    if (this->band != band) {
        this->band = band;
        if (channel != nullptr)
            setChannel(new Ieee80211Channel(band, channel->getChannelNumber()));
    }
}

void Ieee80211Receiver::setChannel(const Ieee80211Channel *channel)
{
    if (this->channel != channel) {
        delete this->channel;
        this->channel = channel;
        this->band = channel->getBand();
        setCenterFrequency(channel->hasAllocation() ? channel->getCenterFrequency(channel->getAllocationWidth()) : channel->getCenterFrequency());
        if (channel->hasAllocation())
            setBandwidth(channel->getAllocationWidth());
    }
}

void Ieee80211Receiver::setChannelNumber(int channelNumber)
{
    if (channel == nullptr || channelNumber != channel->getChannelNumber())
        setChannel(new Ieee80211Channel(band, channelNumber));
}

bool Ieee80211Receiver::isHtChannelWidthSupported(Hz channelWidth) const
{
    // The receiver listens around the primary-channel center and cannot yet
    // represent simultaneous primary-only and primary/secondary reception.
    return (channelWidth == MHz(20) || (channel && channel->hasAllocation() && channelWidth <= channel->getAllocationWidth())) && channelWidth <= getBandwidth() && modeSet != nullptr &&
            modeSet->getHtSupportedChannelWidths().count(channelWidth) != 0;
}

} // namespace physicallayer

} // namespace inet

