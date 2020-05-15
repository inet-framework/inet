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
#include "inet/protocol/ethernet/EthernetSocketPacketProcessor.h"

namespace inet {

Define_Module(EthernetSocketPacketProcessor);

void EthernetSocketPacketProcessor::initialize()
{
    socketTable = getModuleFromPar<EthernetSocketTable>(par("socketTableModule"), this);
}

void EthernetSocketPacketProcessor::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("in"))
        processPacket(check_and_cast<Packet *>(msg));
    else if (msg->arrivedOn("cmdIn"))
        send(msg, "cmdOut");
    else
        throw cRuntimeError("Unknown gate: %s", msg->getArrivalGate()->getFullName());
}

void EthernetSocketPacketProcessor::processPacket(Packet *packet)
{
    bool stealPacket = false;
    auto sockets = socketTable->findSocketsFor(packet);
    for (auto socket : sockets) {
        auto packetCopy = packet->dup();
        packetCopy->setKind(ETHERNET_I_DATA);
        packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(socket->socketId);
        EV_INFO << "Passing up to socket " << socket->socketId << "\n";
        send(packetCopy, "socketOut");
        stealPacket |= socket->vlanId != -1;    //TODO Why?
    }

    // TODO: should the socket configure if it steals packets or not?
    if (stealPacket) {
        delete packet;
        return;
    }

    //TODO mark packet when sent to any socket
    send(packet, "out");
}

} // namespace inet

