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

#include "inet/protocolelement/dispatching/SendWithProtocol.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/dispatching/ProtocolHeader_m.h"

namespace inet {

Define_Module(SendWithProtocol);

void SendWithProtocol::processPacket(Packet *packet)
{
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    auto header = makeShared<ProtocolHeader>();
    header->setProtocolId(protocol->getId());
    packet->insertAtFront(header);
}

void SendWithProtocol::handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    registerService(protocol, inputGate, servicePrimitive);
}

void SendWithProtocol::handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    registerProtocol(protocol, outputGate, servicePrimitive);
}

} // namespace inet

