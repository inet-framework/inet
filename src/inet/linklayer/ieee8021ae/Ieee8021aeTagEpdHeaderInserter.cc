//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/linklayer/ieee8021ae/Ieee8021aeTagEpdHeaderInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/EncryptedChunk.h"
#include "inet/linklayer/ieee8021ae/Ieee8021aeTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021aeTagEpdHeaderInserter);

void Ieee8021aeTagEpdHeaderInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        registerService(Protocol::ethernetMac, inputGate, nullptr);
}

void Ieee8021aeTagEpdHeaderInserter::processPacket(Packet *packet)
{
    // TODO: this code is incomplete
    auto data = packet->removeData();
    data->markImmutable();
    auto encryptedData = makeShared<EncryptedChunk>(data, data->getChunkLength());
    packet->insertData(encryptedData);
    auto header = makeShared<Ieee8021aeTagEpdHeader>();
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    auto protocol = packetProtocolTag->getProtocol();
    if (protocol == &Protocol::ieee8022llc)
        header->setTypeOrLength(packet->getByteLength());
    else
        header->setTypeOrLength(ProtocolGroup::ethertype.findProtocolNumber(protocol));
    packet->insertAtFront(header);
    packetProtocolTag->setProtocol(&Protocol::ieee8021ae);
    packetProtocolTag->setFrontOffset(b(0));
}

} // namespace inet

