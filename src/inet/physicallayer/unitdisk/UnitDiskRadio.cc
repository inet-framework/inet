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
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/physicallayer/unitdisk/UnitDiskPhyHeader_m.h"
#include "inet/physicallayer/unitdisk/UnitDiskRadio.h"
#include "inet/physicallayer/unitdisk/UnitDiskTransmitter.h"

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
    packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::unitDisk);
}

void UnitDiskRadio::decapsulate(Packet *packet) const
{
    const auto& phyHeader = packet->popAtFront<UnitDiskPhyHeader>();
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(phyHeader->getPayloadProtocol());
}

} // namespace physicallayer

} // namespace inet

