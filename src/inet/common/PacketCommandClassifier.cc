//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/PacketCommandClassifier.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"

namespace inet {

Define_Module(PacketCommandClassifier);

void PacketCommandClassifier::initialize()
{
    packetOutGate = gate("packetOut");
    messageOutGate = gate("messageOut");
}

void PacketCommandClassifier::arrived(cMessage *message, cGate *inGate, simtime_t t)
{
    auto outGate = (message->isPacket()) ? packetOutGate : messageOutGate;
    outGate->deliver(message, t);
}

void PacketCommandClassifier::handleMessage(cMessage *message)
{
    auto outGate = (message->isPacket()) ? packetOutGate : messageOutGate;
    send(message, outGate);
}

void PacketCommandClassifier::handleRegisterService(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    if (g->getType() == cGate::INPUT) {
        registerService(protocol, packetOutGate, servicePrimitive);
        registerService(protocol, messageOutGate, servicePrimitive);
    }
    else {
        registerService(protocol, gate("in"), servicePrimitive);
    }
}

void PacketCommandClassifier::handleRegisterProtocol(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (g->getType() == cGate::INPUT) {
        registerProtocol(protocol, packetOutGate, servicePrimitive);
        registerProtocol(protocol, messageOutGate, servicePrimitive);
    }
    else {
        registerProtocol(protocol, gate("in"), servicePrimitive);
    }
}

} // namespace inet

