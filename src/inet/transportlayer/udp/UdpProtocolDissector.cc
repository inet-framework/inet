//
// Copyright (C) 2018 OpenSim Ltd.
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
// @author: Zoltan Bojthe
//

#include "inet/transportlayer/udp/UdpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/transportlayer/udp/Udp.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"


namespace inet {

Register_Protocol_Dissector(&Protocol::udp, UdpProtocolDissector);

void UdpProtocolDissector::dissect(Packet *packet, ICallback& callback) const
{
    auto originalTrailerPopOffset = packet->getBackOffset();
    auto udpHeaderOffset = packet->getFrontOffset();
    auto header = packet->popAtFront<UdpHeader>();
    callback.startProtocolDataUnit(&Protocol::udp);
    bool isCorrectPacket = Udp::isCorrectPacket(packet, header);
    if (!isCorrectPacket)
        callback.markIncorrect();
    callback.visitChunk(header, &Protocol::udp);
    auto udpPayloadEndOffset = udpHeaderOffset + B(header->getTotalLengthField());
    packet->setBackOffset(udpPayloadEndOffset);
    callback.dissectPacket(packet, nullptr);
    ASSERT(packet->getDataLength() == B(0));
    packet->setFrontOffset(udpPayloadEndOffset);
    packet->setBackOffset(originalTrailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::udp);
}

} // namespace inet

