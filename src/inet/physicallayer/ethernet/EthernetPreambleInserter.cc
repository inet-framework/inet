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
#include "inet/physicallayer/ethernet/EthernetPhyHeader_m.h"
#include "inet/physicallayer/ethernet/EthernetPreambleInserter.h"

namespace inet {

namespace physicallayer {

Define_Module(EthernetPreambleInserter);

void EthernetPreambleInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<EthernetPhyHeader>();
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetPhy);
}

void EthernetPreambleInserter::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
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

