//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wired/ethernet/EthernetFragmentPhyHeaderInserter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"
#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

namespace inet {

namespace physicallayer {

Define_Module(EthernetFragmentPhyHeaderInserter);

void EthernetFragmentPhyHeaderInserter::processPacket(Packet *packet)
{
    const auto& fragmentTag = packet->getTag<FragmentTag>();
    const auto& header = makeShared<EthernetFragmentPhyHeader>();
    header->setPreambleType(fragmentTag->getFirstFragment() ? SMD_Sx : SMD_Cx);
    header->setSmdNumber(smdNumber);
    header->setFragmentNumber(fragmentNumber % 4);
    packet->insertAtFront(header);
    const auto& packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setProtocol(&Protocol::ethernetPhy);
    packetProtocolTag->setFrontOffset(b(0));
    packetProtocolTag->setBackOffset(b(0));
}

void EthernetFragmentPhyHeaderInserter::handlePacketProcessed(Packet *packet)
{
    auto fragmentTag = packet->getTag<FragmentTag>();
    if (!fragmentTag->getFirstFragment())
        fragmentNumber++;
    if (fragmentTag->getLastFragment()) {
        fragmentNumber = 0;
        smdNumber = (smdNumber + 1) % 4;
    }
}

void EthernetFragmentPhyHeaderInserter::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
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

