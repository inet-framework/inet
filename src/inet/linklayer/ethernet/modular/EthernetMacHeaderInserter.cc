//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetMacHeaderInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(EthernetMacHeaderInserter);

void EthernetMacHeaderInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        interfaceTable.reference(this, "interfaceTableModule", true);
    else if (stage == INITSTAGE_LINK_LAYER)
        registerService(Protocol::ethernetMac, inputGate, nullptr);
}

void EthernetMacHeaderInserter::handleMessage(cMessage *message)
{
    // the socket command processor above this module serves socket commands, and it exists only with socket support;
    // passed down, a socket command would come back up to the same protocol layer from the node's dispatcher, forever
    if (auto request = dynamic_cast<Request *>(message))
        if (request->findTag<SocketReq>() != nullptr)
            throw cRuntimeError("Received socket command %s, but the protocol layer has no socket support (hasSocketSupport = false)", request->getName());
    PacketFlowBase::handleMessage(message);
}

void EthernetMacHeaderInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<EthernetMacHeader>();
    auto macAddressReq = packet->getTag<MacAddressReq>();
    auto srcAddress = macAddressReq->getSrcAddress();
    auto interfaceReq = packet->getTag<InterfaceReq>();
    auto networkInterface = interfaceTable->getInterfaceById(interfaceReq->getInterfaceId());
    if (srcAddress.isUnspecified() && networkInterface != nullptr)
        srcAddress = networkInterface->getMacAddress();
    header->setSrc(srcAddress);
    header->setDest(macAddressReq->getDestAddress());
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    auto protocol = packetProtocolTag->getProtocol();
    if (protocol == &Protocol::ieee8022llc)
        header->setTypeOrLength(packet->getByteLength());
    else
        header->setTypeOrLength(ProtocolGroup::getEthertypeProtocolGroup()->findProtocolNumber(protocol));
    packet->insertAtFront(header);
    packetProtocolTag->setFrontOffset(packetProtocolTag->getFrontOffset() + header->getChunkLength());
    packet->removeTagIfPresent<DispatchProtocolReq>();
}

} // namespace inet

