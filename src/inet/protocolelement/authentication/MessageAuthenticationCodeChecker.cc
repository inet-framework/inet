//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/authentication/MessageAuthenticationCodeChecker.h"

#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace inet {

Define_Module(MessageAuthenticationCodeChecker);

void MessageAuthenticationCodeChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerLength = b(par("headerLength"));
    }
}

void MessageAuthenticationCodeChecker::processPacket(Packet *packet)
{
    packet->popAtFront<ByteCountChunk>(headerLength);
}

bool MessageAuthenticationCodeChecker::matchesPacket(const Packet *packet) const
{
    return true;
}

} // namespace inet

