//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/generic/GenericRadio.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/physicallayer/wireless/generic/GenericPhyHeader_m.h"
#include "inet/physicallayer/wireless/generic/GenericTransmitter.h"

namespace inet {

namespace physicallayer {

Define_Module(GenericRadio);

GenericRadio::GenericRadio() :
    Radio()
{
}

void GenericRadio::encapsulate(Packet *packet) const
{
    auto idealTransmitter = check_and_cast<const GenericTransmitter *>(transmitter);
    auto phyHeader = makeShared<GenericPhyHeader>();
    phyHeader->setChunkLength(idealTransmitter->getHeaderLength());
    phyHeader->setPayloadProtocol(packet->getTag<PacketProtocolTag>()->getProtocol());
    packet->insertAtFront(phyHeader);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::unitDisk);
}

void GenericRadio::decapsulate(Packet *packet) const
{
    const auto& phyHeader = packet->popAtFront<GenericPhyHeader>();
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(phyHeader->getPayloadProtocol());
}

} // namespace physicallayer

} // namespace inet

