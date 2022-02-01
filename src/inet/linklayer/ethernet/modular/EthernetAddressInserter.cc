//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetAddressInserter.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(EthernetAddressInserter);

void EthernetAddressInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        interfaceTable.reference(this, "interfaceTableModule", true);
}

void EthernetAddressInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<EthernetMacAddressFields>();
    auto macAddressReq = packet->getTag<MacAddressReq>();
    auto srcAddress = macAddressReq->getSrcAddress();
    auto interfaceReq = packet->getTag<InterfaceReq>();
    auto networkInterface = interfaceTable->getInterfaceById(interfaceReq->getInterfaceId());
    if (srcAddress.isUnspecified() && networkInterface != nullptr)
        srcAddress = networkInterface->getMacAddress();
    header->setSrc(srcAddress);
    header->setDest(macAddressReq->getDestAddress());
    packet->insertAtFront(header);
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(packetProtocolTag->getFrontOffset() + header->getChunkLength());
}

} // namespace inet

