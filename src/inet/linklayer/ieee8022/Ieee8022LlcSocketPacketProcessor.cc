//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
        socketTable = getModuleFromPar<Ieee8022LlcSocketTable>(par("socketTableModule"), this);
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

    //TODO mark packet when sent to any socket
    send(packet, "out");
}

} // namespace inet

