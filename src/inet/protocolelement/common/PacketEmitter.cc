//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        packetFilter.setExpression(par("packetFilter").objectValue());
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

void PacketEmitter::pushPacketEnd(Packet *packet, cGate *gate)
{
    emitPacket(packet);
    PacketFlowBase::pushPacketEnd(packet, gate);
}

void PacketEmitter::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
//    emitPacket(processedPacket);
    PacketFlowBase::handlePushPacketProcessed(packet, gate, successful);
}

void PacketEmitter::emitPacket(Packet *packet)
{
    if (packetFilter.matches(packet))
        emit(signal, packet);
}

} // namespace inet

