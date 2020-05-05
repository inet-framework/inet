//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/checksum/EthernetCRC.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/protocol/ethernet/EthernetFragmentFcsInserter.h"
#include "inet/protocol/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(EthernetFragmentFcsInserter);

uint32_t EthernetFragmentFcsInserter::computeComputedFcs(const Packet *packet) const
{
    auto data = packet->peekDataAsBytes();
    auto bytes = data->getBytes();
    auto fragmentTag = packet->getTag<FragmentTag>();
    currentFragmentCompleteFcs = ethernetCRC(bytes.data(), packet->getByteLength(), fragmentTag->getFirstFragment() ? 0 : lastFragmentCompleteFcs);
    if (fragmentTag->getLastFragment())
        return currentFragmentCompleteFcs;
    else
        return ethernetCRC(bytes.data(), packet->getByteLength()) ^ 0xFFFF0000;
}

uint32_t EthernetFragmentFcsInserter::computeFcs(const Packet *packet, FcsMode fcsMode) const
{
    switch (fcsMode) {
        case FCS_DECLARED_CORRECT:
            return computeDeclaredCorrectFcs(packet);
        case FCS_DECLARED_INCORRECT:
            return computeDeclaredIncorrectFcs(packet);
        case FCS_COMPUTED:
            return computeComputedFcs(packet);
        default:
            throw cRuntimeError("Unknown FCS mode: %d", (int)fcsMode);
    }
}

void EthernetFragmentFcsInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<EthernetFragmentFcs>();
    auto fragmentTag = packet->getTag<FragmentTag>();
    auto fcs = computeFcs(packet, fcsMode);
    header->setFcs(fcs);
    header->setFcsMode(fcsMode);
    header->setMCrc(!fragmentTag->getLastFragment());
    packet->insertAtBack(header);
    auto packetProtocolTag = packet->findTag<PacketProtocolTag>();
    if (packetProtocolTag != nullptr)
        packetProtocolTag->setBackOffset(packetProtocolTag->getBackOffset() - header->getChunkLength());
}

void EthernetFragmentFcsInserter::handlePacketProcessed(Packet *packet)
{
    FcsInserterBase::handlePacketProcessed(packet);
    lastFragmentCompleteFcs = currentFragmentCompleteFcs;
}

} // namespace inet

