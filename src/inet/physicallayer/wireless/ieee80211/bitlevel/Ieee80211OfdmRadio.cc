//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmRadio.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211LayeredOfdmReceiver.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211LayeredOfdmTransmitter.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211OfdmRadio);

Ieee80211OfdmRadio::Ieee80211OfdmRadio() :
    FlatRadioBase()
{
}

void Ieee80211OfdmRadio::encapsulate(Packet *packet) const
{
    auto ofdmTransmitter = check_and_cast<const Ieee80211LayeredOfdmTransmitter *>(transmitter);
    const auto& phyHeader = makeShared<Ieee80211OfdmPhyHeader>();
    auto mode = ofdmTransmitter->getMode(packet);
    phyHeader->setRate(mode->getSignalMode()->getRate());
    phyHeader->setLengthField(B(packet->getDataLength()));
    packet->insertAtFront(phyHeader);
    auto paddingLength = mode->getDataMode()->getPaddingLength(B(phyHeader->getLengthField()));
    const auto& phyTrailer = makeShared<BitCountChunk>(paddingLength + b(6));
    packet->insertAtBack(phyTrailer);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211OfdmPhy);
}

void Ieee80211OfdmRadio::decapsulate(Packet *packet) const
{
    if (!packet->hasBitError()) {
        auto ofdmReceiver = check_and_cast<const Ieee80211LayeredOfdmReceiver *>(receiver);
        const auto& phyHeaderBytes = packet->peekDataAt<BytesChunk>(b(0), B(5), Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
        auto signal = phyHeaderBytes->getByte(0) | (phyHeaderBytes->getByte(1) << 8) | (phyHeaderBytes->getByte(2) << 16);
        const auto& phyHeader = makeShared<Ieee80211OfdmPhyHeader>();
        phyHeader->setRate(signal & 0xF);
        phyHeader->setReserved((signal & 0x10) != 0);
        phyHeader->setLengthField(B((signal >> 5) & 0xFFF));
        phyHeader->setParity((signal & 0x20000) != 0);
        phyHeader->setTail((signal >> 18) & 0x3F);
        phyHeader->setService(phyHeaderBytes->getByte(3) | (phyHeaderBytes->getByte(4) << 8));
        packet->eraseAtFront(B(5));
        auto mode = ofdmReceiver->getMode(packet);
        auto dataLength = B(phyHeader->getLengthField());
        auto paddingLength = mode->getDataMode()->getPaddingLength(dataLength);
        auto trailerLength = std::min(paddingLength + b(6), packet->getDataLength());
        if (trailerLength > b(0))
            packet->popAtBack(trailerLength,
                    Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
        packet->setBitError(packet->hasBitError() || packet->getDataLength() != dataLength);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
    }
}

} // namespace physicallayer

} // namespace inet
