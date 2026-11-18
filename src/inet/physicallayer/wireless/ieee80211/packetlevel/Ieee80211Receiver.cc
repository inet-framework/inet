//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Receiver.h"

#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211AmpduReceptionDecision.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211HtSignalField.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/common/packet/chunk/SequenceChunk.h"

namespace inet {

namespace physicallayer {

namespace {

Ptr<const Ieee80211HtPhyHeader> peekHtHeader(const ITransmission *transmission)
{
    return dynamicPtrCast<const Ieee80211HtPhyHeader>(Ieee80211Radio::peekIeee80211PhyHeaderAtFront(
            transmission->getPacket(), b(-1), Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE |
            Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED));
}

} // namespace

Define_Module(Ieee80211Receiver);

Ieee80211Receiver::~Ieee80211Receiver()
{
    delete channel;
}

void Ieee80211Receiver::initialize(int stage)
{
    FlatReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *opMode = par("opMode");
        modeSet = *opMode ? Ieee80211ModeSet::getModeSet(opMode) : nullptr;
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
    return ieee80211Transmission && modeSet->containsMode(ieee80211Transmission->getMode()) && NarrowbandReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool Ieee80211Receiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto ieee80211Transmission = dynamic_cast<const Ieee80211Transmission *>(reception->getTransmission());
    return ieee80211Transmission && modeSet->containsMode(ieee80211Transmission->getMode()) && getAnalogModel()->computeIsReceptionPossible(listening, reception, sensitivity);
}

const IReceptionResult *Ieee80211Receiver::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    auto transmission = check_and_cast<const Ieee80211Transmission *>(reception->getTransmission());
    auto header = peekHtHeader(transmission);
    bool aggregate = header != nullptr && header->getAggregation();
    bool validHeader = header == nullptr || isIeee80211HtSignalValid(*header, transmission->getMode());
    auto receptionResult = aggregate || !validHeader ?
            NarrowbandReceiverBase::computeReceptionResult(listening, reception, interference, snir, decisions) :
            FlatReceiverBase::computeReceptionResult(listening, reception, interference, snir, decisions);
    auto packet = const_cast<Packet *>(receptionResult->getPacket());
    if (aggregate || !validHeader) {
        auto indication = packet->addTag<Ieee80211AmpduInd>();
        if (aggregate) {
            auto medium = reception->getReceiverRadio()->getMedium();
            auto decision = check_and_cast<const Ieee80211AmpduReceptionDecision *>(medium->getReceptionDecision(
                    reception->getReceiverRadio(), listening, transmission, IRadioSignal::SIGNAL_PART_DATA));
            indication->setHeaderSuccessful(decision->isHeaderSuccessful());
            indication->setAggregation(decision->isHeaderSuccessful());
            indication->setContentComplete(decision->isContentComplete());
            if (!decision->isHeaderSuccessful() || !decision->isContentComplete() || decision->getData() == nullptr)
                packet->setBitError(true);
        }
        if (errorModel != nullptr) {
            auto rates = packet->addTagIfAbsent<ErrorRateInd>();
            rates->setPacketErrorRate(NaN);
            rates->setBitErrorRate(NaN);
            rates->setSymbolErrorRate(NaN);
        }
    }
    packet->addTagIfAbsent<Ieee80211ModeInd>()->setMode(transmission->getMode());
    packet->addTagIfAbsent<Ieee80211ChannelInd>()->setChannel(transmission->getChannel());
    return receptionResult;
}

void Ieee80211Receiver::setModeSet(const Ieee80211ModeSet *modeSet)
{
    this->modeSet = modeSet;
}

Packet *Ieee80211Receiver::computeReceivedPacket(const ISnir *snir, bool successful) const
{
    auto transmission = check_and_cast<const Ieee80211Transmission *>(snir->getReception()->getTransmission());
    auto header = peekHtHeader(transmission);
    if (header != nullptr && header->getAggregation()) {
        auto radio = snir->getReception()->getReceiverRadio();
        auto medium = radio->getMedium();
        auto decision = check_and_cast<const Ieee80211AmpduReceptionDecision *>(medium->getReceptionDecision(
                radio, medium->getListening(radio, transmission), transmission, IRadioSignal::SIGNAL_PART_DATA));
        auto received = ReceiverBase::computeReceivedPacket(snir,
                decision->isHeaderSuccessful() && decision->isContentComplete() && decision->getData() != nullptr);
        if (decision->getData() != nullptr) {
            b offset = received->getFrontOffset() + header->getChunkLength();
            b end = offset + b(header->getLengthField());
            std::deque<Ptr<const Chunk>> chunks{received->peekAt(b(0), offset), decision->getData()};
            if (end < received->getTotalLength())
                chunks.push_back(received->peekAt(end, received->getTotalLength() - end));
            auto content = makeShared<SequenceChunk>();
            // Automatic insertion can merge byte chunks and erase their distinct error flags.
            content->setChunks(chunks);
            content->copyTags(*received->peekAll(Chunk::PF_ALLOW_ALL));
            content->markImmutable();
            received->replaceAll(content, Chunk::PF_ALLOW_ALL);
        }
        return received;
    }
    if (header != nullptr && !isIeee80211HtSignalValid(*header, transmission->getMode()))
        return ReceiverBase::computeReceivedPacket(snir, false);
    return FlatReceiverBase::computeReceivedPacket(snir, successful);
}

bool Ieee80211Receiver::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception,
        IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    auto transmission = check_and_cast<const Ieee80211Transmission *>(reception->getTransmission());
    auto header = peekHtHeader(transmission);
    if (header != nullptr && header->getAggregation())
        return reception->getReceiverRadio()->getMedium()->getReceptionDecision(
                reception->getReceiverRadio(), listening, transmission, part)->isReceptionSuccessful();
    if (header != nullptr && part != IRadioSignal::SIGNAL_PART_PREAMBLE) {
        if (!isIeee80211HtSignalValid(*header, transmission->getMode()))
            return false;
        if ((part == IRadioSignal::SIGNAL_PART_DATA || part == IRadioSignal::SIGNAL_PART_WHOLE) &&
                header->getChunkLength() + header->getLengthField() > transmission->getPacket()->getDataLength())
            return false;
    }
    return FlatReceiverBase::computeIsReceptionSuccessful(listening, reception, part, interference, snir);
}

const IReceptionDecision *Ieee80211Receiver::computeReceptionDecision(const IListening *listening,
        const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    auto transmission = check_and_cast<const Ieee80211Transmission *>(reception->getTransmission());
    auto packet = transmission->getPacket();
    auto header = peekHtHeader(transmission);
    if (header == nullptr || !header->getAggregation())
        return FlatReceiverBase::computeReceptionDecision(listening, reception, part, interference, snir);
    auto radio = reception->getReceiverRadio();
    auto medium = radio->getMedium();
    bool possible = computeIsReceptionPossible(listening, reception, part);
    bool attempted = possible && computeIsReceptionAttempted(listening, reception, part, interference);
    bool successful = attempted;
    bool headerSuccessful = false;
    bool contentComplete = false;
    Ptr<const Chunk> data;
    auto prerequisite = [&](IRadioSignal::SignalPart previous) {
        return medium->getReceptionDecision(radio, listening, transmission, previous)->isReceptionSuccessful();
    };
    if (part == IRadioSignal::SIGNAL_PART_PREAMBLE)
        successful = successful && FlatReceiverBase::computeIsReceptionSuccessful(listening, reception, part, interference, snir);
    else if (part == IRadioSignal::SIGNAL_PART_HEADER) {
        successful = successful && prerequisite(IRadioSignal::SIGNAL_PART_PREAMBLE) &&
                isIeee80211HtSignalValid(*header, transmission->getMode()) &&
                FlatReceiverBase::computeIsReceptionSuccessful(listening, reception, part, interference, snir);
        headerSuccessful = successful;
    }
    else if (part == IRadioSignal::SIGNAL_PART_DATA) {
        headerSuccessful = prerequisite(IRadioSignal::SIGNAL_PART_HEADER);
        contentComplete = headerSuccessful && header->getChunkLength() + header->getLengthField() <= packet->getDataLength();
        successful = successful && contentComplete && SnirReceiverBase::computeIsReceptionSuccessful(listening, reception, part, interference, snir);
        if (successful) {
            data = packet->peekDataAt(b(header->getChunkLength()), b(header->getLengthField()), Chunk::PF_ALLOW_ALL);
            if (errorModel != nullptr)
                data = errorModel->computeReceivedData(snir, data);
            successful = data->isCorrect();
        }
    }
    else if (part == IRadioSignal::SIGNAL_PART_WHOLE) {
        auto decision = check_and_cast<const Ieee80211AmpduReceptionDecision *>(medium->getReceptionDecision(
                radio, listening, transmission, IRadioSignal::SIGNAL_PART_DATA));
        successful = successful && decision->isReceptionSuccessful();
        headerSuccessful = decision->isHeaderSuccessful();
        contentComplete = decision->isContentComplete();
        data = decision->getData();
    }
    else
        throw cRuntimeError("Invalid A-MPDU signal part");
    return new Ieee80211AmpduReceptionDecision(reception, part, possible, attempted, successful,
            headerSuccessful, contentComplete, data);
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
        setCenterFrequency(channel->getCenterFrequency());
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
    return channelWidth == MHz(20) && channelWidth <= getBandwidth() && modeSet != nullptr &&
            modeSet->getHtSupportedChannelWidths().count(channelWidth) != 0;
}

} // namespace physicallayer

} // namespace inet

