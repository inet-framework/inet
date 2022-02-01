//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/Ieee8021qSocketPacketProcessor.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qCommand_m.h"

namespace inet {

Define_Module(Ieee8021qSocketPacketProcessor);

void Ieee8021qSocketPacketProcessor::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        socketTable.reference(this, "socketTableModule", true);
}

cGate *Ieee8021qSocketPacketProcessor::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void Ieee8021qSocketPacketProcessor::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    const auto& protocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = protocolTag ? protocolTag->getProtocol() : nullptr;
    const auto& vlanInd = packet->findTag<VlanInd>();
    auto vlanId = vlanInd ? vlanInd->getVlanId() : -1;
    auto sockets = socketTable->findSockets(protocol, vlanId);
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

