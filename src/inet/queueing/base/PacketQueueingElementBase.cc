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

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/base/PacketQueueingElementBase.h"

namespace inet {
namespace queueing {

void PacketQueueingElementBase::animateSend(Packet *packet, cGate *gate)
{
    auto envir = getEnvir();
    if (envir->isGUI()) {
        packet->setSentFrom(gate->getOwnerModule(), gate->getId(), simTime());
        if (gate->getNextGate() != nullptr) {
#if OMNETPP_BUILDNUM >= 1504
            envir->beginSend(packet, SendOptions());
#else
            envir->beginSend(packet);
#endif
            while (gate->getNextGate() != nullptr) {
                envir->messageSendHop(packet, gate);
                gate = gate->getNextGate();
            }
            envir->endSend(packet);
        }
    }
}

void PacketQueueingElementBase::checkPushPacketSupport(cGate *gate) const
{
    auto startGate = findConnectedGate<IPacketQueueingElement>(gate, -1);
    if (startGate != nullptr) {
        auto startElement = check_and_cast<IPacketQueueingElement *>(startGate->getOwnerModule());
        if (!startElement->supportsPushPacket(startGate))
            throw cRuntimeError(startGate->getOwnerModule(), "doesn't support push on gate %s", startGate->getFullPath().c_str());
    }
    auto endGate = findConnectedGate<IPacketQueueingElement>(gate, 1);
    if (endGate != nullptr) {
        auto endElement = check_and_cast<IPacketQueueingElement *>(endGate->getOwnerModule());
        if (!endElement->supportsPushPacket(endGate))
            throw cRuntimeError(endGate->getOwnerModule(), "doesn't support push on gate %s", endGate->getFullPath().c_str());
    }
}

void PacketQueueingElementBase::checkPopPacketSupport(cGate *gate) const
{
    auto startGate = findConnectedGate<IPacketQueueingElement>(gate, -1);
    if (startGate != nullptr) {
        auto startElement = check_and_cast<IPacketQueueingElement *>(startGate->getOwnerModule());
        if (!startElement->supportsPopPacket(startGate))
            throw cRuntimeError(startGate->getOwnerModule(), "doesn't support pop on gate %s", startGate->getFullPath().c_str());
    }
    auto endGate = findConnectedGate<IPacketQueueingElement>(gate, 1);
    if (endGate != nullptr) {
        auto endElement = check_and_cast<IPacketQueueingElement *>(endGate->getOwnerModule());
        if (!endElement->supportsPopPacket(endGate))
            throw cRuntimeError(endGate->getOwnerModule(), "doesn't support pop on gate %s", endGate->getFullPath().c_str());
    }
}

void PacketQueueingElementBase::checkPushOrPopPacketSupport(cGate *gate) const
{
    auto startGate = findConnectedGate<IPacketQueueingElement>(gate, -1);
    if (startGate != nullptr) {
        auto startElement = check_and_cast<IPacketQueueingElement *>(startGate->getOwnerModule());
        if (!startElement->supportsPushPacket(startGate) && !startElement->supportsPopPacket(startGate))
            throw cRuntimeError(startGate->getOwnerModule(), "neither supports push or pop on gate %s", startGate->getFullPath().c_str());
    }
    auto endGate = findConnectedGate<IPacketQueueingElement>(gate, 1);
    if (endGate != nullptr) {
        auto endElement = check_and_cast<IPacketQueueingElement *>(endGate->getOwnerModule());
        if (!endElement->supportsPushPacket(endGate) && !endElement->supportsPopPacket(endGate))
            throw cRuntimeError(endGate->getOwnerModule(), "neither supports push or pop on gate %s", endGate->getFullPath().c_str());
    }
}

void PacketQueueingElementBase::pushOrSendPacket(Packet *packet, cGate *gate)
{
    auto consumer = findConnectedModule<IPassivePacketSink>(gate);
    pushOrSendPacket(packet, gate, consumer);
}

void PacketQueueingElementBase::pushOrSendPacket(Packet *packet, cGate *gate, IPassivePacketSink *consumer)
{
    if (consumer != nullptr) {
        animateSend(packet, gate);
        consumer->pushPacket(packet, gate->getPathEndGate());
    }
    else {
        take(packet);
        send(packet, gate);
    }
}

void PacketQueueingElementBase::dropPacket(Packet *packet, PacketDropReason reason, int limit)
{
    PacketDropDetails details;
    details.setReason(reason);
    details.setLimit(limit);
    emit(packetDroppedSignal, packet, &details);
    delete packet;
}

} // namespace queueing
} // namespace inet

