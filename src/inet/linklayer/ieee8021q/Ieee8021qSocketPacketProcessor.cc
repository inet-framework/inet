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
        socketTable = getModuleFromPar<Ieee8021qSocketTable>(par("socketTableModule"), this);
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

