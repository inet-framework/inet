//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"

#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211DsssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211DsssOfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ErpOfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211FhssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HrDsssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211IrMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211VhtMode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Receiver.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmitter.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211Radio);

simsignal_t Ieee80211Radio::radioChannelChangedSignal = cComponent::registerSignal("radioChannelChanged");
simsignal_t IIeee80211CcaProvider::ccaStateChangedSignal = cComponent::registerSignal("ccaStateChanged");

Ieee80211Radio::Ieee80211Radio() :
    FlatRadioBase(),
    ccaSnapshot(std::make_unique<Ieee80211CcaSnapshot>())
{
}

bool Ieee80211Radio::computeIsBandBusy(Hz centerFrequency) const
{
    const simtime_t now = simTime();
    const Coord& position = antenna->getMobility()->getCurrentPosition();
    BandListening listening(this, now, now + SimTime::fromRaw(1), position, position,
            centerFrequency, MHz(20));
    const IListeningDecision *decision = medium->listenOnMedium(this, &listening);
    bool busy = decision->isListeningPossible();
    delete decision;
    return busy;
}

void Ieee80211Radio::updateCcaState()
{
    auto ieee80211Receiver = dynamic_cast<const Ieee80211Receiver *>(receiver);
    auto channel = ieee80211Receiver == nullptr ? nullptr : ieee80211Receiver->getChannel();
    bool ht40Configured = channel != nullptr &&
            channel->getSecondaryChannelOffset() != IEEE80211_SECONDARY_CHANNEL_NONE &&
            modeSet != nullptr && !strcmp(modeSet->getName(), "n(mixed-2.4Ghz)") &&
            ieee80211Receiver->getBandwidth() == MHz(40);
    bool ht40 = ht40Configured && isReceiverMode(radioMode);
    bool primaryBusy = false;
    bool secondaryBusy = false;
    if (ht40) {
        // IEEE Std 802.11-2024, 8.3.5.12/Table 8-5 and 19.3.19.6.5:
        // preserve {primary}, {secondary}, and {primary,secondary} CCA state.
        primaryBusy = computeIsBandBusy(channel->getCenterFrequency());
        secondaryBusy = computeIsBandBusy(channel->getSecondaryCenterFrequency());
    }
    if (ccaSnapshot->isHt40() != ht40 || ccaSnapshot->isPrimaryBusy() != primaryBusy ||
            ccaSnapshot->isSecondaryBusy() != secondaryBusy) {
        ccaSnapshot = std::make_unique<Ieee80211CcaSnapshot>(ht40, primaryBusy, secondaryBusy);
        emit(IIeee80211CcaProvider::ccaStateChangedSignal, ccaSnapshot.get());
    }
}

void Ieee80211Radio::updateTransceiverState()
{
    FlatRadioBase::updateTransceiverState();
    updateCcaState();
}

void Ieee80211Radio::initialize(int stage)
{
    FlatRadioBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *fcsModeString = par("fcsMode");
        fcsMode = parseFcsMode(fcsModeString, true);
        opMode = par("opMode").stringValue();
    }
    if (stage == INITSTAGE_PHYSICAL_LAYER) {
        const char *bandName = par("bandName");
        setBand(*bandName ? Ieee80211CompliantBands::getBand(bandName) : nullptr);
        setModeSet(*opMode.c_str() ? Ieee80211ModeSet::getModeSet(opMode.c_str()) : nullptr);
        htSecondaryChannelOffset = Ieee80211Channel::parseSecondaryChannelOffset(par("htSecondaryChannelOffset"));
        Ieee80211Receiver *ieee80211Receiver = const_cast<Ieee80211Receiver *>(check_and_cast<const Ieee80211Receiver *>(receiver));
        Ieee80211Transmitter *ieee80211Transmitter = const_cast<Ieee80211Transmitter *>(check_and_cast<const Ieee80211Transmitter *>(transmitter));
        Hz radioBw = !std::isnan(par("bandwidth").doubleValue()) ? Hz(par("bandwidth").doubleValue()) : ieee80211Receiver->getBandwidth();
        if (radioBw == MHz(40)) {
            ieee80211Receiver->setBandwidth(radioBw);
            ieee80211Transmitter->setBandwidth(radioBw);
        }
        if (modeSet != nullptr && !strcmp(modeSet->getName(), "n(mixed-2.4Ghz)") &&
                radioBw == MHz(40) &&
                htSecondaryChannelOffset == IEEE80211_SECONDARY_CHANNEL_NONE)
            throw cRuntimeError("HT 40 MHz operation requires a secondary channel offset of above or below");
        if (htSecondaryChannelOffset != IEEE80211_SECONDARY_CHANNEL_NONE) {
            // IEEE Std 802.11-2024, 19.2.3 and 19.3.15.4: the secondary
            // channel is an HT40-only operating-channel property.
            if (modeSet == nullptr || strcmp(modeSet->getName(), "n(mixed-2.4Ghz)") ||
                    radioBw != MHz(40))
                throw cRuntimeError("htSecondaryChannelOffset above/below requires HT 40 MHz operation");
        }
        int channelNumber = par("channelNumber");
        if (channelNumber != -1)
            setChannelNumber(channelNumber);
    }
}

void Ieee80211Radio::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE) {
        Ieee80211ConfigureRadioCommand *configureCommand = dynamic_cast<Ieee80211ConfigureRadioCommand *>(message->getControlInfo());
        if (configureCommand != nullptr) {
            Ieee80211Receiver *ieee80211Receiver = const_cast<Ieee80211Receiver *>(check_and_cast<const Ieee80211Receiver *>(receiver));
            const Ieee80211Channel *currentChannel = ieee80211Receiver->getChannel();
            const char *requestedOpMode = configureCommand->getOpMode();
            std::string targetOpMode = *requestedOpMode ? requestedOpMode : this->opMode;
            const IIeee80211Band *bandParam = configureCommand->getBand();
            const IIeee80211Band *targetBand = bandParam != nullptr ? bandParam : this->band;
            const Ieee80211ModeSet *modeSetParam = configureCommand->getModeSet();
            const Ieee80211ModeSet *targetModeSet = modeSetParam != nullptr ? modeSetParam :
                    *targetOpMode.c_str() ? Ieee80211ModeSet::getModeSet(targetOpMode.c_str()) : this->modeSet;
            const Ieee80211Channel *channel = configureCommand->getChannel();
            int newChannelNumber = configureCommand->getChannelNumber();
            int targetChannelNumber = channel != nullptr ? channel->getChannelNumber() :
                    newChannelNumber != -1 ? newChannelNumber :
                    currentChannel != nullptr ? currentChannel->getChannelNumber() : -1;
            auto targetSecondaryChannelOffset = channel != nullptr ? channel->getSecondaryChannelOffset() :
                    htSecondaryChannelOffset;
            if (targetChannelNumber != -1 && (targetBand == nullptr || targetChannelNumber < 0 || targetChannelNumber >= targetBand->getNumChannels()))
                throw cRuntimeError("Invalid target 802.11 channel number %d", targetChannelNumber);
            if (targetChannelNumber != -1)
                (void)Ieee80211Channel(targetBand, targetChannelNumber, targetSecondaryChannelOffset).getCenterFrequency();

            Hz newBandwidth = configureCommand->getBandwidth();
            Hz targetBandwidth = std::isnan(newBandwidth.get()) ? ieee80211Receiver->getBandwidth() : newBandwidth;
            bps newBitrate = configureCommand->getBitrate();
            const IIeee80211Mode *mode = configureCommand->getMode();
            const IIeee80211Mode *resolvedMode = mode;
            if (resolvedMode == nullptr && targetModeSet != nullptr && !std::isnan(newBitrate.get())) {
                if (!std::isnan(newBandwidth.get()))
                    resolvedMode = targetModeSet->getMode(newBitrate, newBandwidth);
                else
                    resolvedMode = targetModeSet->getMode(newBitrate);
            }
            if (targetModeSet != nullptr && !strcmp(targetModeSet->getName(), "n(mixed-2.4Ghz)") &&
                    ((targetBandwidth == MHz(40)) ||
                     (resolvedMode != nullptr && dynamic_cast<const Ieee80211HtMode *>(resolvedMode) != nullptr &&
                      resolvedMode->getDataMode()->getBandwidth() == MHz(40))) &&
                    targetSecondaryChannelOffset == IEEE80211_SECONDARY_CHANNEL_NONE)
                throw cRuntimeError("HT 40 MHz operation requires a secondary channel offset of above or below");

            bool publishModeSet = targetModeSet != this->modeSet || targetBand != this->band ||
                    !std::isnan(newBandwidth.get()) || !std::isnan(newBitrate.get()) || *requestedOpMode;

            if (targetChannelNumber != -1 &&
                    (currentChannel == nullptr || targetBand != this->band || targetChannelNumber != currentChannel->getChannelNumber() ||
                     targetSecondaryChannelOffset != currentChannel->getSecondaryChannelOffset()))
                setChannel(new Ieee80211Channel(targetBand, targetChannelNumber, targetSecondaryChannelOffset));
            else if (targetBand != this->band)
                setBand(targetBand);
            this->opMode = targetOpMode;
            if (publishModeSet && targetModeSet != nullptr)
                setModeSet(targetModeSet);
            if (resolvedMode != nullptr)
                setMode(resolvedMode);
        }
    }
    FlatRadioBase::handleUpperCommand(message);
}

void Ieee80211Radio::setModeSet(const Ieee80211ModeSet *modeSet)
{
    this->modeSet = modeSet;
    Ieee80211Transmitter *ieee80211Transmitter = const_cast<Ieee80211Transmitter *>(check_and_cast<const Ieee80211Transmitter *>(transmitter));
    Ieee80211Receiver *ieee80211Receiver = const_cast<Ieee80211Receiver *>(check_and_cast<const Ieee80211Receiver *>(receiver));
    ieee80211Transmitter->setModeSet(modeSet);
    ieee80211Receiver->setModeSet(modeSet);
    EV << "Changing radio mode set to " << modeSet << endl;
    receptionTimer = nullptr;
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setMode(const IIeee80211Mode *mode)
{
    Ieee80211Transmitter *ieee80211Transmitter = const_cast<Ieee80211Transmitter *>(check_and_cast<const Ieee80211Transmitter *>(transmitter));
    ieee80211Transmitter->setMode(mode);
    EV << "Changing radio mode to " << mode << endl;
    receptionTimer = nullptr;
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setBand(const IIeee80211Band *band)
{
    this->band = band;
    Ieee80211Transmitter *ieee80211Transmitter = const_cast<Ieee80211Transmitter *>(check_and_cast<const Ieee80211Transmitter *>(transmitter));
    Ieee80211Receiver *ieee80211Receiver = const_cast<Ieee80211Receiver *>(check_and_cast<const Ieee80211Receiver *>(receiver));
    ieee80211Transmitter->setBand(band);
    ieee80211Receiver->setBand(band);
    EV << "Changing radio band to " << band << endl;
    receptionTimer = nullptr;
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setChannel(const Ieee80211Channel *channel)
{
    Ieee80211Transmitter *ieee80211Transmitter = const_cast<Ieee80211Transmitter *>(check_and_cast<const Ieee80211Transmitter *>(transmitter));
    Ieee80211Receiver *ieee80211Receiver = const_cast<Ieee80211Receiver *>(check_and_cast<const Ieee80211Receiver *>(receiver));
    ieee80211Transmitter->setChannel(channel);
    ieee80211Receiver->setChannel(new Ieee80211Channel(channel->getBand(), channel->getChannelNumber(), channel->getSecondaryChannelOffset()));
    band = channel->getBand();
    htSecondaryChannelOffset = channel->getSecondaryChannelOffset();
    EV << "Changing radio channel to " << channel->getChannelNumber() << endl;
    receptionTimer = nullptr;
    emit(radioChannelChangedSignal, channel->getChannelNumber());
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setChannelNumber(int newChannelNumber)
{
    Ieee80211Transmitter *ieee80211Transmitter = const_cast<Ieee80211Transmitter *>(check_and_cast<const Ieee80211Transmitter *>(transmitter));
    Ieee80211Receiver *ieee80211Receiver = const_cast<Ieee80211Receiver *>(check_and_cast<const Ieee80211Receiver *>(receiver));
    ieee80211Transmitter->setChannel(new Ieee80211Channel(band, newChannelNumber, htSecondaryChannelOffset));
    ieee80211Receiver->setChannel(new Ieee80211Channel(band, newChannelNumber, htSecondaryChannelOffset));
    EV << "Changing radio channel to " << newChannelNumber << ".\n";
    receptionTimer = nullptr;
    emit(radioChannelChangedSignal, newChannelNumber);
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::insertFcs(const Ptr<Ieee80211PhyHeader>& phyHeader) const
{
    if (auto header = dynamic_cast<Ieee80211FhssPhyHeader *>(phyHeader.get())) {
        header->setFcsMode(fcsMode);
        switch (fcsMode) {
            case FCS_COMPUTED:
                header->setFcs(0); // TODO calculate FCS
                break;
            case FCS_DECLARED_CORRECT:
                header->setFcs(0xC00D);
                break;
            case FCS_DECLARED_INCORRECT:
                header->setFcs(0xBAAD);
                break;
            default:
                throw cRuntimeError("Invalid FCS mode: %i", (int)fcsMode);
        }
    }
    else if (auto header = dynamic_cast<Ieee80211IrPhyHeader *>(phyHeader.get())) {
        header->setFcsMode(fcsMode);
        switch (fcsMode) {
            case FCS_COMPUTED:
                header->setFcs(0); // TODO calculate FCS
                break;
            case FCS_DECLARED_CORRECT:
                header->setFcs(0xC00D);
                break;
            case FCS_DECLARED_INCORRECT:
                header->setFcs(0xBAAD);
                break;
            default:
                throw cRuntimeError("Invalid FCS mode: %i", (int)fcsMode);
        }
    }
    else if (auto header = dynamic_cast<Ieee80211DsssPhyHeader *>(phyHeader.get())) {
        header->setFcsMode(fcsMode);
        switch (fcsMode) {
            case FCS_COMPUTED:
                header->setFcs(0); // TODO calculate FCS
                break;
            case FCS_DECLARED_CORRECT:
                header->setFcs(0xC00D);
                break;
            case FCS_DECLARED_INCORRECT:
                header->setFcs(0xBAAD);
                break;
            default:
                throw cRuntimeError("Invalid FCS mode: %i", (int)fcsMode);
        }
    }
}

bool Ieee80211Radio::verifyFcs(const Ptr<const Ieee80211PhyHeader>& phyHeader) const
{
    if (auto header = dynamicPtrCast<const Ieee80211FhssPhyHeader>(phyHeader)) {
        switch (header->getFcsMode()) {
            case FCS_COMPUTED:
                return true; // TODO calculate and check FCS
            case FCS_DECLARED_CORRECT:
                return true;
            case FCS_DECLARED_INCORRECT:
                return false;
            default:
                throw cRuntimeError("Invalid FCS mode: %i", (int)fcsMode);
        }
    }
    else if (auto header = dynamicPtrCast<const Ieee80211IrPhyHeader>(phyHeader)) {
        switch (header->getFcsMode()) {
            case FCS_COMPUTED:
                return true; // TODO calculate and check FCS
            case FCS_DECLARED_CORRECT:
                return true;
            case FCS_DECLARED_INCORRECT:
                return false;
            default:
                throw cRuntimeError("Invalid FCS mode: %i", (int)fcsMode);
        }
    }
    else if (auto header = dynamicPtrCast<const Ieee80211DsssPhyHeader>(phyHeader)) {
        switch (header->getFcsMode()) {
            case FCS_COMPUTED:
                return true; // TODO calculate and check FCS
            case FCS_DECLARED_CORRECT:
                return true;
            case FCS_DECLARED_INCORRECT:
                return false;
            default:
                throw cRuntimeError("Invalid FCS mode: %i", (int)fcsMode);
        }
    }
    else
        return true;
}

void Ieee80211Radio::encapsulate(Packet *packet) const
{
    auto ieee80211Transmitter = check_and_cast<const Ieee80211Transmitter *>(transmitter);
    auto mode = ieee80211Transmitter->computeTransmissionMode(packet);
    auto phyHeader = mode->getHeaderMode()->createHeader();
    phyHeader->setChunkLength(b(mode->getHeaderMode()->getLength()));
    phyHeader->setLengthField(B(packet->getDataLength()));
    insertFcs(phyHeader);
    packet->insertAtFront(phyHeader);

    auto tailLength = dynamic_cast<const Ieee80211OfdmMode *>(mode) ? b(6) : b(0);
    auto paddingLength = mode->getDataMode()->getPaddingLength(B(phyHeader->getLengthField()));
    if (tailLength + paddingLength != b(0)) {
        const auto& phyTrailer = makeShared<BitCountChunk>(tailLength + paddingLength);
        packet->insertAtBack(phyTrailer);
    }
    const Protocol *protocol = nullptr;
    if (dynamic_cast<Ieee80211FhssPhyHeader *>(phyHeader.get()))
        protocol = &Protocol::ieee80211FhssPhy;
    else if (dynamic_cast<Ieee80211IrPhyHeader *>(phyHeader.get()))
        protocol = &Protocol::ieee80211IrPhy;
    else if (dynamic_cast<Ieee80211DsssPhyHeader *>(phyHeader.get()))
        protocol = &Protocol::ieee80211DsssPhy;
    else if (dynamic_cast<Ieee80211HrDsssPhyHeader *>(phyHeader.get()))
        protocol = &Protocol::ieee80211HrDsssPhy;
    else if (dynamic_cast<Ieee80211OfdmPhyHeader *>(phyHeader.get()))
        protocol = &Protocol::ieee80211OfdmPhy;
    else if (dynamic_cast<Ieee80211ErpOfdmPhyHeader *>(phyHeader.get()))
        protocol = &Protocol::ieee80211ErpOfdmPhy;
    else if (dynamic_cast<Ieee80211HtPhyHeader *>(phyHeader.get()))
        protocol = &Protocol::ieee80211HtPhy;
    else if (dynamic_cast<Ieee80211VhtPhyHeader *>(phyHeader.get()))
        protocol = &Protocol::ieee80211VhtPhy;
    else
        throw cRuntimeError("Invalid IEEE 802.11 PHY header type.");
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(protocol);
}

void Ieee80211Radio::decapsulate(Packet *packet) const
{
    auto mode = packet->getTag<Ieee80211ModeInd>()->getMode();
    const auto& phyHeader = popIeee80211PhyHeaderAtFront(packet, b(-1), Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
    if (phyHeader->isIncorrect() || phyHeader->isIncomplete() || phyHeader->isImproperlyRepresented() || !verifyFcs(phyHeader))
        packet->setBitError(true);
    auto tailLength = dynamic_cast<const Ieee80211OfdmMode *>(mode) ? b(6) : b(0);
    auto paddingLength = mode->getDataMode()->getPaddingLength(B(phyHeader->getLengthField()));
    if (tailLength + paddingLength != b(0))
        packet->popAtBack(tailLength + paddingLength, Chunk::PF_ALLOW_INCORRECT);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
}

const Ptr<const Ieee80211PhyHeader> Ieee80211Radio::popIeee80211PhyHeaderAtFront(Packet *packet, b length, int flags)
{
    int id = packet->getTag<PacketProtocolTag>()->getProtocol()->getId();
    if (id == Protocol::ieee80211FhssPhy.getId())
        return packet->popAtFront<Ieee80211FhssPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211IrPhy.getId())
        return packet->popAtFront<Ieee80211IrPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211DsssPhy.getId())
        return packet->popAtFront<Ieee80211DsssPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211HrDsssPhy.getId())
        return packet->popAtFront<Ieee80211HrDsssPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211OfdmPhy.getId())
        return packet->popAtFront<Ieee80211OfdmPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211ErpOfdmPhy.getId())
        return packet->popAtFront<Ieee80211ErpOfdmPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211HtPhy.getId())
        return packet->popAtFront<Ieee80211HtPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211VhtPhy.getId())
        return packet->popAtFront<Ieee80211VhtPhyHeader>(length, flags);
    else
        throw cRuntimeError("Invalid IEEE 802.11 PHY protocol.");
}

const Ptr<const Ieee80211PhyHeader> Ieee80211Radio::peekIeee80211PhyHeaderAtFront(const Packet *packet, b length, int flags)
{
    int id = packet->getTag<PacketProtocolTag>()->getProtocol()->getId();
    if (id == Protocol::ieee80211FhssPhy.getId())
        return packet->peekAtFront<Ieee80211FhssPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211IrPhy.getId())
        return packet->peekAtFront<Ieee80211IrPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211DsssPhy.getId())
        return packet->peekAtFront<Ieee80211DsssPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211HrDsssPhy.getId())
        return packet->peekAtFront<Ieee80211HrDsssPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211OfdmPhy.getId())
        return packet->peekAtFront<Ieee80211OfdmPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211ErpOfdmPhy.getId())
        return packet->peekAtFront<Ieee80211ErpOfdmPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211HtPhy.getId())
        return packet->peekAtFront<Ieee80211HtPhyHeader>(length, flags);
    else if (id == Protocol::ieee80211VhtPhy.getId())
        return packet->peekAtFront<Ieee80211VhtPhyHeader>(length, flags);
    else
        throw cRuntimeError("Invalid IEEE 802.11 PHY protocol.");
}

} // namespace physicallayer

} // namespace inet

