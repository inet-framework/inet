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

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ReceiverBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmitterBase.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211DsssMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211DsssOfdmMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ErpOfdmMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211FhssMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HrDsssMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211IrMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211VhtMode.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211Radio);

simsignal_t Ieee80211Radio::radioChannelChangedSignal = cComponent::registerSignal("radioChannelChanged");

Ieee80211Radio::Ieee80211Radio() :
    FlatRadioBase()
{
}

void Ieee80211Radio::initialize(int stage)
{
    FlatRadioBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, true);
    }
    if (stage == INITSTAGE_PHYSICAL_LAYER) {
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
            const char *opMode = configureCommand->getOpMode();
            if (*opMode)
                setModeSet(Ieee80211ModeSet::getModeSet(opMode));
            const Ieee80211ModeSet *modeSet = configureCommand->getModeSet();
            if (modeSet != nullptr)
                setModeSet(modeSet);
            const IIeee80211Mode *mode = configureCommand->getMode();
            if (mode != nullptr)
                setMode(mode);
            const IIeee80211Band *band = configureCommand->getBand();
            if (band != nullptr)
                setBand(band);
            const Ieee80211Channel *channel = configureCommand->getChannel();
            if (channel != nullptr)
                setChannel(channel);
            int newChannelNumber = configureCommand->getChannelNumber();
            if (newChannelNumber != -1)
                setChannelNumber(newChannelNumber);
        }
    }
    FlatRadioBase::handleUpperCommand(message);
}

void Ieee80211Radio::setModeSet(const Ieee80211ModeSet *modeSet)
{
    Ieee80211TransmitterBase *ieee80211Transmitter = const_cast<Ieee80211TransmitterBase *>(check_and_cast<const Ieee80211TransmitterBase *>(transmitter));
    Ieee80211ReceiverBase *ieee80211Receiver = const_cast<Ieee80211ReceiverBase *>(check_and_cast<const Ieee80211ReceiverBase *>(receiver));
    ieee80211Transmitter->setModeSet(modeSet);
    ieee80211Receiver->setModeSet(modeSet);
    EV << "Changing radio mode set to " << modeSet << endl;
    receptionTimer = nullptr;
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setMode(const IIeee80211Mode *mode)
{
    Ieee80211TransmitterBase *ieee80211Transmitter = const_cast<Ieee80211TransmitterBase *>(check_and_cast<const Ieee80211TransmitterBase *>(transmitter));
    ieee80211Transmitter->setMode(mode);
    EV << "Changing radio mode to " << mode << endl;
    receptionTimer = nullptr;
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setBand(const IIeee80211Band *band)
{
    Ieee80211TransmitterBase *ieee80211Transmitter = const_cast<Ieee80211TransmitterBase *>(check_and_cast<const Ieee80211TransmitterBase *>(transmitter));
    Ieee80211ReceiverBase *ieee80211Receiver = const_cast<Ieee80211ReceiverBase *>(check_and_cast<const Ieee80211ReceiverBase *>(receiver));
    ieee80211Transmitter->setBand(band);
    ieee80211Receiver->setBand(band);
    EV << "Changing radio band to " << band << endl;
    receptionTimer = nullptr;
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setChannel(const Ieee80211Channel *channel)
{
    Ieee80211TransmitterBase *ieee80211Transmitter = const_cast<Ieee80211TransmitterBase *>(check_and_cast<const Ieee80211TransmitterBase *>(transmitter));
    Ieee80211ReceiverBase *ieee80211Receiver = const_cast<Ieee80211ReceiverBase *>(check_and_cast<const Ieee80211ReceiverBase *>(receiver));
    ieee80211Transmitter->setChannel(channel);
    ieee80211Receiver->setChannel(channel);
    EV << "Changing radio channel to " << channel->getChannelNumber() << endl;
    receptionTimer = nullptr;
    emit(radioChannelChangedSignal, channel->getChannelNumber());
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::setChannelNumber(int newChannelNumber)
{
    Ieee80211TransmitterBase *ieee80211Transmitter = const_cast<Ieee80211TransmitterBase *>(check_and_cast<const Ieee80211TransmitterBase *>(transmitter));
    Ieee80211ReceiverBase *ieee80211Receiver = const_cast<Ieee80211ReceiverBase *>(check_and_cast<const Ieee80211ReceiverBase *>(receiver));
    ieee80211Transmitter->setChannelNumber(newChannelNumber);
    ieee80211Receiver->setChannelNumber(newChannelNumber);
    EV << "Changing radio channel to " << newChannelNumber << ".\n";
    receptionTimer = nullptr;
    emit(radioChannelChangedSignal, newChannelNumber);
    emit(listeningChangedSignal, 0);
}

void Ieee80211Radio::insertCrc(const Ptr<Ieee80211PhyHeader>& phyHeader) const
{
    if (auto header = dynamic_cast<Ieee80211FhssPhyHeader*>(phyHeader.get())) {
        header->setCrcMode(crcMode);
        switch (crcMode) {
          case CRC_COMPUTED:
            header->setCrc(0);    //TODO calculate CRC
            break;
          case CRC_DECLARED_CORRECT:
              header->setCrc(0xC00D);
              break;
          case CRC_DECLARED_INCORRECT:
              header->setCrc(0xBAAD);
              break;
          default:
              throw cRuntimeError("Invalid CRC mode: %i", (int)crcMode);
        }
    }
    else if (auto header = dynamic_cast<Ieee80211IrPhyHeader*>(phyHeader.get())) {
        header->setCrcMode(crcMode);
        switch (crcMode) {
          case CRC_COMPUTED:
            header->setCrc(0);    //TODO calculate CRC
            break;
          case CRC_DECLARED_CORRECT:
              header->setCrc(0xC00D);
              break;
          case CRC_DECLARED_INCORRECT:
              header->setCrc(0xBAAD);
              break;
          default:
              throw cRuntimeError("Invalid CRC mode: %i", (int)crcMode);
        }
    }
    else if (auto header = dynamic_cast<Ieee80211DsssPhyHeader*>(phyHeader.get())) {
        header->setCrcMode(crcMode);
        switch (crcMode) {
          case CRC_COMPUTED:
            header->setCrc(0);    //TODO calculate CRC
            break;
          case CRC_DECLARED_CORRECT:
              header->setCrc(0xC00D);
              break;
          case CRC_DECLARED_INCORRECT:
              header->setCrc(0xBAAD);
              break;
          default:
              throw cRuntimeError("Invalid CRC mode: %i", (int)crcMode);
        }
    }
}

bool Ieee80211Radio::verifyCrc(const Ptr<const Ieee80211PhyHeader>& phyHeader) const
{
    if (auto header = dynamicPtrCast<const Ieee80211FhssPhyHeader>(phyHeader)) {
        switch (header->getCrcMode()) {
          case CRC_COMPUTED:
            return true;    //TODO calculate and check CRC
          case CRC_DECLARED_CORRECT:
            return true;
          case CRC_DECLARED_INCORRECT:
            return false;
          default:
            throw cRuntimeError("Invalid CRC mode: %i", (int)crcMode);
        }
    }
    else if (auto header = dynamicPtrCast<const Ieee80211IrPhyHeader>(phyHeader)) {
        switch (header->getCrcMode()) {
          case CRC_COMPUTED:
            return true;    //TODO calculate and check CRC
          case CRC_DECLARED_CORRECT:
            return true;
          case CRC_DECLARED_INCORRECT:
            return false;
          default:
            throw cRuntimeError("Invalid CRC mode: %i", (int)crcMode);
        }
    }
    else if (auto header = dynamicPtrCast<const Ieee80211DsssPhyHeader>(phyHeader)) {
        switch (header->getCrcMode()) {
          case CRC_COMPUTED:
            return true;    //TODO calculate and check CRC
          case CRC_DECLARED_CORRECT:
            return true;
          case CRC_DECLARED_INCORRECT:
            return false;
          default:
            throw cRuntimeError("Invalid CRC mode: %i", (int)crcMode);
        }
    }
    else
        return true;
}

void Ieee80211Radio::encapsulate(Packet *packet) const
{
    auto ieee80211Transmitter = check_and_cast<const Ieee80211TransmitterBase *>(transmitter);
    auto mode = ieee80211Transmitter->computeTransmissionMode(packet);
    auto phyHeader = mode->getHeaderMode()->createHeader();
    phyHeader->setChunkLength(b(mode->getHeaderMode()->getLength()));
    phyHeader->setLengthField(B(packet->getTotalLength()));
    insertCrc(phyHeader);
    packet->insertAtFront(phyHeader);

    auto tailLength = dynamic_cast<const Ieee80211OfdmMode *>(mode) ? b(6) : b(0);
    auto paddingLength = mode->getDataMode()->getPaddingLength(B(phyHeader->getLengthField()));
    if (tailLength + paddingLength != b(0)) {
        const auto &phyTrailer = makeShared<BitCountChunk>(tailLength + paddingLength);
        packet->insertAtBack(phyTrailer);
    }
    if (dynamic_cast<Ieee80211FhssPhyHeader*>(phyHeader.get()))
        packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211FhssPhy);
    else if (dynamic_cast<Ieee80211IrPhyHeader*>(phyHeader.get()))
        packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211IrPhy);
    else if (dynamic_cast<Ieee80211DsssPhyHeader*>(phyHeader.get()))
        packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211DsssPhy);
    else if (dynamic_cast<Ieee80211HrDsssPhyHeader*>(phyHeader.get()))
        packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211HrDsssPhy);
    else if (dynamic_cast<Ieee80211OfdmPhyHeader*>(phyHeader.get()))
        packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211OfdmPhy);
    else if (dynamic_cast<Ieee80211ErpOfdmPhyHeader*>(phyHeader.get()))
        packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211ErpOfdmPhy);
    else if (dynamic_cast<Ieee80211HtPhyHeader*>(phyHeader.get()))
        packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211HtPhy);
    else if (dynamic_cast<Ieee80211VhtPhyHeader*>(phyHeader.get()))
        packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211VhtPhy);
    else
        throw cRuntimeError("Invalid IEEE 802.11 PHY header type.");
}

void Ieee80211Radio::decapsulate(Packet *packet) const
{
    auto mode = packet->getTag<Ieee80211ModeInd>()->getMode();
    const auto& phyHeader = popIeee80211PhyHeaderAtFront(packet, b(-1), Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
    if (phyHeader->isIncorrect() || phyHeader->isIncomplete() || phyHeader->isImproperlyRepresented() || !verifyCrc(phyHeader))
        packet->setBitError(true);
    auto tailLength = dynamic_cast<const Ieee80211OfdmMode *>(mode) ? b(6) : b(0);
    auto paddingLength = mode->getDataMode()->getPaddingLength(B(phyHeader->getLengthField()));
    if (tailLength + paddingLength != b(0))
        packet->popAtBack(tailLength + paddingLength, Chunk::PF_ALLOW_INCORRECT);
    packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
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

