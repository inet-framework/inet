//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021ae/Ieee8021aeTagEpdHeaderInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/EncryptedChunk.h"
#include "inet/linklayer/ieee8021ae/Ieee8021aeTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021aeTagEpdHeaderInserter);

void Ieee8021aeTagEpdHeaderInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        registerService(Protocol::ethernetMac, inputGate, nullptr);
}

void Ieee8021aeTagEpdHeaderInserter::processPacket(Packet *packet)
{
    // TODO this code is incomplete
    auto data = packet->removeData();
    data->markImmutable();
    auto encryptedData = makeShared<EncryptedChunk>(data, data->getChunkLength());
    packet->insertData(encryptedData);
    auto header = makeShared<Ieee8021aeTagEpdHeader>();
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    auto protocol = packetProtocolTag->getProtocol();
    if (protocol == &Protocol::ieee8022llc)
        header->setTypeOrLength(packet->getByteLength());
    else
        header->setTypeOrLength(ProtocolGroup::getEthertypeProtocolGroup()->findProtocolNumber(protocol));
    packet->insertAtFront(header);
    packetProtocolTag->setProtocol(&Protocol::ieee8021ae);
    packetProtocolTag->setFrontOffset(b(0));
}

} // namespace inet

