//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetSocketPacketProcessor.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetCommand_m.h"

namespace inet {

Define_Module(EthernetSocketPacketProcessor);

void EthernetSocketPacketProcessor::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        socketTable.reference(this, "socketTableModule", true);
}

cGate *EthernetSocketPacketProcessor::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void EthernetSocketPacketProcessor::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    const auto& protocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = protocolTag ? protocolTag->getProtocol() : nullptr;
    MacAddress localAddress, remoteAddress;
    if (const auto& macAddressInd = packet->findTag<MacAddressInd>()) {
        localAddress = macAddressInd->getDestAddress();
        remoteAddress = macAddressInd->getSrcAddress();
    }
    auto sockets = socketTable->findSockets(localAddress, remoteAddress, protocol);
    bool steal = false;
    for (auto socket : sockets) {
        auto packetCopy = packet->dup();
        packetCopy->setKind(SOCKET_I_DATA);
        packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(socket->socketId);
        EV_INFO << "Passing up packet to socket" << EV_FIELD(socket) << EV_FIELD(packet) << EV_ENDL;
        pushOrSendPacket(packetCopy, outputGate, consumer);
        steal |= socket->steal;
    }
    handlePacketProcessed(packet);
    if (steal)
        delete packet;
    else
        pushOrSendPacket(packet, outputGate, consumer);
    updateDisplayString();
}

} // namespace inet

