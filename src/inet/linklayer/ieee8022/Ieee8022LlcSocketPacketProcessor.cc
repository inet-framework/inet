//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8022/Ieee8022LlcSocketPacketProcessor.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommand_m.h"

namespace inet {

Define_Module(Ieee8022LlcSocketPacketProcessor);

void Ieee8022LlcSocketPacketProcessor::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        socketTable.reference(this, "socketTableModule", true);
}

cGate *Ieee8022LlcSocketPacketProcessor::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void Ieee8022LlcSocketPacketProcessor::pushPacket(Packet *packet, cGate *gate)
{
    const auto& sap = packet->findTag<Ieee802SapInd>();
    auto sockets = socketTable->findSockets(sap->getDsap(), sap->getSsap());
    for (auto socket : sockets) {
        auto packetCopy = packet->dup();
        packetCopy->setKind(SOCKET_I_DATA);
        packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(socket->socketId);
        EV_INFO << "Passing up packet to socket" << EV_FIELD(socket) << EV_FIELD(packet) << EV_ENDL;
        send(packetCopy, "upperLayerOut");
    }

    // TODO mark packet when sent to any socket
    send(packet, "out");
}

} // namespace inet

