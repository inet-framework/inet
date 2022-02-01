//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/common/ProtocolChecker.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/stlutils.h"

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
    return protocol != nullptr && contains(protocols, protocol);
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

void ProtocolChecker::handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (gate == outputGate && servicePrimitive == SP_INDICATION)
        protocols.insert(&protocol);
}

} // namespace inet

