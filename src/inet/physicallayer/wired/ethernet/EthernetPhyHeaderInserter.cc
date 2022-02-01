//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wired/ethernet/EthernetPhyHeaderInserter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

namespace inet {

namespace physicallayer {

Define_Module(EthernetPhyHeaderInserter);

void EthernetPhyHeaderInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<EthernetPhyHeader>();
    packet->insertAtFront(header);
    const auto& packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setProtocol(&Protocol::ethernetPhy);
    packetProtocolTag->setFrontOffset(b(0));
    packetProtocolTag->setBackOffset(b(0));
}

void EthernetPhyHeaderInserter::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    checkPacketStreaming(packet);
    startPacketStreaming(packet);
    processPacket(packet);
    pushOrSendPacketProgress(packet, outputGate, consumer, datarate, B(8), b(0), packet->getTransmissionId());
    updateDisplayString();
}

} // namespace physicallayer

} // namespace inet

