//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Receiver.h"

#include "inet/common/math/Functions.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INarrowbandSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ErpOfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211Receiver);

Ieee80211Receiver::~Ieee80211Receiver()
{
    delete channel;
}

void Ieee80211Receiver::initialize(int stage)
{
    FlatReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        htCca20Sensitivity = mW(math::dBmW2mW(par("htCca20Sensitivity")));
        htCca40Sensitivity = mW(math::dBmW2mW(par("htCca40Sensitivity")));
        htCcaEnergyDetection = mW(math::dBmW2mW(par("htCcaEnergyDetection")));
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
    return ieee80211Transmission && modeSet->containsMode(ieee80211Transmission->getMode()) && NarrowbandReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool Ieee80211Receiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto ieee80211Transmission = dynamic_cast<const Ieee80211Transmission *>(reception->getTransmission());
    return ieee80211Transmission && modeSet->containsMode(ieee80211Transmission->getMode()) && getAnalogModel()->computeIsReceptionPossible(listening, reception, sensitivity);
}

const IListeningDecision *Ieee80211Receiver::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    if (isHtCcaOperation() && dynamic_cast<const BandListening *>(listening) != nullptr &&
            dynamic_cast<const BandListening *>(listening)->getBandwidth() == MHz(20))
        return new ListeningDecision(listening, computeHtCcaBusy(listening, interference));
    return FlatReceiverBase::computeListeningDecision(listening, interference);
}

bool Ieee80211Receiver::isHtCcaOperation() const
{
    return modeSet != nullptr && !strcmp(modeSet->getName(), "n(mixed-2.4Ghz)") &&
            channel != nullptr && (bandwidth == MHz(20) ||
            (bandwidth == MHz(40) && channel->getSecondaryChannelOffset() != IEEE80211_SECONDARY_CHANNEL_NONE));
}

static bool isBandOverlapping(const BandListening *listening, const INarrowbandSignalAnalogModel *signal)
{
    auto listeningMin = listening->getCenterFrequency() - listening->getBandwidth() / 2;
    auto listeningMax = listening->getCenterFrequency() + listening->getBandwidth() / 2;
    auto signalMin = signal->getCenterFrequency() - signal->getBandwidth() / 2;
    auto signalMax = signal->getCenterFrequency() + signal->getBandwidth() / 2;
    return signalMin <= listeningMax && signalMax >= listeningMin;
}

static bool isPrimaryChannel(const Ieee80211Channel *channel, const BandListening *listening)
{
    return channel != nullptr && listening->getCenterFrequency() == channel->getCenterFrequency();
}

static bool isSecondaryChannel(const Ieee80211Channel *channel, const BandListening *listening)
{
    return channel != nullptr && channel->getSecondaryChannelOffset() != IEEE80211_SECONDARY_CHANNEL_NONE &&
            listening->getCenterFrequency() == channel->getSecondaryCenterFrequency();
}

static bool isHt40SignalOccupyingChannel(const Ieee80211Channel *channel, const INarrowbandSignalAnalogModel *signal)
{
    return channel != nullptr && signal->getBandwidth() == MHz(40) &&
            signal->getCenterFrequency() == channel->getBondedCenterFrequency();
}

bool Ieee80211Receiver::computeHtCcaBusy(const IListening *listening, const IInterference *interference) const
{
    const auto *bandListening = check_and_cast<const BandListening *>(listening);
    const auto *mediumAnalogModel = listening->getReceiverRadio()->getMedium()->getAnalogModel();
    const INoise *noise = mediumAnalogModel->computeNoise(listening, interference);
    bool busy = noise->computeMaxPower(listening->getStartTime(), listening->getEndTime()) >= htCcaEnergyDetection;
    delete noise;
    if (busy)
        return true;

    const bool primary = isPrimaryChannel(channel, bandListening);
    const bool secondary = isSecondaryChannel(channel, bandListening);
    if (!primary && !secondary)
        return false;

    for (auto reception : *interference->getInterferingReceptions()) {
        const auto *transmission = dynamic_cast<const Ieee80211Transmission *>(reception->getTransmission());
        const auto *signal = dynamic_cast<const INarrowbandSignalAnalogModel *>(reception->getAnalogModel());
        if (transmission == nullptr || transmission->getMode() == nullptr || signal == nullptr ||
                !modeSet->containsMode(transmission->getMode()) || !isBandOverlapping(bandListening, signal))
            continue;

        const W signalPower = signal->computeMinPower(reception->getStartTime(), reception->getEndTime());
        const IIeee80211Mode *mode = transmission->getMode();
        bool isHt = dynamic_cast<const Ieee80211HtMode *>(mode) != nullptr;
        bool isOfdmOrErp = (dynamic_cast<const Ieee80211OfdmMode *>(mode) != nullptr) ||
                           (dynamic_cast<const Ieee80211ErpOfdmMode *>(mode) != nullptr);
        const Hz signalBandwidth = mode->getDataMode()->getBandwidth();
        if (isHt) {
            // IEEE Std 802.11-2024, 19.3.19.6.4 and 19.3.19.6.5:
            // a 20 MHz HT signal is detected on the primary channel at
            // -82 dBm; a 40 MHz HT signal is detected on each occupied
            // channel at -79 dBm. The comparison uses the received signal
            // level over the PPDU bandwidth, not the power apportioned to a
            // 20 MHz slice by the analog interference model.
            if (signalBandwidth == MHz(40) && isHtCcaOperation() &&
                    isHt40SignalOccupyingChannel(channel, signal) && signalPower >= htCca40Sensitivity)
                return true;
            if (signalBandwidth == MHz(20) && primary && signalPower >= htCca20Sensitivity)
                return true;
        }
        else if (primary && isOfdmOrErp && signalPower >= htCca20Sensitivity) {
            // Clause 19.3.19.6.3 delegates non-HT CCA to the OFDM/ERP-OFDM
            // preamble-detection requirement, which uses the 20 MHz
            // sensitivity threshold.
            return true;
        }
    }
    return false;
}

const IReceptionResult *Ieee80211Receiver::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    auto transmission = check_and_cast<const Ieee80211Transmission *>(reception->getTransmission());
    auto receptionResult = FlatReceiverBase::computeReceptionResult(listening, reception, interference, snir, decisions);
    auto packet = const_cast<Packet *>(receptionResult->getPacket());
    packet->addTagIfAbsent<Ieee80211ModeInd>()->setMode(transmission->getMode());
    packet->addTagIfAbsent<Ieee80211ChannelInd>()->setChannel(transmission->getChannel());
    return receptionResult;
}

void Ieee80211Receiver::setModeSet(const Ieee80211ModeSet *modeSet)
{
    this->modeSet = modeSet;
}

void Ieee80211Receiver::setBand(const IIeee80211Band *band)
{
    if (this->band != band) {
        if (channel != nullptr)
            setChannel(new Ieee80211Channel(band, channel->getChannelNumber(), channel->getSecondaryChannelOffset()));
        else
            this->band = band;
    }
}

void Ieee80211Receiver::setChannel(const Ieee80211Channel *channel)
{
    if (this->channel != channel) {
        // IEEE Std 802.11-2024, 19.3.15.4 and 19.3.19.6.5: HT40 listening
        // spans both 20 MHz channels around their bonded center.
        auto centerFrequency = channel->getSecondaryChannelOffset() == IEEE80211_SECONDARY_CHANNEL_NONE ?
                channel->getCenterFrequency() : channel->getBondedCenterFrequency();
        delete this->channel;
        this->channel = channel;
        this->band = channel->getBand();
        setCenterFrequency(centerFrequency);
    }
}

void Ieee80211Receiver::setChannelNumber(int channelNumber)
{
    if (channel == nullptr || channelNumber != channel->getChannelNumber())
        setChannel(new Ieee80211Channel(band, channelNumber, channel == nullptr ?
                IEEE80211_SECONDARY_CHANNEL_NONE : channel->getSecondaryChannelOffset()));
}

} // namespace physicallayer

} // namespace inet

