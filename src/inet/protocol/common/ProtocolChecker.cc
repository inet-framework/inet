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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/common/ProtocolTag_m.h"
#include "inet/protocol/common/ProtocolChecker.h"

namespace inet {

Define_Module(ProtocolChecker);

void ProtocolChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        WATCH_PTRSET(protocols);
    }
}

bool ProtocolChecker::matchesPacket(const Packet *packet) const
{
    const auto& packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    return protocol != nullptr && protocols.find(protocol) != protocols.end();
}

void ProtocolChecker::dropPacket(Packet *packet)
{
    auto packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    if (protocol)
        EV_WARN << "Dropping packet because the protocol is not registered" << EV_FIELD(protocol) << EV_FIELD(packet) << EV_ENDL;
    else
        EV_WARN << "Dropping packet because the PacketProtocolTag is missing" << EV_FIELD(packet) << EV_ENDL;
    PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
}

void ProtocolChecker::handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
}

void ProtocolChecker::handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (gate == outputGate && servicePrimitive == SP_INDICATION)
        protocols.insert(&protocol);
}

} // namespace inet

