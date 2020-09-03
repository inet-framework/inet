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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/ethernet/EthernetFragmentPreambleInserter.h"
#include "inet/physicallayer/ethernet/EthernetPhyHeader_m.h"
#include "inet/protocol/fragmentation/tag/FragmentTag_m.h"

namespace inet {

namespace physicallayer {

Define_Module(EthernetFragmentPreambleInserter);

void EthernetFragmentPreambleInserter::processPacket(Packet *packet)
{
    auto fragmentTag = packet->getTag<FragmentTag>();
    const auto& header = makeShared<EthernetFragmentPhyHeader>();
    header->setPreambleType(fragmentTag->getFirstFragment() ? SMD_Sx : SMD_Cx);
    header->setSmdNumber(smdNumber);
    header->setFragmentNumber(fragmentNumber % 4);
    packet->insertAtFront(header);
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setProtocol(&Protocol::ethernetPhy);
    packetProtocolTag->setFrontOffset(b(0));
    packetProtocolTag->setBackOffset(b(0));
}

void EthernetFragmentPreambleInserter::handlePacketProcessed(Packet *packet)
{
    auto fragmentTag = packet->getTag<FragmentTag>();
    if (!fragmentTag->getFirstFragment())
        fragmentNumber++;
    if (fragmentTag->getLastFragment()) {
        fragmentNumber = 0;
        smdNumber = (smdNumber + 1) % 4;
    }
}

void EthernetFragmentPreambleInserter::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    checkPacketStreaming(packet);
    startPacketStreaming(packet);
    processPacket(packet);
    pushOrSendPacketProgress(packet, outputGate, consumer, datarate, B(8), b(0));
    updateDisplayString();
}

} // namespace physicallayer

} // namespace inet

