//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskRadio.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskPhyHeader_m.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskTransmitter.h"

namespace inet {

namespace physicallayer {

Define_Module(UnitDiskRadio);

UnitDiskRadio::UnitDiskRadio() :
    Radio()
{
}

void UnitDiskRadio::encapsulate(Packet *packet) const
{
    auto idealTransmitter = check_and_cast<const UnitDiskTransmitter *>(transmitter);
    auto phyHeader = makeShared<UnitDiskPhyHeader>();
    phyHeader->setChunkLength(idealTransmitter->getHeaderLength());
    phyHeader->setPayloadProtocol(packet->getTag<PacketProtocolTag>()->getProtocol());
    packet->insertAtFront(phyHeader);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::unitDisk);
}

void UnitDiskRadio::decapsulate(Packet *packet) const
{
    const auto& phyHeader = packet->popAtFront<UnitDiskPhyHeader>();
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(phyHeader->getPayloadProtocol());
}

} // namespace physicallayer

} // namespace inet

