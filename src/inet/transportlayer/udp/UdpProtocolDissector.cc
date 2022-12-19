//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/udp/UdpProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/transportlayer/udp/Udp.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::udp, UdpProtocolDissector);

void UdpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
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
    auto dataProtocol = ProtocolGroup::getUdpProtocolGroup()->findProtocol(header->getDestPort());
    if (dataProtocol == nullptr)
        dataProtocol = ProtocolGroup::getUdpProtocolGroup()->findProtocol(header->getSrcPort());
    callback.dissectPacket(packet, dataProtocol);
    ASSERT(packet->getDataLength() == B(0));
    packet->setFrontOffset(udpPayloadEndOffset);
    packet->setBackOffset(originalTrailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::udp);
}

} // namespace inet

