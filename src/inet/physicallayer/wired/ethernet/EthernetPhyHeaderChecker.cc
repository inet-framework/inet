//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wired/ethernet/EthernetPhyHeaderChecker.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

namespace inet {

namespace physicallayer {

Define_Module(EthernetPhyHeaderChecker);

void EthernetPhyHeaderChecker::processPacket(Packet *packet)
{
    packet->popAtFront<EthernetPhyHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT + Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
    const auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    packetProtocolTag->setProtocol(&Protocol::ethernetMac);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    const auto& dispatchProtocolReq = packet->addTagIfAbsent<DispatchProtocolReq>();
    dispatchProtocolReq->setProtocol(&Protocol::ethernetMac);
    dispatchProtocolReq->setServicePrimitive(SP_INDICATION);
}

bool EthernetPhyHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<EthernetPhyHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT + Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
    return header->isCorrect() && header->isProperlyRepresented();
}

void EthernetPhyHeaderChecker::dropPacket(Packet *packet)
{
    PacketFilterBase::dropPacket(packet, INCORRECTLY_RECEIVED);
}

} // namespace physicallayer

} // namespace inet

