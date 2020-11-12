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

#include "inet/protocolelement/common/PacketEmitter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"

namespace inet {

Define_Module(PacketEmitter);

void PacketEmitter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        signal = registerSignal(par("signalName"));
        packetFilter.setPattern(par("packetFilter"), par("packetDataFilter"));
        const char *directionString = par("direction");
        if (!strcmp(directionString, "inbound"))
            direction = DIRECTION_INBOUND;
        else if (!strcmp(directionString, "outbound"))
            direction = DIRECTION_OUTBOUND;
        else if (!strcmp(directionString, "undefined"))
            direction = DIRECTION_UNDEFINED;
        else
            throw cRuntimeError("Unknown direction parameter value");
    }
}

void PacketEmitter::processPacket(Packet *packet)
{
    if (direction != DIRECTION_UNDEFINED) {
        const auto& directionTag = packet->addTagIfAbsent<DirectionTag>();
        if (directionTag->getDirection() == DIRECTION_UNDEFINED)
            directionTag->setDirection(direction);
        else if (directionTag->getDirection() != direction)
            throw cRuntimeError("Packet direction tag doesn't match direction parameter");
    }
    delete processedPacket;
    processedPacket = packet->dup();
}

void PacketEmitter::pushPacket(Packet *packet, cGate *gate)
{
    emitPacket(packet);
    PacketFlowBase::pushPacket(packet, gate);
}

void PacketEmitter::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    emitPacket(processedPacket);
    PacketFlowBase::handlePushPacketProcessed(packet, gate, successful);
}

void PacketEmitter::emitPacket(Packet *packet)
{
    if (packetFilter.matches(packet))
        emit(signal, packet);
}

} // namespace inet

