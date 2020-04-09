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

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/protocol/common/PacketEmitter.h"

namespace inet {

Define_Module(PacketEmitter);

void PacketEmitter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetFilter.setPattern(par("packetFilter"), par("packetDataFilter"));
        const char *directionString = par("direction");
        if (!strcmp(directionString, "inbound"))
            direction = DIRECTION_INBOUND;
        else if (!strcmp(directionString, "outbound"))
            direction = DIRECTION_OUTBOUND;
        else
            throw cRuntimeError("Unknown direction");
        const char *protocolName = par("protocolName");
        protocol = Protocol::getProtocol(protocolName);
    }
}

void PacketEmitter::processPacket(Packet *packet)
{
    // void
}

void PacketEmitter::pushPacket(Packet *packet, cGate *gate)
{
    emitPacket(packet);
    PacketFlowBase::pushPacket(packet, gate);
}

void PacketEmitter::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    emitPacket(packet);
    PacketFlowBase::handlePushPacketProcessed(packet, gate, successful);
}

void PacketEmitter::emitPacket(Packet *packet)
{
    if (packetFilter.matches(packet)) {
        auto clone = new Packet(packet->getName(), packet->peekAllAsBytes());
        clone->copyTags(*packet);
        clone->addTagIfAbsent<DirectionTag>()->setDirection(direction);
        clone->addTagIfAbsent<PacketProtocolTag>()->setProtocol(protocol);
        emit(packetSentToLowerSignal, clone);
        delete clone;
    }
}

} // namespace inet

