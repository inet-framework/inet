//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommand_m.h"
#include "inet/protocol/ethernet/LlcSocketPacketProcessor.h"

namespace inet {

Define_Module(LlcSocketPacketProcessor);

void LlcSocketPacketProcessor::initialize()
{
    socketTable = getModuleFromPar<LlcSocketTable>(par("socketTableModule"), this);
}

void LlcSocketPacketProcessor::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("in"))
        processPacket(check_and_cast<Packet *>(msg));
    else if (msg->arrivedOn("cmdIn"))
        send(msg, "cmdOut");
    else
        throw cRuntimeError("Unknown gate: %s", msg->getArrivalGate()->getFullName());
}

void LlcSocketPacketProcessor::processPacket(Packet *packet)
{
    auto sockets = socketTable->findSocketsFor(packet);
    for (auto socket : sockets) {
        auto packetCopy = packet->dup();
        packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(socket->socketId);
        EV_INFO << "Passing up to socket " << socket->socketId << "\n";
        packetCopy->setKind(IEEE8022_LLC_I_DATA);
        send(packetCopy, "upperLayerOut");
    }

    //TODO mark packet when sent to any socket
    send(packet, "out");
}

} // namespace inet

